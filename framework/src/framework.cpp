/*******************************************************************************
*
* FILENAME : framework.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 26.10.2023
* 
*******************************************************************************/

#include "framework.hpp" // nsrd::Framework

using namespace nsrd;

Framework::Framework(const std::string &plugin_dir_path)
    : m_fd_monitor(),
      m_th_pool(),
      m_commands_factory(Handleton<framework_factory_t>::GetInstance()),
      m_plugins_dir_monitor(plugin_dir_path),
      m_plugins_loader(),
      m_logger(Handleton<Logger>::GetInstance()),
      m_run(false),
      m_stop_fd_monitor_pipe()
{
    m_plugins_dir_monitor.AttachSub(m_plugins_loader.GetCallback());
    m_plugins_dir_monitor.Start();
    m_logger->Info("Framework has been created");

    if (-1 == pipe(m_stop_fd_monitor_pipe))
    {
        m_logger->Error("Couldn't open closing pipe");
        throw::std::runtime_error("Couldn't open closing pipe");
    }

    SetUpDefHandlers();
}

Framework::~Framework() noexcept
{
    Stop();
    m_plugins_dir_monitor.Stop();
    close(m_stop_fd_monitor_pipe[READ_END]);
    close(m_stop_fd_monitor_pipe[WRITE_END]);
    m_logger->Info("Framework destroying");
}

void Framework::Run()
{
    if (!m_run)
    {
        m_run = true;
        m_logger->Info("Starting the framework");
        m_fd_monitor.Run();
    }
}

void Framework::Stop()
{
    if (m_run)
    {
        m_run = false;
        m_logger->Info("Stopping the framework");
        if (-1 == write(m_stop_fd_monitor_pipe[WRITE_END], "", 1))
        {
            m_logger->Error("Got error stopping monitor");
        }
    }
}

void Framework::SetUpDefHandlers()
{
    m_fd_monitor.Add(m_stop_fd_monitor_pipe[READ_END],
                    std::bind(&Framework::StopFdMonitorHandler, this),
                    SocketEventType::READ);
}

void Framework::StopFdMonitorHandler()
{
    char buf[1];
    if (-1 == read(m_stop_fd_monitor_pipe[READ_END], buf, 1))
    {
        m_logger->Error("Reading closing pipe");
    }

    m_fd_monitor.Stop();
}

class CommandTask : public Task
{
public:
    explicit CommandTask(std::shared_ptr<ICommand> command);
    ~CommandTask() = default;
    void operator()();
private:
    std::shared_ptr<ICommand> m_concrete_command;
};

CommandTask::CommandTask(std::shared_ptr<ICommand> command)
    : m_concrete_command(command)
{}

void CommandTask::operator()()
{
    (*m_concrete_command)();
}

void Framework::AddSocketHandler(int fd, SocketEventType type, const socket_handler_t &handler)
{
    Stop();

    auto handler_wrapper = [fd, handler, this]()
    {
        std::pair<builder_id_t, ICommandParams * > p = handler(fd);
        std::shared_ptr<ICommand> concrete_command(m_commands_factory->Create(p.first, p.second));
        std::shared_ptr<CommandTask> command_task(new CommandTask(concrete_command));
        m_th_pool.Add(command_task, ThreadPool::Priority::MEDIUM);
    };
    
    m_fd_monitor.Add(fd, handler_wrapper, type);
}

void Framework::RemoveSocketHandler(int fd, SocketEventType type)
{
    Stop();
    
    m_fd_monitor.Remove(fd, type);
}