/*******************************************************************************
*
* FILENAME : async_injection.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 05.10.2023
* 
*******************************************************************************/

#include <iostream> // std::cerr, std::endl

#include "async_injection.hpp" // nsrd::AsyncInjection


using namespace nsrd;

namespace
{
inline Scheduler::timepoint_t ComputeNewTimepoint(const AsyncInjection::interval_t &interval)
{
    using namespace std::chrono;
    return (system_clock::now() + duration_cast<system_clock::duration>(interval));
}
}

void AsyncInjection::Inject(const action_t &action, const interval_t &interval)
{
    new AsyncInjection(action, interval);
}

AsyncInjection::AsyncInjection(const action_t &action, const interval_t &interval)
    : m_action(action), m_interval(interval), m_task(new Task(*this))
{
    Schedule();
}

AsyncInjection::~AsyncInjection()
{}

void AsyncInjection::Schedule(bool reschedule)
{
    try
    {
        Scheduler *scheduler = Handleton<Scheduler>::GetInstance();
        scheduler->Add(m_task, ComputeNewTimepoint(m_interval));
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        DeleteSelf();
        if (!reschedule)
        {
            throw e;
        }
    }
}

void AsyncInjection::DeleteSelf()
{
    delete this;
}

/* Task */
/* ************************************************************************** */

AsyncInjection::Task::Task(AsyncInjection &injector)
    : m_injector(injector)
{}

AsyncInjection::Task::~Task()
{}

void AsyncInjection::Task::operator()()
{
    if (m_injector.m_action())
    {
        m_injector.DeleteSelf();
        return;
    }

    m_injector.Schedule(true);
}