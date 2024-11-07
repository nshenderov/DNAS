/*******************************************************************************
*
* FILENAME : logger.hpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 21.09.2023
* 
*******************************************************************************/

#ifndef NSRD_LOGGER_HPP
#define NSRD_LOGGER_HPP

#include <string> // std::string
#include <iomanip> // std::put_time

#include "handleton.hpp" // nsrd::Handleton
#include "thread_pool.hpp" // nsrd::ThreadPool

namespace nsrd
{
class Logger
{
public:
    static void SetPath(const std::string &new_path) noexcept;

    void Info(const std::string &message);  // can throw std::system_error, std::ios::failure, std::bad_alloc exception
    void Warn(const std::string &message);  // can throw std::system_error, std::ios::failure, std::bad_alloc exception
    void Error(const std::string &message); // can throw std::system_error, std::ios::failure, std::bad_alloc exception
    void Debug(const std::string &message); // can throw std::system_error, std::ios::failure, std::bad_alloc exception
    
    Logger(const Logger &) = delete;
    Logger(Logger&&) = delete;
    Logger &operator=(const Logger &) = delete;
    Logger &operator=(Logger&&) = delete;

private:
    explicit Logger();
    friend class Handleton<Logger>;
    static std::string s_pathname;
    ThreadPool m_pool;

    class LogTask : public Task
    {
    public:
        LogTask(const std::string &type, const std::string &message);
        ~LogTask();
        void operator()();
    private:
        std::_Put_time<char> GetTimestamp();

        std::time_t m_curr_time;
        std::string m_type;
        std::string m_message;
    };
};
} // namespace nsrd

#endif // NSRD_LOGGER_HPP
