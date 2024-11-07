/*******************************************************************************
*
* FILENAME : reactor.hpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 01.10.2023
* 
*******************************************************************************/

#ifndef NSRD_REACTOR_HPP
#define NSRD_REACTOR_HPP

#include <utility> // std::pair
#include <unordered_map> // std::unordered_map
#include <cstddef> // std::size_t
#include <functional> // std::function

#include "listener.hpp" // nsrd::Listener
#include "socket_event_type.hpp" // nsrd::SocketEventType

namespace nsrd
{
class Reactor
{
public:
    using Handler = std::function<void()>;
    
    explicit Reactor();
    ~Reactor() noexcept;
    
    Reactor(const Reactor &other) = delete;
    Reactor(const Reactor &&other) = delete;
    Reactor &operator=(const Reactor &other) = delete;
    Reactor &operator=(const Reactor &&other) = delete;

    void Add(int fd, const Handler &handler, SocketEventType type);
    void Remove(int fd, SocketEventType type) noexcept;
    
    void Run();
    void Stop();

private:
    using Event = std::pair<int, SocketEventType>;

    struct PairHash{std::size_t operator()(const Event &p) const noexcept;};

    std::unordered_map<Event, Handler, PairHash> m_fd_func_map;
    Listener m_listener;
    bool m_is_running;
};
} // namepsace nsrd

#endif // NSRD_REACTOR_HPP
