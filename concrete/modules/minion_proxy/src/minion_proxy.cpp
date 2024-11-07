/*******************************************************************************
*
* FILENAME : minion_proxy.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 19.11.2023
* 
*******************************************************************************/

#include <arpa/inet.h> // inet_pton
#include <iostream> // std::cerr, std::endl;
#include <cstring> // std::strerror
#include <unistd.h> // close
#include <ifaddrs.h> // getifaddrs

#include "handleton.hpp" // nsrd::Handleton
#include "async_injection.hpp" // nsrd::AsyncInjection

#include "minion_proxy.hpp" // nsrd::MinionProxy

using namespace nsrd;

namespace
{
const AsyncInjection::interval_t CHECK_INTERVAL(300);
const std::size_t MAX_ATTEMPTS = 15;
void InitSockaddr(struct sockaddr *sa, const char *addr, unsigned short port);
void InitEvent(MinionEvent *event, std::size_t cmd_id, nsrd::MinionEventType type, std::size_t offset, std::size_t length);
std::string FindLocalIp();
void CleanUpSocket(int socket);
bool IsResponceValid(const MinionEvent &request, const MinionEvent &responce);
} // namespace

MinionProxy::MinionProxy(const std::string &minion_ip, unsigned short minion_port)
    : m_minion_ip(minion_ip),
      m_minion_port(minion_port),
      m_minion_sa(),
      m_proxy_socket(0),
      m_mutex(),
      m_ids_counter(0)
{
    InitSockaddr(&m_minion_sa, m_minion_ip.c_str(), m_minion_port);
    OpenProxySocket();
    ConnectToMinion();
}

MinionProxy::~MinionProxy()
{
    StopCommunicate();
    close(m_proxy_socket);
}

bool MinionProxy::Write(MinionManager::CommandParams params)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    
    CleanUpSocket(m_proxy_socket);

    MinionEvent request;
    InitEvent(&request, GetNewCmdId(), nsrd::MinionEventType::WRITE, params.offset, params.length);

    if (!Send(request, params.buffer))
    {
        return (false);
    }

    std::shared_ptr<bool> shouldnt_reschedule(new bool(false));
    AsyncInjection::Inject(CheckResponse(shouldnt_reschedule, this, request, params.buffer), CHECK_INTERVAL);

    if(!Read(request))
    {
        *shouldnt_reschedule = true;
        return (false);
    }

    *shouldnt_reschedule = true;
    return (true);
}

bool MinionProxy::Read(MinionManager::CommandParams params)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    
    CleanUpSocket(m_proxy_socket);

    MinionEvent request;
    InitEvent(&request, GetNewCmdId(), nsrd::MinionEventType::READ, params.offset, params.length);

    if (!Send(request))
    {
        return (false);
    }

    std::shared_ptr<bool> shouldnt_reschedule(new bool(false));
    AsyncInjection::Inject(CheckResponse(shouldnt_reschedule, this, request, nullptr), CHECK_INTERVAL);

    if (!Read(request, params.buffer))
    {
        *shouldnt_reschedule = true;
        return (false);
    }
    
    *shouldnt_reschedule = true;
    return (true);
}

void MinionProxy::OpenProxySocket()
{
    m_proxy_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == m_proxy_socket)
    {
        throw std::runtime_error("Couldn't open proxy socket");
    }

    InitSockaddr(&m_proxy_sa, FindLocalIp().c_str(), m_minion_port + 1);

    if (-1 == bind(m_proxy_socket, &m_proxy_sa, INET_ADDRSTRLEN))
    {
        throw std::runtime_error("Couldn't bind proxy socket");
    }   
}

void MinionProxy::ConnectToMinion()
{
    std::unique_lock<std::mutex> lock(m_mutex);

    CleanUpSocket(m_proxy_socket);
    
    MinionEvent request;
    InitEvent(&request, GetNewCmdId(), nsrd::MinionEventType::START_COMMUNICATE, 0, 0);

    if (!Send(request))
    {
        throw std::runtime_error("Couldn't connect to the minion");
    }

    std::shared_ptr<bool> shouldnt_reschedule(new bool(false));
    AsyncInjection::Inject(CheckResponse(shouldnt_reschedule, this, request, nullptr), CHECK_INTERVAL);

    if (!Read(request))
    {
        *shouldnt_reschedule = true;
        throw std::runtime_error("Couldn't connect to the minion");
    }

    *shouldnt_reschedule = true;
}

void MinionProxy::StopCommunicate()
{
    std::unique_lock<std::mutex> lock(m_mutex);

    MinionEvent request;
    InitEvent(&request, GetNewCmdId(), nsrd::MinionEventType::STOP_COMMUNICATE, request.m_offset, request.m_length);

    if (!Send(request))
    {
        std::cerr << "Couldn't send the request" << std::endl;
        throw std::runtime_error("SendHeader failed, couldn't connect to the minion");
    }
}

/* Checkresponse */
/* ************************************************************************** */
MinionProxy::CheckResponse::CheckResponse(std::shared_ptr<bool> shouldnt_reschedule,
                                          MinionProxy *m_proxy,
                                          MinionEvent event,
                                          void *buf)
    : m_shouldnt_reschedule(shouldnt_reschedule),
      m_proxy(m_proxy),
      m_event(),
      m_buf(buf),
      m_attempts_counter(0)
{
    InitEvent(&m_event, event.m_event_id, event.m_type, event.m_offset, event.m_length);
}

MinionProxy::CheckResponse::~CheckResponse()
{}

bool MinionProxy::CheckResponse::operator()()
{
    if (false == *m_shouldnt_reschedule)
    {
        if (MAX_ATTEMPTS == m_attempts_counter++ || !m_proxy->Send(m_event, m_buf))
        {
            *m_shouldnt_reschedule = true;
            
            if (MAX_ATTEMPTS == m_attempts_counter)
            {
                std::cerr << "Reached max_attempts to retransmit" << std::endl;
            }
            else
            {
                std::cerr << "Injection failed to retransmit" << std::endl;
            }

            MinionEvent event;
            InitEvent(&event, m_proxy->GetNewCmdId(), nsrd::MinionEventType::RESPONSE_FAIL, 0, 0);
            if (-1 == send(m_proxy->m_proxy_socket, &event, sizeof(MinionEvent), 0))
            {
                std::cerr << "Failed sending RESPONSE_FAIL " << std::strerror(errno) << std::endl;
            }
        }
    }

    return (*m_shouldnt_reschedule);
}

bool MinionProxy::Send(const MinionEvent &request, void *buf)
{
    bool status = SendData(const_cast<MinionEvent*>(&request), sizeof(MinionEvent));

    if (request.m_type == nsrd::MinionEventType::WRITE)
    {
       status = SendData(buf, request.m_length);
    }

    if (!status)
    {
        std::cerr << "Couldn't send the request" << std::endl;
    }

    return (status);
}

bool MinionProxy::Read(const MinionEvent &request, void *buf)
{
    MinionEvent response;

    bool status = false;

    while (true)
    {
        status = ReadData(&response, sizeof(MinionEvent))
                  && nsrd::MinionEventType::RESPONSE_FAIL != response.m_type
                  && IsResponceValid(request, response);

        if (status && request.m_type != nsrd::MinionEventType::READ)
        {
            return (status);
        }

        if (status && request.m_type == nsrd::MinionEventType::READ)
        {
            status = ReadData(buf, request.m_length)
                  && ReadData(&response, sizeof(MinionEvent))
                  && IsResponceValid(request, response);

            if (status && nsrd::MinionEventType::RESPONSE_FAIL == response.m_type)
            {
                std::cerr << "Received response fail" << std::endl;
                return (false);
            }

            if (status)
            {
                return (status);
            }
        }

        CleanUpSocket(m_proxy_socket);
    }
}

bool MinionProxy::SendData(void *buf, std::size_t length)
{
    const char *buffer = static_cast<const char *>(buf);
    std::size_t sendlen = std::min(length, 1024UL);
    int bytes_written = 0;

    while (0 < length)
    {
        if (0 > (bytes_written = sendto(m_proxy_socket, buffer, sendlen,
                                            0, &m_minion_sa, INET_ADDRSTRLEN)))
        {
            if (errno == EINTR){ continue; }
            std::cerr << std::strerror(errno) << std::endl;
            std::cerr << "SendData has failed" << std::endl;
            return (false);
        }

        buffer += bytes_written;
        length -= bytes_written;
        sendlen = std::min(length, 1024UL);
    }

    return (true);
}

bool MinionProxy::ReadData(void *buf, std::size_t length)
{
    sockaddr from_sa = {0, {0}};
    socklen_t from_sa_len = sizeof(sockaddr);

    char *buffer = static_cast<char *>(buf);
    int bytes_read = 0;

    while (0 < length)
    {
        if (0 > (bytes_read = recvfrom(m_proxy_socket, buffer, length, 0, &from_sa, &from_sa_len)))
        {
            if (errno == EINTR){ continue; }
            std::cerr << std::strerror(errno) << std::endl;
            std::cerr << "ReadData has failed" << std::endl;
            return (false);
        }

        if (0 != std::memcmp(from_sa.sa_data, m_minion_sa.sa_data, 14)
         && 0 != std::memcmp(from_sa.sa_data, m_proxy_sa.sa_data, 14))
        {
            std::cerr << "Received data is not from the current minion" << std::endl;
            return (false);
        }
        
        buffer += bytes_read;
        length -= bytes_read;
    }

    return (true);
}


std::size_t MinionProxy::GetNewCmdId()
{
    return (m_ids_counter += 2);
}

namespace
{
void InitSockaddr(struct sockaddr *sa, const char *addr, unsigned short port)
{
    struct sockaddr_in *sai = reinterpret_cast<struct sockaddr_in *>(sa);
    sai->sin_family = AF_INET;
    sai->sin_port = htons(port);

    int pton_status = inet_pton(AF_INET, addr, &sai->sin_addr.s_addr);
    if (-1 == pton_status || 0 == pton_status)
    {
        throw std::runtime_error("InitSockaddr has failed");
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
        throw std::runtime_error("FindLocalIp couldn't find any proper ip");
    }
    
    return (ip);
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

bool IsResponceValid(const MinionEvent &request, const MinionEvent &responce)
{
    return (request.m_event_id == responce.m_event_id
         && request.m_length == responce.m_length
         && request.m_offset == responce.m_offset
         && request.m_magic == responce.m_magic);
}
} // namespace
