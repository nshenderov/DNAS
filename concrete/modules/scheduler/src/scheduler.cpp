/*******************************************************************************
*
* FILENAME : scheduler.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 03.10.2023
* 
*******************************************************************************/

#include <iostream> // std::cerr, std::endl
#include <cstring> // std::strerror(errno), std::memset
#include <time.h> // timer_create, timer_delete

#include "scheduler.hpp" // nsrd::Scheduler

using namespace nsrd;

namespace
{
typedef std::pair<timer_t, std::shared_ptr<Scheduler::Task> > scheduled_task_t;
typedef Scheduler::Task task_t;

void TimerExpHandler(int sig, siginfo_t *si, void *uc)
{
    auto scheduled_task = reinterpret_cast<scheduled_task_t*>(si->si_value.sival_ptr);

    auto task = scheduled_task->second;

    if (-1 == timer_delete(scheduled_task->first))
    {
        std::cerr << "TimerExpHandler: " << std::strerror(errno) << std::endl;
    }
    
    delete scheduled_task;

    task->operator()();

    (void) sig;
    (void) uc;
}

timespec TimepointToTimespec(Scheduler::timepoint_t tp)
{
    using namespace std::chrono;
    using nsec = std::chrono::nanoseconds;
    using sec = std::chrono::seconds;

    auto secs = time_point_cast<sec>(tp);
    auto ns = time_point_cast<nsec>(tp) - time_point_cast<nsec>(secs);

    return (timespec{secs.time_since_epoch().count(), ns.count()});
}

void InitSegevent(struct sigevent *sev, scheduled_task_t *scheduled_task)
{
    std::memset(sev, 0, sizeof(struct sigevent));
    sev->sigev_notify = SIGEV_SIGNAL;
    sev->sigev_signo = SIGRTMIN;

    sev->sigev_value.sival_ptr = scheduled_task;
}

void InitItimerspec(struct itimerspec *its, Scheduler::timepoint_t &deadline)
{
    std::memset(its, 0, sizeof(struct itimerspec));
    its->it_value = TimepointToTimespec(deadline);
}

void SetupSigHandler()
{
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    sa.sa_sigaction = TimerExpHandler;
    sigemptyset(&sa.sa_mask);

    if (0 != sigaction(SIGRTMIN, &sa, nullptr))
    {
        throw std::runtime_error("SetupSigHandler failed");
    }
}
} // namespace anonimus

/* Scheduler */
/* ************************************************************************** */

Scheduler::Scheduler()
{
    SetupSigHandler();
}

Scheduler::~Scheduler()
{}

void Scheduler::Add(std::shared_ptr<Task> task, timepoint_t deadline)
{
    auto scheduled_task = new scheduled_task_t({nullptr, task});

    struct sigevent sev;
    InitSegevent(&sev, scheduled_task);

    if (0 != timer_create(CLOCK_REALTIME, &sev, &scheduled_task->first))
    {
        throw std::runtime_error(std::strerror(errno));
    }
    
    struct itimerspec its;
    InitItimerspec(&its, deadline);

    if (0 != timer_settime(scheduled_task->first, TIMER_ABSTIME, &its, nullptr))
    {
        throw std::runtime_error(std::strerror(errno));
    }

}

Scheduler::Task::~Task()
{}