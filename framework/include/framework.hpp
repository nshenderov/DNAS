/*******************************************************************************
*
* FILENAME : framework.hpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 26.10.2023
* 
* A set of generic modules implementing the Reactor design pattern, providing
* extensibility through plugins and enabling concurrent task execution.
* 
* The framework takes commands provided by plugins and
* endpoint/event_type/handler sets. At runtime, it listens for specified events
* on designated endpoints and responds with the configured commands in a
* multithreaded environment.
* 
* Reactions to events on specific endpoints are defined by commands, which are
* provided by plugins. Plugins are dynamic libraries that offer
* commands (essentially functors) to the framework’s command factory.
* For a plugin to register with the factory, it must use the plugin_tools module
* and be located in the designated plugins folder.
* 
* To establish a new chain call AddSocketHandler, specify an endpoint, event
* type, and a function that returns a pair containing the plugin ID and a
* parameters object. These are used to construct a command that is triggered in
* the multithreaded environment each time the specified event occurs on the
* specified endpoint.
* 
*******************************************************************************/

#ifndef NSRD_FRAMEWORK_HPP
#define NSRD_FRAMEWORK_HPP

#include <atomic> // std::atomic
#include <functional> // std::function

#include "framework_factory.hpp" // concrete nsrd::Factory
#include "logger.hpp" // nsrd::Logger
#include "dir_monitor.hpp" // nsrd::DirMonitor
#include "dll_loader.hpp" // nsrd::DirLoader
#include "reactor.hpp" // nsrd::Reactor

namespace nsrd
{
class Framework
{
public:
    typedef std::function<std::pair<builder_id_t, ICommandParams * > (int fd)> socket_handler_t;

    explicit Framework(const std::string &plugin_dir_path);
    ~Framework() noexcept;

    Framework(Framework &) = delete;
    Framework(Framework &&) = delete;
    Framework &operator=(Framework &) = delete;
    Framework &operator=(Framework &&) = delete;

    void Run();  // blocking function
    void Stop(); // needs to be called asynchroniously

    void AddSocketHandler(int fd, SocketEventType type, const socket_handler_t &handler);
    void RemoveSocketHandler(int fd, SocketEventType type);

private:
    enum PIPE_PAIR {READ_END = 0, WRITE_END, PIPE_PAIR};

    Reactor m_fd_monitor;
    ThreadPool m_th_pool;
    framework_factory_t *m_commands_factory;
    DirMonitor m_plugins_dir_monitor;
    DllLoader m_plugins_loader;
    Logger *m_logger;
    std::atomic<bool> m_run;
    int m_stop_fd_monitor_pipe[PIPE_PAIR];

    void StopFdMonitorHandler();
    void SetUpDefHandlers();

};
} // namespace rd

#endif // NSRD_FRAMEWORK_HPP