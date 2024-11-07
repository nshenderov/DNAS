/*******************************************************************************
*
* FILENAME : logger_testing_plugin.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 21.09.2023
* 
*******************************************************************************/

#include "logger.hpp"

using nsrd::Handleton;
using nsrd::Logger;

extern "C" Logger *LogInfo()
{
    Logger *log = Handleton<Logger>::GetInstance();
    
    for (size_t i = 0; i < 5; ++i)
    {
        log->Info("Info message");
    }

    return (log);
}

extern "C" Logger *LogWarn()
{
    Logger *log = Handleton<Logger>::GetInstance();
    
    for (size_t i = 0; i < 5; ++i)
    {
        log->Warn("Warn message");
    }

    return (log);
}

extern "C" Logger *LogError()
{
    Logger *log = Handleton<Logger>::GetInstance();
    
    for (size_t i = 0; i < 5; ++i)
    {
        log->Error("Error message");
    }

    return (log);
}

extern "C" Logger *LogDebug()
{
    Logger *log = Handleton<Logger>::GetInstance();
    
    for (size_t i = 0; i < 5; ++i)
    {
        log->Debug("Debug message");
    }

    return (log);
}
