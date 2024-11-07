/*******************************************************************************
*
* FILENAME : task.hpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 14.09.2023
* 
*******************************************************************************/

#ifndef NSRD_TASK_HPP
#define NSRD_TASK_HPP

#include <functional> // std::funtion
#include <boost/interprocess/sync/interprocess_semaphore.hpp> 
               // boost::interprocess::interprocess_semaphore

namespace nsrd
{
using boost::interprocess::interprocess_semaphore;

class Task
{
public:
    virtual ~Task() noexcept =0;
    virtual void operator()() =0;
};

// T must be Copyable and CopyAssignable, has Default constructor
template<typename T>
class FutureTask : public Task
{
public:
    explicit FutureTask(std::function<T()> &func);
    
    void Wait() const;
    const T &Get() const;
    bool IsReady() const;

    void operator()();

private:
    T m_value;
    mutable interprocess_semaphore m_sem;
    bool m_is_ready;
    std::function<T()> m_function;
};

template<typename T>
FutureTask<T>::FutureTask(std::function<T()> &func)
    : m_value(0), m_sem(0), m_is_ready(false), m_function(func)
{}

template<typename T>
void FutureTask<T>::Wait() const
{
    m_sem.wait();
    m_sem.post();
}

template<typename T>
const T &FutureTask<T>::Get() const
{
    Wait();

    return (m_value);
}

template<typename T>
bool FutureTask<T>::IsReady() const
{
    return (m_is_ready);
}

template<typename T>
void FutureTask<T>::operator()()
{
    m_value = m_function();
    m_is_ready = true;
    m_sem.post();
}
} // namespace nsrd

#endif // NSRD_TASK_HPP