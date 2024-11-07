/*******************************************************************************
*
* FILENAME : minion_proxy.hpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 19.11.2023
* 
*******************************************************************************/

#ifndef NSRD_MINION_PROXY_HPP
#define NSRD_MINION_PROXY_HPP

#include <mutex> // std::timed_mutex
#include <memory> // std::shared_ptr
#include <sys/socket.h> // sockaddr

#include "minion_event.hpp" // nsrd::MinionEventType, nsrd::MinionEvent
#include "minion_manager.hpp" // nsrd::MinionManager::IMinionProxy

namespace nsrd
{

class MinionProxy: public MinionManager::IMinionProxy
{
public:
    explicit MinionProxy(const std::string &minion_ip, unsigned short minion_port);
    ~MinionProxy();

    bool Read(MinionManager::CommandParams params);
    bool Write(MinionManager::CommandParams params);

    MinionProxy(const MinionProxy &) =delete;
    MinionProxy(const MinionProxy &&) =delete;
    MinionProxy &operator=(const MinionProxy &) =delete;
    MinionProxy &operator=(const MinionProxy &&) =delete;

private:
    std::string m_minion_ip;
    unsigned short m_minion_port;
    sockaddr m_minion_sa;
    int m_proxy_socket;
    std::mutex m_mutex;
    sockaddr m_proxy_sa;
    std::size_t m_ids_counter;

    void ConnectToMinion();
    void OpenProxySocket();
    void StopCommunicate();
    bool Send(const MinionEvent &request, void *buf = nullptr);
    bool Read(const MinionEvent &request, void *buf = nullptr);
    bool SendData(void *buf, std::size_t length);
    bool ReadData(void *buf, std::size_t length);
    std::size_t GetNewCmdId();

    class CheckResponse
    {
    public:
        CheckResponse(std::shared_ptr<bool> shouldnt_reschedule,
                      MinionProxy *m_proxy,
                      MinionEvent event,
                      void *buf);
        ~CheckResponse();

        bool operator()();

    private:
        std::shared_ptr<bool> m_shouldnt_reschedule;
        MinionProxy *m_proxy;
        MinionEvent m_event;
        void *m_buf;
        std::size_t m_attempts_counter;
    };
};
} // namespace nsrd

#endif // RD142_nsRD_MINION_PROXY