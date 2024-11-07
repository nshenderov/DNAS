/*******************************************************************************
*
* FILENAME : pqueue.hpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 10.09.2023
* 
*******************************************************************************/

#ifndef NSRD_PRIORITY_QUEUE_HPP
#define NSRD_PRIORITY_QUEUE_HPP

#include <queue> // std::priority_queue
#include <functional> // std::less

namespace nsrd
{
// The behavior is undefined if T is not the same type as Container::value_type. 
template < typename T,
           typename CONTAINER = std::vector<T>,
           typename COMPARE = std::less<typename CONTAINER::value_type> >
class PriorityQueue
{
public:
    // PriorityQueue() = default;
    // ~PriorityQueue() = default;
    
    void push_back(const T &data);
    void pop_front();
    const T &front() const;
    bool empty() const;
    
private:
    std::priority_queue<T, CONTAINER, COMPARE> m_pqueue;
};

template <typename T, typename CONTAINER, typename COMPARE>
void PriorityQueue<T, CONTAINER, COMPARE>::push_back(const T &data)
{
    m_pqueue.push(data);
}

template <typename T, typename CONTAINER, typename COMPARE>
void PriorityQueue<T, CONTAINER, COMPARE>::pop_front()
{
    m_pqueue.pop();
}

template <typename T, typename CONTAINER, typename COMPARE>
const T &PriorityQueue<T, CONTAINER, COMPARE>::front() const
{
    return (m_pqueue.top());
}

template <typename T, typename CONTAINER, typename COMPARE>
bool PriorityQueue<T, CONTAINER, COMPARE>::empty() const
{
    return (m_pqueue.empty());
}
} // namespace nsrd

#endif // NSRD_PRIORITY_QUEUE_HPP