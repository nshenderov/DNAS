/*******************************************************************************
*
* FILENAME : minion_proxy_test.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 19.11.2023
* 
*******************************************************************************/


#include <iostream> // std::cout, std::endl
#include <arpa/inet.h> // inet_pton
#include <string> // std::string
#include <cstring> // std::memcpy
#include <thread> // std::thread
#include <unistd.h> // close

#include "testing.hpp" // testing::Testscmp, testing::UnitTest

#include "minion_manager.hpp" // nsrd::MinionManager
#include "minion_proxy.hpp" // nsrd::MinionProxy

using namespace nsrd;
using namespace nsrd::testing;

MinionManager::IMinionProxy::~IMinionProxy(){}

namespace
{
void TestMinionManager();
void TestInjection();

const char LOCALHOST[INET_ADDRSTRLEN] = "0.0.0.0";
typedef struct sockaddr sockaddr_t;

void InitSockaddr(struct sockaddr *sa, const char *addr, unsigned short port);

class Minion
{
public:
    explicit Minion(const char *ip, unsigned short port, std::size_t storage_size);
    ~Minion();
    void Run(bool delayed = false);
    void Stop();

private:
    char m_minion_ip[INET_ADDRSTRLEN];
    unsigned short m_minion_port;
    int m_minion_socket;

    char m_proxy_ip[INET_ADDRSTRLEN];
    unsigned short m_proxy_port;
    struct sockaddr_in m_proxy_sa;
    socklen_t m_proxy_sa_len;
    std::shared_ptr<char> m_storage;
    bool m_run;

    void OpenSocket();
    void AcceptProxy();
    void SendData(const char *buf, std::size_t length);
    void ReadData(char *buf, std::size_t length);
    void ReadFromInLoop(char *buf, std::size_t length, struct sockaddr *from_sa, socklen_t *from_sa_len);
    void SendToInLoop(const char *buf, std::size_t length, struct sockaddr *to_sa, socklen_t addr_len);
    void StorageRead(char *buf, unsigned len, std::size_t offset);
    void StorageWrite(const char *buf, unsigned len, std::size_t offset);
};
} // namespace

int main()
{
    Testscmp cmp;
    cmp.AddTest(UnitTest("General", TestMinionManager));
    cmp.AddTest(UnitTest("Injection", TestInjection));
    cmp.Run();

    return (0);
}

namespace
{
void MinionThread()
{
    Minion minion(LOCALHOST, 1502, 500);
    minion.Run();
}

void SleepingMinionThread()
{
    Minion minion(LOCALHOST, 1604, 500);
    minion.Run(true);
}

void TestMinionManager()
{
    std::thread minon_thread = std::thread(&MinionThread);

    MinionProxy *proxy = new MinionProxy(LOCALHOST, 1502);

    char req[8] = "Hellowa";
    proxy->Write(MinionManager::CommandParams({8, 0, req}));

    char res[8] = {0};
    proxy->Read(MinionManager::CommandParams({8, 0, res}));

    std::cout << "Response: " << res << std::endl;

    TH_ASSERT(0 == std::strcmp(res, req));

    delete proxy;
    minon_thread.join();
}

void TestInjection()
{
    std::thread minon_thread = std::thread(&SleepingMinionThread);

    MinionProxy *proxy = new MinionProxy(LOCALHOST, 1604);

    char req[8] = "Hellowa";
    proxy->Write(MinionManager::CommandParams({8, 0, req}));

    char res[8] = {0};
    proxy->Read(MinionManager::CommandParams({8, 0, res}));

    std::cout << "Responce: " << res << std::endl;

    TH_ASSERT(0 == std::strcmp(res, req));

    delete proxy;
    minon_thread.join();
}

void InitSockaddr(struct sockaddr *sa, const char *addr, unsigned short port)
{
    struct sockaddr_in *sai = reinterpret_cast<struct sockaddr_in *>(sa);
    sai->sin_family = AF_INET;
    sai->sin_port = htons(port);

    int pton_status = inet_pton(AF_INET, addr, &sai->sin_addr.s_addr);
    if (-1 == pton_status || 0 == pton_status)
    {
        throw std::runtime_error("InitSockaddr");
    }
}

void InitEvent(minion_event_t *event, nsrd::MinionEventType type, std::size_t offset, std::size_t length)
{
    std::memset(event, 0, sizeof(minion_event_t));
    event->m_type = type;
    event->m_offset = offset;
    event->m_length = length;   
}


Minion::Minion(const char *ip, unsigned short port, std::size_t storage_size)
    : m_minion_ip(),
      m_minion_port(port),
      m_minion_socket(0),
      m_proxy_ip(),
      m_proxy_port(0),
      m_proxy_sa(),
      m_proxy_sa_len(sizeof(struct sockaddr_in)),
      m_storage(static_cast<char*>(operator new(storage_size))),
      m_run(false)
{
    std::memcpy(m_minion_ip, ip, INET_ADDRSTRLEN);
    std::memset(m_storage.get(), 0, storage_size);
    OpenSocket();
}

Minion::~Minion()
{
    close(m_minion_socket);
}

void Minion::OpenSocket()
{
    sockaddr_t sa;
    InitSockaddr(&sa, m_minion_ip, m_minion_port);

    m_minion_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == m_minion_socket)
    {
        throw std::runtime_error("[MINION_STUB] Couldn't open proxy socket");
    }

    if (-1 == bind(m_minion_socket, &sa, INET_ADDRSTRLEN))
    {
        close(m_minion_socket);
        throw std::runtime_error("[MINION_STUB] Couldn't bind socket");
    }
}

void Minion::ReadFromInLoop(char *buf, std::size_t length, struct sockaddr *from_sa, socklen_t *from_sa_len)
{
    int bytes_read = 0;
    while (0 < length)
    {
        if (0 > (bytes_read = recvfrom(m_minion_socket, buf, length, 0, from_sa, from_sa_len)))
        {
            if (errno == EINTR){ continue; }
            throw std::runtime_error("[MINION_STUB] Read failed");
        }
        
        buf += bytes_read;
        length -= bytes_read;
    }
}

void Minion::SendToInLoop(const char *buf, std::size_t length, struct sockaddr *to_sa, socklen_t addr_len)
{
    int bytes_written = 0;
    while (0 < length)
    {
        if (0 > (bytes_written = sendto(m_minion_socket, buf, length, 0, to_sa, addr_len)))
        {
            if (errno == EINTR){ continue; }
            throw std::runtime_error("[MINION_STUB] Send failed");
        }
        buf += bytes_written;
        length -= bytes_written;
    }
}

void Minion::AcceptProxy()
{
    minion_event_t event;
    ReadFromInLoop(reinterpret_cast<char *>(&event), sizeof(minion_event_t), reinterpret_cast<struct sockaddr *>(&m_proxy_sa), &m_proxy_sa_len);

    m_proxy_port = ntohs(m_proxy_sa.sin_port);
    inet_ntop(AF_INET, &(m_proxy_sa.sin_addr), m_proxy_ip, INET_ADDRSTRLEN);

    minion_event_t response;
    InitEvent(&response, nsrd::MinionEventType::RESPONSE_SUCCESS, 0, 0);

    SendToInLoop(reinterpret_cast<const char *>(&response), sizeof(minion_event_t), reinterpret_cast<struct sockaddr *>(&m_proxy_sa), INET_ADDRSTRLEN);

    std::cout 
    << "ACCEPTED_PROXY:"
    << "\nip: " << m_proxy_ip
    << "\nport: " << m_proxy_port
    << std::endl;
}

void Minion::StorageRead(char *buf, unsigned len, std::size_t offset)
{
  std::memcpy(buf, m_storage.get() + offset, len);
}

void Minion::StorageWrite(const char *buf, unsigned len, std::size_t offset)
{
  std::memcpy(m_storage.get() + offset, buf, len);
}

void Minion::Run(bool delayed)
{
    char buffer[1024];
    int bytes_read = 0;
    do
    {
        bytes_read = recv(m_minion_socket, buffer, 1024, MSG_DONTWAIT);
    }
    while (0 <= bytes_read || errno == EINTR);
    
    AcceptProxy();

    m_run = true;

    while (m_run)
    {
        minion_event_t request;

        if (delayed)
        {
            int i = 0;
            while (delayed && 2 > i++)
            {
                ReadData(reinterpret_cast<char*>(&request), sizeof(minion_event_t));

                if (request.m_type == nsrd::MinionEventType::STOP_COMMUNICATE)
                {
                    Stop();
                    return;
                }

                if (request.m_type == nsrd::MinionEventType::WRITE)
                {
                    char buffer[1024];
                    ReadData(buffer, request.m_length);
                }

                std::cout << "[MINION_STUB] RECEIVED AND SKIPPED REQUEST NUMBER: " << i << std::endl;
            }
        }
        
        
        std::cout << "[MINION_STUB] RECEIVED REQUEST ";
        ReadData(reinterpret_cast<char*>(&request), sizeof(minion_event_t));
        
        switch (request.m_type)
        {
            case (nsrd::MinionEventType::WRITE):
            {
                std::cout << "WRITE" << std::endl;

                char *buf = static_cast<char *>(operator new(request.m_length));
                std::memset(buf, 0, request.m_length);

                ReadData(buf, request.m_length);
                StorageWrite(buf, request.m_length, request.m_offset);

                delete buf;

                minion_event_t response;
                InitEvent(&response, nsrd::MinionEventType::RESPONSE_SUCCESS, 0, 0);

                SendData(reinterpret_cast<const char *>(&response), sizeof(minion_event_t));
                break;
            }
            case (nsrd::MinionEventType::READ):
            {
                std::cout << "READ" << std::endl;

                char *buf = static_cast<char *>(operator new(request.m_length));
                std::memset(buf, 0, request.m_length);

                StorageRead(buf, request.m_length, request.m_offset);

                minion_event_t response;
                InitEvent(&response, nsrd::MinionEventType::RESPONSE_SUCCESS, request.m_offset, request.m_length);

                SendData(reinterpret_cast<const char *>(&response), sizeof(minion_event_t));
                SendData(buf, request.m_length);

                delete buf;

                break;
            }
            case (nsrd::MinionEventType::STOP_COMMUNICATE):
            {
                std::cout << "[MINION_STUB] STOP_COMMUNICATE" << std::endl;

                Stop();
                return;
            }
            
            default:
                std::cout << "[MINION_STUB] NOT_SUPPORTED" << std::endl;
                break;
        }

    }
    
}

void Minion::Stop()
{
    m_run = false;
}

void Minion::SendData(const char *buf, std::size_t length)
{
    int bytes_written = 0;
    while (0 < length)
    {
        if (0 > (bytes_written = sendto(m_minion_socket, buf, length, 0, reinterpret_cast<struct sockaddr *>(&m_proxy_sa), INET_ADDRSTRLEN)))
        {
            if (errno == EINTR){ continue; }
            throw std::runtime_error("[MINION_STUB] Send failed");
        }
        buf += bytes_written;
        length -= bytes_written;
    }
}

void Minion::ReadData(char *buf, std::size_t length)
{
    struct sockaddr_in from_sa;
    socklen_t from_sa_len = sizeof(from_sa);
    std::memset(&from_sa, 0, sizeof(from_sa));
    int bytes_read = 0;

    while (0 < length)
    {
        if (0 > (bytes_read = recvfrom(m_minion_socket, buf, length, 0, reinterpret_cast<struct sockaddr *>(&from_sa), &from_sa_len)))
        {
            if (errno == EINTR){ continue; }
            
            throw std::runtime_error("[MINION_STUB] Read failed");
        }
        
        if (0 != std::memcmp(&from_sa, &m_proxy_sa,sizeof(from_sa)))
        {
            std::cerr << "[MINION_STUB] Received data is not from the current proxy" << std::endl;
        }
        
        buf += bytes_read;
        length -= bytes_read;
    }
}
} // namespace