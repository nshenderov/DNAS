/*******************************************************************************
*
* FILENAME : thread_pool.hpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 14.09.2023
* 
*******************************************************************************/

#ifndef NSRD_THREAD_POOL_HPP
#define NSRD_THREAD_POOL_HPP

#include <cstddef> // std::size_t
#include <thread> // std::thread
#include <memory> // std::shared_ptr
#include <list> // std::list
#include <boost/interprocess/sync/interprocess_semaphore.hpp> 
                // boost::interprocess::interprocess_semaphore

#include "pqueue.hpp"
#include "waitable_queue.hpp"

#include "task.hpp"

namespace nsrd
{
using boost::interprocess::interprocess_semaphore;

class ThreadPool
{
private:
    class TaskWrapper;
    class ThreadWrapper;
    class SleepAction;
    class DieAction;
    class IdleAction;

public:
    enum Priority { LOW = 0, MEDIUM = 1, HIGH = 2 };

    explicit ThreadPool(std::size_t num_of_threads = std::thread::hardware_concurrency()); 
    ~ThreadPool() noexcept;

    ThreadPool(ThreadPool &other) = delete;
    ThreadPool(ThreadPool &&other) = delete;
    ThreadPool &operator=(ThreadPool &other) = delete;
    ThreadPool &operator=(ThreadPool &&other) = delete;

    void SetNumOfThreads(std::size_t num_of_threads);
    void Add(std::shared_ptr<Task> task, Priority priority);
    void Pause();
    void Resume();
    void Stop();

#ifndef NDEBUG
    std::size_t GetNThreads();
    bool QIsEmpty();
#endif

private:
    enum ExtraPriority { LOWEST = -1, HIGHEST = 3 };
    enum Action { IDLE = 0, SLEEP = 1, DIE = 2, ACTIONS_AMOUNT };

    WaitableQueue<TaskWrapper, PriorityQueue<TaskWrapper> > m_tasks;
    std::list<std::shared_ptr<ThreadWrapper> > m_threads;
    std::shared_ptr<Task> m_actions[ACTIONS_AMOUNT];

    mutable interprocess_semaphore m_pause_sem;
    mutable interprocess_semaphore m_death_sem;

    bool m_run;
    bool m_pause;

    void AddThreads(std::size_t num_of_threads);
    void IncreaseThreads(std::size_t num_of_threads);
    void DecreaseThreads(std::size_t num_of_threads);
    static bool IsThreadDead(const std::shared_ptr<ThreadWrapper> &thread);
    void TakeTasksOut();
    void TakeAction(Action action_type, Priority priority, std::size_t amount);
};
} //namespace nsrd

#endif // NSRD_THREAD_POOL_HPP
