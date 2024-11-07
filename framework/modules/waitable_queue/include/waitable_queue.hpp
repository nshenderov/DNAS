/*******************************************************************************
*
* FILENAME : waitable_queue.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 07.09.2023
* 
*******************************************************************************/

#ifndef NSRD_WAITABLE_QUEUE_HPP
#define NSRD_WAITABLE_QUEUE_HPP

#include <queue> // std::queue
#include <mutex> // std::timed_mutex
#include <condition_variable> // std::condition_variable
#include <chrono> // std::chrono::time_point, std::chrono::system_clock

using std::chrono::nanoseconds;

namespace nsrd
{
// CONTAINER should be SequenceContainer and support:
// pop_front, push_back, front and empty functions
// T must be Copyable and CopyAssignable

template < typename T, typename CONTAINER = std::deque<T> >
class WaitableQueue
{
public:
     WaitableQueue() = default;
    ~WaitableQueue() = default;

    WaitableQueue(WaitableQueue &other) = delete;
    WaitableQueue(WaitableQueue &&other) = delete;
    WaitableQueue &operator=(WaitableQueue &other) = delete;
    WaitableQueue &operator=(WaitableQueue &&other) = delete;
    
    void Push(const T& data); 
    void Pop(T &dest);
    bool Pop(T &dest, nanoseconds timeout);
    bool IsEmpty() const;

private:
    mutable std::timed_mutex m_mutex;
    std::condition_variable_any m_has_data;
    CONTAINER m_queue;
    void Dequeue(T &dest);
};

template<class T, class CONTAINER>
void WaitableQueue<T, CONTAINER>::Push(const T &data)
{
    {
        std::unique_lock<std::timed_mutex> lock(m_mutex);
        m_queue.push_back(data);
    }
    m_has_data.notify_one();
}
template<class T, class CONTAINER>
void nsrd::WaitableQueue<T, CONTAINER>::Dequeue(T &dest)
{
    dest = m_queue.front();
    m_queue.pop_front();
}

template<class T, class CONTAINER>
void WaitableQueue<T, CONTAINER>::Pop(T &dest)
{
    std::unique_lock<std::timed_mutex> lock(m_mutex);

    while (m_queue.empty())
    {
        m_has_data.wait(lock);
    }

    Dequeue(dest);
}

template<class T, class CONTAINER>
bool WaitableQueue<T, CONTAINER>::Pop(T &dest, nanoseconds timeout)
{
    using std::chrono::system_clock;

    system_clock::time_point timeout_tp(system_clock::now() + timeout);

    std::unique_lock<std::timed_mutex> timed_lock(m_mutex, timeout_tp);
    if (!timed_lock.owns_lock())
    {
        return (false);
    }

    while (m_queue.empty())
    {
        if (std::cv_status::timeout == m_has_data.wait_until(timed_lock, timeout_tp))
        {
            return (false);
        }
    }

    Dequeue(dest);

    return (true);
}

template<class T, class CONTAINER>
bool WaitableQueue<T, CONTAINER>::IsEmpty() const
{
    std::unique_lock<std::timed_mutex> lock(m_mutex);
    return (m_queue.empty());
}
} // namespace nsrd


#endif // NSRD_WAITABLE_QUEUE_HPP