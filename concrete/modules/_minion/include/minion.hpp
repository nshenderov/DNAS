/*******************************************************************************
*
* FILENAME : minion.hpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 23.11.2023
* 
*******************************************************************************/

#ifndef NSRD_MINION_HPP
#define NSRD_MINION_HPP

#include <sys/types.h>
#include <sys/socket.h>

#include <thread> // std::thread
#include <mutex> // std::timed_mutex
#include <memory> // std::shared_ptr

#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <string> // std::string
#include <cstring> // std::memcpy

#include "logger.hpp" // nsrd::Logger
#include "reactor.hpp" // nsrd::Reactor

#include "minion_event.hpp" // nsrd::MinionEventType, nsrd::MinionEvent

namespace nsrd
{
class Minion
{
public:
    explicit Minion(unsigned short port, std::size_t storage_size);
    ~Minion();
    void Run();
    void Stop();

private:
    enum PIPE_PAIR {READ_END = 0, WRITE_END, PIPE_PAIR};

    unsigned short m_minion_port;
    int m_minion_socket;
    sockaddr m_proxy_sa;
    socklen_t m_proxy_sa_len;
    int m_storage_file;
    bool m_run;
    int m_stop_reactor_pipe[PIPE_PAIR];
    Logger *m_logger;
    Reactor m_reactor;

    void OpenSocket();
    void AcceptProxyConnection();
    void ReadConnectionRequest(MinionEvent *event);
    void InputMediator();
    void StopReactorHandler();
    void Write(const MinionEvent &request);
    void Read(const MinionEvent &request);
    bool SendData(void *buf, std::size_t length);
    bool ReadData(void *buf, std::size_t length);
    void StorageRead(void *buf, unsigned len, std::size_t offset);
    void StorageWrite(void *buf, unsigned len, std::size_t offset);
};
}


#endif // NSRD_MINION_HPP