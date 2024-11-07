/*******************************************************************************
*
* FILENAME : listener.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 01.10.2023
* 
*******************************************************************************/

#include <iostream> // std::cerr, std::endl
#include <cstring> // std::strerror
#include <cerrno> // errno
#include <cassert> // assert

#include "listener.hpp" // nsrd::Listener

using namespace nsrd;

Listener::Listener()
    : m_listened(), m_ready_list()
{}

Listener::~Listener()
{}

void Listener::Add(int fd, SocketEventType type)
{
    m_listened.insert({fd, type});
}

void Listener::Remove(int fd, SocketEventType type) noexcept
{
    m_ready_list.remove({fd, type});
    m_listened.erase({fd, type});
}

Listener::Event Listener::Listen()
{
    if (0 == m_ready_list.size())
    {
        Select();
    }

    return (PeekAndPopEvent());
}

void Listener::Select()
{
    fd_set sets[SocketEventType::SOCKET_EVENT_TYPES] = {0};

    FillUpSets(sets);

    int nready = 0;
    while (0 >= nready)
    {
        nready = select(GetMaxFd(), 
                        &sets[SocketEventType::READ], 
                        &sets[SocketEventType::WRITE], 
                        &sets[SocketEventType::EXCEPTION],
                        nullptr);

        if (-1 == nready)
        {
            HandleSelectError();
        }
    }

    PushToReadyList(sets, nready);
}

void Listener::PushToReadyList(fd_set *sets, int nready)
{
    for (auto event = m_listened.begin(); event != m_listened.end() && nready; ++event)
    {
        if (FD_ISSET(event->first, &sets[event->second]))
        {
            m_ready_list.push_back(*event);
            FD_CLR(event->first, &sets[event->second]);
            --nready;
        }
    }
}

void Listener::FillUpSets(fd_set *sets)
{
    for (int i = 0; i < SocketEventType::SOCKET_EVENT_TYPES; ++i)
    {
        FD_ZERO(&sets[i]);
    }
    
    for (auto fd : m_listened)
    {
        FD_SET(fd.first, &sets[fd.second]);
    }
}

Listener::Event Listener::PeekAndPopEvent()
{
    Event event = m_ready_list.back();
    m_ready_list.pop_back();
    return (event);
}

void Listener::HandleSelectError()
{
    if (EINTR == errno)
    {
        return;
    }
    else if (EBADF == errno)
    {
        throw std::invalid_argument("Invalid file descriptor detected");
    }
    else
    {
        std::cerr << "Select error: " << std::strerror(errno) << std::endl;
    }
}

int Listener::GetMaxFd()
{
    assert(!m_listened.empty());
    return ((*(--m_listened.end())).first + 1);
}