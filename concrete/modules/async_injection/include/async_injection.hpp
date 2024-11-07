/*******************************************************************************
*
* FILENAME : async_injection.hpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 05.10.2023
* 
*******************************************************************************/

#ifndef NSRD_ASYNCINJECTION_HPP
#define NSRD_ASYNCINJECTION_HPP

#include <functional> // std::function
#include <chrono> // std::chrono::time_point, std::chrono::system_clock

#include "handleton.hpp" // nsrd::Handleton
#include "scheduler.hpp" // nsrd::Scheduler

namespace nsrd
{
class AsyncInjection
{
public:
    typedef std::chrono::duration<double, std::milli> interval_t;
    typedef std::function<bool()> action_t;

    static void Inject(const action_t &action, const interval_t &interval);
    
    AsyncInjection(AsyncInjection &other) = delete;
    AsyncInjection(AsyncInjection &&other) = delete;
    AsyncInjection &operator=(AsyncInjection &other) = delete;
    AsyncInjection &operator=(AsyncInjection &&other) = delete;
    
private:
    class Task : public Scheduler::Task
    {
    public:
        explicit Task(AsyncInjection &injector);
        ~Task();
        
        void operator()();
        
    private:
        AsyncInjection &m_injector;
    };

    typedef std::shared_ptr<Scheduler::Task> task_sp_t;
    
    explicit AsyncInjection(const action_t &action, const interval_t &interval);
    ~AsyncInjection();
    
    void Schedule(bool reschedule = false);
    void DeleteSelf();
    
    action_t m_action;
    interval_t m_interval;
    task_sp_t m_task;
};
} // namespace nsrd

#endif // NSRD_ASYNCINJECTION_HPP  
  
