/*******************************************************************************
*
* FILENAME : reactor.cpp
*
* AUTHOR : Nick Shenderov
*
* DATE : 01.10.2023
*
*******************************************************************************/

#include <functional> // std::hash
#include <iostream>
#include "reactor.hpp" // nsrd::Reactor

using namespace nsrd;

Reactor::Reactor()
    : m_fd_func_map(), m_listener(), m_is_running(false)
{}

Reactor::~Reactor() noexcept
{}

void Reactor::Add(int fd, const Reactor::Handler &handler, SocketEventType type)
{
    m_fd_func_map.insert({{fd, type}, handler});
    m_listener.Add(fd, type);
}

void Reactor::Remove(int fd, SocketEventType type) noexcept
{
    m_fd_func_map.erase({fd, type});
    m_listener.Remove(fd, type);
}

void Reactor::Run()
{
    if (!m_is_running)
    {
        m_is_running = true;
        while (m_is_running)
        {
            m_fd_func_map[m_listener.Listen()]();
        }
    }
}

void Reactor::Stop()
{
    m_is_running = false;
}

std::size_t Reactor::PairHash::operator()(const std::pair<int, SocketEventType> &p) const noexcept
{
    return std::hash<int>()(p.first) ^ std::hash<SocketEventType>()(p.second);
}