/*******************************************************************************
*
* FILENAME : dir_monitor.hpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 28.09.2023
* 
*******************************************************************************/

#ifndef NSRD_DIR_MONITOR_HPP
#define NSRD_DIR_MONITOR_HPP

#include <string> // std::string
#include <thread> // std::thread
#include <unordered_map> // std::unordered_map
#include <functional> // std::function, std::bind
#include <sys/inotify.h> // inotify

#include "dispatcher.hpp" // nsrd::Dispatcher

namespace nsrd
{
class DirMonitor
{
public:
    enum DirEventType {ADDED, REMOVED, UPDATED};
    using DirEvent = std::list<std::pair<std::string, DirEventType> >;

    explicit DirMonitor(const std::string &dir_path);
    ~DirMonitor() noexcept;
    
    DirMonitor(DirMonitor &) = delete;
    DirMonitor(DirMonitor &&) = delete;
    DirMonitor &operator=(DirMonitor &) = delete;
    DirMonitor &operator=(DirMonitor &&) = delete;

    void AttachSub(Callback<DirEvent> *sub);
    void Start(); // may throw std::runtime_error
    void Stop();
    
private:
    using ifystruct = struct inotify_event;
    using handler_func = std::function<void(const ifystruct *, DirEvent &)>;
    using time_point = std::chrono::time_point<std::chrono::system_clock>;

    void Watch();
    void HandleAdded(const ifystruct *, DirEvent &);
    void HandleRemoved(const ifystruct *, DirEvent &);
    void HandleUpdated(const ifystruct *, DirEvent &);
    void HandleClose(const ifystruct *, DirEvent &);
    void SetUpHandlers();

    Dispatcher<DirEvent> m_dispatcher;
    std::string m_dir_path;
    std::thread m_watcher;
    std::unordered_map<uint32_t, handler_func> m_handlers;
    std::unordered_map<std::string, time_point> m_cooldowns;
    bool m_is_running;
    int m_file_d;
    int m_watch_d;
};
} // namespace nsrd


#endif // NSRD_DIR_MONITOR_HPP