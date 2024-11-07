/*******************************************************************************
*
* FILENAME : thread_pool.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 14.09.2023
* 
*******************************************************************************/

#include "thread_pool.hpp"

using namespace nsrd;

/* ThreadPool's private nested classes */
/* ************************************************************************** */
class ThreadPool::TaskWrapper
{
public:
    TaskWrapper();
    explicit TaskWrapper(std::shared_ptr<Task> task, ThreadPool::Priority priority);

    void operator()() const;
    bool operator<(const TaskWrapper &other) const;

private:
    std::shared_ptr<Task> m_task;
    Priority m_priority;
};

class ThreadPool::ThreadWrapper
{
public:
    enum Status { ALIVE = 0, ASLEEP = 1, DEAD = 2 };

    explicit ThreadWrapper(ThreadPool &pool);
    ~ThreadWrapper() noexcept;

private:
    Status m_status;
    ThreadPool &m_pool;
    std::thread m_thread;

    void ThreadWorker();
    void PerformTask();

friend class ThreadPool;
};
class ThreadPool::IdleAction : public Task
{
public:
    IdleAction(const ThreadPool &pool){(void) pool;};
    void operator()(){};
private:
};
class ThreadPool::SleepAction : public Task
{
public:
    SleepAction(const ThreadPool &pool);
    void operator()();
private:
    const ThreadPool &m_pool;

    class Sedative : public std::exception
    {
    public:
        std::string what(){return ("Fell asleep");}
    };

friend class ThreadWrapper;
};
class ThreadPool::DieAction : public Task
{
public:
    DieAction(const ThreadPool &pool);
    void operator()();
private:
    const ThreadPool &m_pool;

    class Poison : public std::exception
    {
    public:
        std::string what(){return ("Got poisoned");}
    };

friend class ThreadWrapper;
};

/* Helpers */
/* ************************************************************************** */
namespace
{
void SemPostMultiple(interprocess_semaphore &m_sem, std::size_t amount)
{
    for (std::size_t i = 0; i < amount; ++i)
    {
        m_sem.post();
    }
}

void SemWaitMultiple(interprocess_semaphore &m_sem, std::size_t amount)
{
    for (std::size_t i = 0; i < amount; ++i)
    {
        m_sem.wait();
    }
}
} // namespace


/* ThreadPool */
/* ************************************************************************** */

ThreadPool::ThreadPool(std::size_t num_of_threads)
    : m_tasks(),
      m_threads(), 
      m_actions(),
      m_pause_sem(0),
      m_death_sem(0),
      m_run(true),
      m_pause(false)
{
    m_actions[IDLE] = std::shared_ptr<Task>(new IdleAction(*this));
    m_actions[SLEEP] = std::shared_ptr<Task>(new SleepAction(*this));
    m_actions[DIE] = std::shared_ptr<Task>(new DieAction(*this));

    AddThreads(num_of_threads);
}

ThreadPool::~ThreadPool() noexcept
{
    m_run = false;
    TakeAction(IDLE, Priority(LOWEST), m_threads.size());
    Resume();
}

void ThreadPool::Add(std::shared_ptr<Task> task, ThreadPool::Priority priority)
{
    TaskWrapper wrapped_task(task, priority);
    m_tasks.Push(wrapped_task);
}

void ThreadPool::Pause()
{
    m_pause = true;
    TakeAction(SLEEP, Priority(HIGHEST), m_threads.size());
}

void ThreadPool::Resume()
{
    m_pause = false;
    SemPostMultiple(m_pause_sem, m_threads.size());
}

void ThreadPool::Stop()
{
    SetNumOfThreads(0);
    TakeTasksOut();
    if (m_pause)
    {
        Resume();
    }
}

void ThreadPool::SetNumOfThreads(std::size_t num_of_threads)
{
    if (m_threads.size() < num_of_threads)
    {
        IncreaseThreads(num_of_threads - m_threads.size());
    }
    else if (m_threads.size() > num_of_threads)
    {
        DecreaseThreads(m_threads.size() - num_of_threads);
    }
}

void ThreadPool::AddThreads(std::size_t num_of_threads)
{
    for (size_t i = 0; i < num_of_threads; ++i)
    {
        m_threads.push_back(std::shared_ptr<ThreadWrapper>(new ThreadWrapper(*this)));
    }
}

void ThreadPool::IncreaseThreads(std::size_t threads_to_add)
{
    if (m_pause)
    {
        TakeAction(SLEEP, Priority(HIGHEST), threads_to_add);
    }

    AddThreads(threads_to_add);
}

void ThreadPool::DecreaseThreads(std::size_t threads_to_kill)
{
    TakeAction(DIE, Priority(HIGHEST), threads_to_kill);

    if (m_pause)
    {
        SemPostMultiple(m_pause_sem, threads_to_kill);
    }

    SemWaitMultiple(m_death_sem, threads_to_kill);

    m_threads.remove_if(IsThreadDead);
}

bool ThreadPool::IsThreadDead(const std::shared_ptr<ThreadWrapper> &thread)
{
    return (ThreadWrapper::DEAD == thread->m_status);
}

void ThreadPool::TakeAction(Action action_type, Priority priority, std::size_t amount)
{
    TaskWrapper wrapped_task(m_actions[action_type], priority);

    for (std::size_t i = 0; i < amount; ++i)
    {
        m_tasks.Push(wrapped_task);
    }
}

void ThreadPool::TakeTasksOut()
{
    TaskWrapper dummy;
    while (!m_tasks.IsEmpty())
    {
        m_tasks.Pop(dummy);
    }
}

/* Task */
/* ************************************************************************** */

Task::~Task() noexcept
{}

/* TaskWrapper */
/* ************************************************************************** */

ThreadPool::TaskWrapper::TaskWrapper()
{}

ThreadPool::TaskWrapper::TaskWrapper(std::shared_ptr<Task> task, ThreadPool::Priority priority)
    : m_task(task), m_priority(priority)
{}

void ThreadPool::TaskWrapper::operator()() const
{
    (*m_task)();
}

bool ThreadPool::TaskWrapper::operator<(const ThreadPool::TaskWrapper &other) const
{
    return (m_priority < other.m_priority);
}

/* ThreadWrapper */
/* ************************************************************************** */

ThreadPool::ThreadWrapper::ThreadWrapper(ThreadPool &pool)
    :  m_status(ALIVE),
       m_pool(pool),
       m_thread(std::thread(&ThreadPool::ThreadWrapper::ThreadWorker, this))
{}

ThreadPool::ThreadWrapper::~ThreadWrapper() noexcept
{
    m_thread.join();
}

void ThreadPool::ThreadWrapper::ThreadWorker()
{
    while (m_pool.m_run)
    {
        try
        {
            PerformTask();
        }
        catch(SleepAction::Sedative &p)
        {
            m_status = ASLEEP;
            m_pool.m_pause_sem.wait();
            m_status = ALIVE;
        }
        catch(DieAction::Poison &p)
        {
            m_status = DEAD;
            m_pool.m_death_sem.post();
            break;
        }
    }
}

void ThreadPool::ThreadWrapper::PerformTask()
{
    TaskWrapper task;
    m_pool.m_tasks.Pop(task);
    task();
}

/* Action */
/* ************************************************************************** */

/* Sleep */

nsrd::ThreadPool::SleepAction::SleepAction(const ThreadPool &pool)
    : m_pool(pool)
{}

void ThreadPool::SleepAction::operator()()
{
    throw Sedative();
}

/* Die */

ThreadPool::DieAction::DieAction(const ThreadPool &pool)
    : m_pool(pool)
{}

void ThreadPool::DieAction::operator()()
{
    throw Poison();
}

#ifndef NDEBUG
/* Debug */
/* ************************************************************************** */

std::size_t ThreadPool::GetNThreads()
{
    return (m_threads.size());
}

bool ThreadPool::QIsEmpty()
{
    return (m_tasks.IsEmpty());
}
#endif