/*******************************************************************************
*
* FILENAME : minion.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 23.11.2023
* 
*******************************************************************************/

#include <algorithm> // std::min
#include <string> // std::string

#include <ifaddrs.h> // getifaddrs

#include "minion.hpp"

using namespace nsrd;

namespace
{
std::string STORAGE_FILE_NAME("minion_storage_");
std::string LOG_FILE_NAME("minion_log_");
void InitSockaddr(struct sockaddr *sa, const char *addr, unsigned short port);
void InitEvent(MinionEvent *event, std::size_t cmd_id, nsrd::MinionEventType type, std::size_t offset, std::size_t length);
void CleanUpSocket(int socket);
std::string FindLocalIp();
}

Minion::Minion(unsigned short port, std::size_t storage_size)
    : m_minion_port(port),
      m_minion_socket(0),
      m_proxy_sa(),
      m_proxy_sa_len(sizeof(struct sockaddr_in)),
      m_storage_file(0),
      m_run(false),
      m_stop_reactor_pipe(),
      m_logger(Handleton<Logger>::GetInstance()),
      m_reactor()
{
    OpenSocket();

    std::string str_port = std::to_string(port);

    std::string file_name(STORAGE_FILE_NAME + str_port);
    m_storage_file = open(file_name.c_str(), O_CREAT | O_RDWR, 0666);

    nsrd::Logger::SetPath(LOG_FILE_NAME + str_port);

    if (-1 == pipe(m_stop_reactor_pipe))
    {
        m_logger->Error("[MINION] COULDN'T OPEN CLOSING PIPE");
        throw::std::runtime_error("[MINION] COULDN'T OPEN CLOSING PIPE");
    }

    m_reactor.Add(m_stop_reactor_pipe[READ_END], std::bind(&Minion::StopReactorHandler, this),
                                                                        SocketEventType::READ);

    m_reactor.Add(m_minion_socket, std::bind(&Minion::InputMediator, this), SocketEventType::READ);

    int v = storage_size;
    (void) v;
}

Minion::~Minion()
{
    m_logger->Info("[MINION] DESTROYED");
    close(m_minion_socket);
    close(m_stop_reactor_pipe[READ_END]);
    close(m_stop_reactor_pipe[WRITE_END]);
}

void Minion::OpenSocket()
{
    sockaddr sa;
    InitSockaddr(&sa, FindLocalIp().c_str(), m_minion_port);

    m_minion_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == m_minion_socket)
    {
        m_logger->Error("[MINION] COULDN'T OPEN SOCKET");
        throw std::runtime_error("[MINION] COULDN'T OPEN SOCKET");
    }

    if (-1 == bind(m_minion_socket, &sa, INET_ADDRSTRLEN))
    {
        close(m_minion_socket);
        m_logger->Error("[MINION] COULDN'T BIND SOCKET");
        throw std::runtime_error("[MINION] COULDN'T BIND SOCKET");
    }
}

void Minion::AcceptProxyConnection()
{
    MinionEvent request;
    ReadConnectionRequest(&request);

    if (nsrd::MinionEventType::START_COMMUNICATE != request.m_type)
    {
        m_logger->Error("[MINION] EXPECTED START_COMMUNICATE EVENT TYPE");
        throw std::runtime_error("[MINION] EXPECTED START_COMMUNICATE EVENT TYPE");
    }

    MinionEvent response;
    InitEvent(&response, request.m_event_id, nsrd::MinionEventType::RESPONSE_SUCCESS, request.m_offset, request.m_length);

    SendData(&response, sizeof(MinionEvent));

    m_logger->Info("[MINION] PROXY HAS BEEN CONNECTED");
}

void Minion::Write(const MinionEvent &request)
{
    void *buf = operator new(request.m_length);
    std::memset(buf, 0, request.m_length);

    ReadData(buf, request.m_length);
    StorageWrite(buf, request.m_length, request.m_offset);

    operator delete (buf);

    MinionEvent response;
    InitEvent(&response, request.m_event_id, nsrd::MinionEventType::RESPONSE_SUCCESS, request.m_offset, request.m_length);

    SendData(&response, sizeof(MinionEvent));
}

void Minion::Read(const MinionEvent &request)
{
    void *buf = operator new(request.m_length);
    std::memset(buf, 0, request.m_length);

    StorageRead(buf, request.m_length, request.m_offset);

    MinionEvent response;
    InitEvent(&response, request.m_event_id, nsrd::MinionEventType::RESPONSE_SUCCESS, request.m_offset, request.m_length);

    SendData(&response, sizeof(MinionEvent));
    SendData(buf, request.m_length);
    SendData(&response, sizeof(MinionEvent));

    operator delete (buf);
}

void Minion::StopReactorHandler()
{
    char buf[1];
    if (-1 == read(m_stop_reactor_pipe[READ_END], buf, 1))
    {
        m_logger->Error("[MINION] CLOSING PIPE");
    }

    m_reactor.Stop();
}

void Minion::InputMediator()
{
    MinionEvent request;
    ReadData(&request, sizeof(MinionEvent));

    if (request.m_event_id == 0 || request.m_magic != MINION_EVENT_MAGIC)
    {
        CleanUpSocket(m_minion_socket);
        return;
    }
    
    switch (request.m_type)
    {
        case (nsrd::MinionEventType::WRITE):
        {
            Write(request);
            break;
        }
        case (nsrd::MinionEventType::READ):
        {
            Read(request);
            break;
        }
        case (nsrd::MinionEventType::STOP_COMMUNICATE):
        {
            Stop();
            break;
        }
        default:
        {
            m_logger->Error("[MINION] REQUEST EVENT TYPE IS NOT SUPPORTED");
            break;
        }
    }
}

void Minion::Run()
{
    CleanUpSocket(m_minion_socket);
    
    AcceptProxyConnection();

    if (!m_run)
    {
        m_run = true;
        m_logger->Info("[MINION] STARTS");
        m_reactor.Run();
    }
}

void Minion::Stop()
{
    m_logger->Info("[MINION] RECEIVED STOP_COMMUNICATE REQUEST"); 

    if (m_run)
    {
        m_run = false;
        if (-1 == write(m_stop_reactor_pipe[WRITE_END], "", 1))
        {
            m_logger->Error("[MINION] GOT ERROR STOPPING MONITOR");
        }
    }
}

bool Minion::SendData(void *buf, std::size_t length)
{
    const char *buffer = static_cast<const char *>(buf);
    std::size_t sendlen = std::min(length, 1024UL);
    int bytes_written = 0;

    while (0 < length)
    {
        if (0 > (bytes_written = sendto(m_minion_socket, buffer, sendlen,
                                            0, &m_proxy_sa, INET_ADDRSTRLEN)))
        {
            if (errno == EINTR){ continue; }
            m_logger->Error(std::string("[MINION] SENDDATA HAS FAILED ") + std::strerror(errno));
            return (false);
        }

        buffer += bytes_written;
        length -= bytes_written;
        sendlen = std::min(length, 1024UL);
    }

    return (true);
}

bool Minion::ReadData(void *buf, std::size_t length)
{
    sockaddr from_sa = {0, {0}};
    socklen_t from_sa_len = sizeof(sockaddr);
    char *buffer = static_cast<char *>(buf);
    int bytes_read = 0;

    while (0 < length)
    {
        if (0 > (bytes_read = recvfrom(m_minion_socket, buffer, length, 0, &from_sa, &from_sa_len)))
        {
            if (errno == EINTR){ continue; }
            m_logger->Error(std::string("[MINION] READDATA HAS FAILED ") + std::strerror(errno));
            return (false);
        }

        if (0 != std::memcmp(from_sa.sa_data, m_proxy_sa.sa_data, 14))
        {
            m_logger->Error("[MINION] RECEIVED DATA IS NOT FROM THE CURRENT PROXY");
            return (false);
        }
        
        buffer += bytes_read;
        length -= bytes_read;
    }

    return (true);
}

void Minion::ReadConnectionRequest(MinionEvent *event)
{
    std::size_t length = sizeof(MinionEvent);
    int bytes_read = 0;

    while (0 < length)
    {
        if (0 > (bytes_read = recvfrom(m_minion_socket, event, length, 0, &m_proxy_sa, &m_proxy_sa_len)))
        {
            if (errno == EINTR){ continue; }
            m_logger->Error("[MINION] READING CONNECTION REQUEST HAS FAILED");
            throw std::runtime_error("[MINION] READING CONNECTION REQUEST HAS FAILED");
        }
        
        event += bytes_read;
        length -= bytes_read;
    }
}

void Minion::StorageRead(void *buf, unsigned len, std::size_t offset)
{
    if (-1 == lseek(m_storage_file, offset, SEEK_SET))
    {
        m_logger->Error(std::string("[MINION] StorageRead, lseek(), ") + std::strerror(errno));
    }

    if (-1 == read(m_storage_file, buf, len))
    {
        m_logger->Error(std::string("[MINION] StorageRead, read(), ") + std::strerror(errno));
    }    
}

void Minion::StorageWrite(void *buf, unsigned len, std::size_t offset)
{
    if (-1 == lseek(m_storage_file, offset, SEEK_SET))
    {
        m_logger->Error(std::string("[MINION] StorageWrite, lseek(), ") + std::strerror(errno));
    }

    if (-1 == write(m_storage_file, buf, len))
    {
        m_logger->Error(std::string("[MINION] StorageWrite, write(), ") + std::strerror(errno));
    }
}

namespace
{
void InitSockaddr(sockaddr *sa, const char *addr, unsigned short port)
{
    sockaddr_in *sai = reinterpret_cast<sockaddr_in *>(sa);
    sai->sin_family = AF_INET;
    sai->sin_port = htons(port);

    int pton_status = inet_pton(AF_INET, addr, &sai->sin_addr.s_addr);
    if (-1 == pton_status || 0 == pton_status)
    {
        throw std::runtime_error("[MINION] INITSOCKADDR");
    }
}

void InitEvent(MinionEvent *event, std::size_t cmd_id, nsrd::MinionEventType type, std::size_t offset, std::size_t length)
{
    std::memset(event, 0, sizeof(MinionEvent));
    event->m_type = type;
    event->m_magic = MINION_EVENT_MAGIC;
    event->m_event_id = cmd_id;
    event->m_offset = offset;
    event->m_length = length;   
}

void CleanUpSocket(int socket)
{
    char buffer[1024];
    int bytes_read = 0;
    do
    {
        bytes_read = recv(socket, buffer, 1024, MSG_DONTWAIT);
    }
    while (0 <= bytes_read || errno == EINTR);
}

std::string FindLocalIp()
{
    ifaddrs *addrs;
    getifaddrs(&addrs);
    ifaddrs *runner = addrs;
    std::string ip;

    while (runner) 
    {
        if (runner->ifa_addr && 0 != strcmp(runner->ifa_name, "lo") && runner->ifa_addr->sa_family == AF_INET)
        {
            sockaddr_in *pAddr = reinterpret_cast<sockaddr_in *>(runner->ifa_addr);
            ip = inet_ntoa(pAddr->sin_addr);
            break;
        }

        runner = runner->ifa_next;
    }

    freeifaddrs(addrs);

    if (0 == ip.length())
    {
        throw std::runtime_error("[MINION] FindLocalIp");
    }
    
    return (ip);
}
}
