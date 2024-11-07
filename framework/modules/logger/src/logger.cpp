/*******************************************************************************
*
* FILENAME : logger.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 21.09.2023
* 
*******************************************************************************/

#include <fstream> // std::ofstream, std::time, std::localtime

#include "logger.hpp"  // nsrd::Logger

template class nsrd::Handleton<nsrd::Logger>;

using namespace nsrd;

/* Logger */
/* ************************************************************************** */

std::string Logger::s_pathname("./framework.log");

void Logger::SetPath(const std::string &new_path) noexcept
{
    s_pathname = new_path;
}

Logger::Logger()
    : m_pool(1UL)
{}

void Logger::Info(const std::string &message)
{
    std::shared_ptr<Task> task(new LogTask("info", message));
    m_pool.Add(task, ThreadPool::HIGH);
}

void Logger::Warn(const std::string &message)
{
    std::shared_ptr<Task> task(new LogTask("warn", message));
    m_pool.Add(task, ThreadPool::HIGH);
}

void Logger::Error(const std::string &message)
{
    std::shared_ptr<Task> task(new LogTask("error", message));
    m_pool.Add(task, ThreadPool::HIGH);
}

void Logger::Debug(const std::string &message)
{
    std::shared_ptr<Task> task(new LogTask("debug", message));
    m_pool.Add(task, ThreadPool::HIGH);
}

/* LogTask */
/* ************************************************************************** */

Logger::LogTask::LogTask(const std::string &type, const std::string &message)
    : m_curr_time(std::time(nullptr)), m_type(type), m_message(message)
{}

Logger::LogTask::~LogTask()
{}

void Logger::LogTask::operator()()
{
    std::ofstream log_file(s_pathname.c_str(), std::ios_base::app);
    log_file << GetTimestamp() << "\t[" << m_type << "]\t" << m_message << "\n";
}

std::_Put_time<char> Logger::LogTask::GetTimestamp()
{
    std::tm *local_time = std::localtime(&m_curr_time);
    return (std::put_time(local_time, "%Y%m%d:%H:%M:%S"));
}