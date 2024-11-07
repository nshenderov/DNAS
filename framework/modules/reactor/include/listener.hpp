/*******************************************************************************
*
* FILENAME : listener.hpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 01.10.2023
* 
*******************************************************************************/

#ifndef NSRD_LISTENER_HPP
#define NSRD_LISTENER_HPP

#include <utility> // std::pair
#include <list> // std::list
#include <set> // std::set
#include <sys/select.h> // select
#include <time.h> // time

#include "socket_event_type.hpp" // nsrd::SocketEventType

namespace nsrd
{
class Listener
{
public:
    using Event = std::pair<int, SocketEventType>;
    using ReadyList = std::list<Event>;

    explicit Listener();
    ~Listener() noexcept;

    Listener(const Listener &other) = delete;
    Listener(const Listener &&other) = delete;
    Listener &operator=(const Listener &other) = delete;
    Listener &operator=(const Listener &&other) = delete;
    
    void Add(int fd, SocketEventType type);
    void Remove(int fd, SocketEventType type) noexcept;
    
    Event Listen();
    
private:
    std::set<Event> m_listened;
    ReadyList m_ready_list;

    void Select();
    void HandleSelectError();
    void FillUpSets(fd_set *sets);
    void PushToReadyList(fd_set *sets, int nready);
    int GetMaxFd();
    Event PeekAndPopEvent();
};
} // namepsace nsrd

#endif // NSRD_LISTENER_HPP
