/*******************************************************************************
*
* FILENAME : scheduler.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 03.10.2023
* 
*******************************************************************************/

#ifndef NSRD_SCHEDULER_HPP
#define NSRD_SCHEDULER_HPP

#include <memory> //std::shared_ptr
#include <chrono> // std::chrono::time_point, std::chrono::system_clock

#include "pqueue.hpp" // nsrd::PriorityQueue
#include "waitable_queue.hpp" // nsrd::WaitableQueue
#include "handleton.hpp" // nsrd::Handleton

#include <csignal>

namespace nsrd
{
class Scheduler
{
public:
    typedef std::chrono::time_point<std::chrono::system_clock> timepoint_t;
    
    class Task
    {
    public:
        Task() = default;
        virtual ~Task() noexcept =0;
        virtual void operator()() =0;
    };

    Scheduler(Scheduler &other) = delete;
    Scheduler(Scheduler &&other) = delete;
    Scheduler &operator=(Scheduler &other) = delete;
    Scheduler &operator=(Scheduler &&other) = delete;

    void Add(std::shared_ptr<Task> task, timepoint_t deadline);

private:
    friend class Handleton<Scheduler>;

    Scheduler();
    ~Scheduler() noexcept;
};
}

#endif // NSRD_SCHEDULER_HPP       