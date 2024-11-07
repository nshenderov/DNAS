/*******************************************************************************
*
* FILENAME : dispather.hpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 27.09.2023
* 
*******************************************************************************/

#ifndef NSRD_DISPATCHER_HPP
#define NSRD_DISPATCHER_HPP

#include <list> // std::list
#include <cassert> // assert

namespace nsrd
{
template <typename EVENT>
class Callback;

template <typename EVENT>
class Dispatcher;

/* Callback */
/* ************************************************************************** */

// EVENT must be Copyable and CopyAssignable
template <typename EVENT>
class Callback
{
public:
    Callback() = default;
    virtual ~Callback() =0;
    
    Callback(Callback &other) = delete;
    Callback(Callback &&other) = delete;
    Callback &operator=(Callback &other) = delete;
    Callback &operator=(Callback &&other) = delete;

protected:
    void DispatcherSubscribe(Callback<EVENT> *callback);
    void DispatcherUnsubscribe();

private:
    friend class Dispatcher<EVENT>;
    
    virtual void Notify(EVENT event) =0;
    virtual void NotifyOnDeath() =0;
    void DefaultNotifyOnDeath();
    void SetDispatcher(Dispatcher<EVENT> *dispatcher);

    Dispatcher<EVENT> *m_dispatcher = nullptr;
};

template <typename EVENT>
Callback<EVENT>::~Callback()
{
    if (nullptr != m_dispatcher)
    {
        m_dispatcher->Unsubscribe(this);
    }
}

template <typename EVENT>
void Callback<EVENT>::DefaultNotifyOnDeath()
{
    NotifyOnDeath();
}

template <typename EVENT>
void Callback<EVENT>::SetDispatcher(Dispatcher<EVENT> *dispatcher)
{
    m_dispatcher = dispatcher;
}

template <typename EVENT>
void Callback<EVENT>::DispatcherSubscribe(Callback<EVENT> *callback)
{
    if (nullptr != m_dispatcher)
    {
        m_dispatcher->Subscribe(callback);
    }
}

template <typename EVENT>
void Callback<EVENT>::DispatcherUnsubscribe()
{
    if (nullptr != m_dispatcher)
    {
        m_dispatcher->Unsubscribe(this);
    }
}

/* Dispatcher */
/* ************************************************************************** */

// EVENT must be Copyable and CopyAssignable
template <typename EVENT>
class Dispatcher
{
public:
    Dispatcher() = default;
    ~Dispatcher();

    void Subscribe(Callback<EVENT> *sub);
    void Broadcast(EVENT event);
    
    Dispatcher(Dispatcher &other) = delete;
    Dispatcher(Dispatcher &&other) = delete;
    Dispatcher &operator=(Dispatcher &other) = delete;
    Dispatcher &operator=(Dispatcher &&other) = delete;

private:
    friend class Callback<EVENT>;

    void Unsubscribe(Callback<EVENT> *sub);

    std::list<Callback<EVENT> *> m_callbacks;
};

template <typename EVENT>
void Dispatcher<EVENT>::Subscribe(Callback<EVENT> *sub)
{
    assert(nullptr != sub);

    m_callbacks.push_front(sub);
    sub->SetDispatcher(this);
}

template <typename EVENT>
void Dispatcher<EVENT>::Broadcast(EVENT event)
{
    for (auto it = m_callbacks.begin(), end = m_callbacks.end(); it != end;)
    {
        auto curr = it++;
        (*curr)->Notify(event);
    }
}

template <typename EVENT>
void Dispatcher<EVENT>::Unsubscribe(Callback<EVENT> *sub)
{
    assert(nullptr != sub);
    sub->SetDispatcher(nullptr);
    m_callbacks.remove(sub);
}

template <typename EVENT>
Dispatcher<EVENT>::~Dispatcher()
{
    for (auto it = m_callbacks.begin(), end = m_callbacks.end(); it != end;)
    {
        auto curr = it++;
        (*curr)->DefaultNotifyOnDeath();
        Unsubscribe(*curr);
    }
}
} // namespace nsrd


#endif // NSRD_DISPATCHER_HPP