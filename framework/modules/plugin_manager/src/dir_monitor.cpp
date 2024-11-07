/*******************************************************************************
*
* FILENAME : dir_monitor.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 28.09.2023
* 
*******************************************************************************/

#include <unistd.h> // read, close
#include <cassert> // assert
#include <sys/inotify.h> // inotify
#include <stdexcept> // std::runtime_error
#include <iostream> // std::cerr, std::cout, std::endl

#include <pthread.h> // pthread_sigmask
#include <signal.h> // sigemptyset, sigaddset

#include "dir_monitor.hpp" // nsrd::DirMonitor

using namespace nsrd;

namespace details
{
bool IsSOFormat(const std::string &dll_path)
{
    std::size_t dot = dll_path.find_last_of('.');
    
    if (dot == std::string::npos || "so" != dll_path.substr(dot + 1))
    {
        return (false);
    }

    return (true);
}
} // namespace details


DirMonitor::DirMonitor(const std::string &dir_path)
    : m_dispatcher(),
      m_dir_path(dir_path),
      m_watcher(),
      m_handlers(),
      m_cooldowns(),
      m_is_running(false),
      m_file_d(0),
      m_watch_d(0)
{
    SetUpHandlers();
}

DirMonitor::~DirMonitor()
{
    Stop();
}

void DirMonitor::SetUpHandlers()
{
    using std::placeholders::_1;
    using std::placeholders::_2;

    m_handlers[IN_CREATE] = std::bind(&DirMonitor::HandleAdded, this, _1, _2);
    m_handlers[IN_MOVED_TO] = std::bind(&DirMonitor::HandleAdded, this, _1, _2);

    m_handlers[IN_DELETE] = std::bind(&DirMonitor::HandleRemoved, this, _1, _2);
    m_handlers[IN_MOVED_FROM] = std::bind(&DirMonitor::HandleRemoved, this, _1, _2);

    // m_handlers[IN_MODIFY] = std::bind(&DirMonitor::HandleUpdated, this, _1, _2);
    // m_handlers[IN_ATTRIB] = std::bind(&DirMonitor::HandleUpdated, this, _1, _2);

    m_handlers[IN_IGNORED] = std::bind(&DirMonitor::HandleClose, this, _1, _2);
}

void DirMonitor::AttachSub(Callback<DirMonitor::DirEvent> *sub)
{
    assert(nullptr != sub);

    m_dispatcher.Subscribe(sub);
}

void DirMonitor::Start()
{
    m_is_running = true;

    m_file_d = inotify_init();
    if (0 > m_file_d)
    {
        throw std::runtime_error("inotify_init");
    }

    m_watch_d = inotify_add_watch(m_file_d, m_dir_path.c_str(),
                                        IN_CREATE | IN_MOVED_TO |
                                        IN_DELETE | IN_MOVED_FROM |
                                        // IN_MODIFY | IN_ATTRIB |
                                        IN_IGNORED);

    if (-1 == m_watch_d)
    {
        throw std::runtime_error("inotify_add_watch");
    }

    m_watcher = std::thread(&DirMonitor::Watch, this);
}

void DirMonitor::Stop()
{
    if (m_is_running)
    {
        m_is_running = false;

        if (-1 == inotify_rm_watch(m_file_d, m_watch_d) || -1 == close(m_file_d))
        {
            throw std::runtime_error("inotify_rm_watch || close");
        }
        
        m_watcher.join();
    }
}

void DirMonitor::HandleAdded(const ifystruct *event, DirEvent &events_to_broadcast)
{
    if (!details::IsSOFormat(event->name))
    {
        return;
    }
    
    #ifdef PRINT_DEBUG
    std::cout << "[added] " << event->name << std::endl;
    #endif

    m_cooldowns[event->name] = std::chrono::system_clock::now() + std::chrono::seconds(2);
    events_to_broadcast.push_back({m_dir_path + event->name, ADDED});
}

void DirMonitor::HandleRemoved(const ifystruct *event, DirEvent &events_to_broadcast)
{
    if (!details::IsSOFormat(event->name))
    {
        return;
    }

    auto it = m_cooldowns.find(event->name);
    if (it != m_cooldowns.end() && std::chrono::system_clock::now() >= m_cooldowns[event->name])
    {
        m_cooldowns.erase(event->name);
    }

    #ifdef PRINT_DEBUG
    std::cout << "[removed] " << event->name << std::endl;
    #endif

    events_to_broadcast.push_back({m_dir_path + event->name, REMOVED});
}

void DirMonitor::HandleUpdated(const ifystruct *event, DirEvent &events_to_broadcast)
{
    if (!details::IsSOFormat(event->name))
    {
        return;
    }
    
    auto it = m_cooldowns.find(event->name);
    if (it != m_cooldowns.end() && std::chrono::system_clock::now() < m_cooldowns[event->name])
    {
        return;
    }

    #ifdef PRINT_DEBUG
    std::cout << "[updated] " << event->name << std::endl;
    #endif

    events_to_broadcast.push_back({m_dir_path + event->name, UPDATED});
}

void DirMonitor::HandleClose(const ifystruct *event, DirEvent &events_to_broadcast)
{
    #ifdef PRINT_DEBUG
    std::cout << "Watch thread: Grace exit" << std::endl;
    #endif

    (void) event;
    (void) events_to_broadcast;
}

void DirMonitor::Watch()
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGRTMIN);

    if (0 != pthread_sigmask(SIG_BLOCK, &set, nullptr))
    {
        std::cerr << "[Error] ServerDriver: pthread_sigmask error" << std::endl;
    }

    const std::size_t IFYSIZE = sizeof(ifystruct);
    const std::size_t MAX_EVENTS_AT_A_TIME = 256UL;
    const std::size_t LEN = MAX_EVENTS_AT_A_TIME * sizeof(ifystruct);

    std::array<char, LEN> buf;

    while (m_is_running)
    {
        DirEvent events_to_broadcast;

        std::ptrdiff_t readed = read(m_file_d, buf.begin(), LEN);
        if (-1 == readed && EAGAIN != errno)
        {
            std::cerr << "DirMonitor read failed" << std::endl;
        }

        for (char *run = buf.begin(), *end = run + readed; run < end;)
        {
            ifystruct *event = reinterpret_cast<ifystruct *>(run);
            m_handlers[event->mask](event, events_to_broadcast);
            run += IFYSIZE + event->len;
        }

        if (0 < events_to_broadcast.size())
        {
            m_dispatcher.Broadcast(events_to_broadcast);
        }
    }
}