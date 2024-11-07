/*******************************************************************************
*
* FILENAME : logger_test.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 21.09.2023
* 
*******************************************************************************/

#include <dlfcn.h> // dlopen, dlsym, dlclose, dlerror

#include "testing.hpp" // nsrd::testing::Testscmp, nsrd::testing::UnitTest

#define NOT_HANDLETON

#include "logger.hpp" // nsrd::Logger

using nsrd::Handleton;
using nsrd::Logger;
using namespace nsrd::testing;

namespace
{
const std::string path = "./logger.log";
const char test_plugin[] = "./test/liblogger_testing_plugin.so";

void TestLogger();
void TestInfo();
void TestWarn();
void TestError();
void TestDebug();
} // namespace

int main()
{  
    Testscmp cmp;
    cmp.AddTest(UnitTest("Logger", TestLogger));
    cmp.AddTest(UnitTest("Info", TestInfo));
    cmp.AddTest(UnitTest("Warn", TestWarn));
    cmp.AddTest(UnitTest("Error", TestError));
    cmp.AddTest(UnitTest("Debug", TestDebug));
    cmp.Run();
    
    return (0);
}

namespace
{
using func = Logger *(*)();

void TestLogger()
{
    Logger::SetPath(path);

    Logger *log = Handleton<Logger>::GetInstance();

    log->Info("Logger has started");

    sleep(1);
}

void TestInfo()
{
    Logger *log = Handleton<Logger>::GetInstance();

    void *handle = dlopen(test_plugin, RTLD_LAZY);

    TH_ASSERT(handle);

    dlerror();

    char *error;

    func f = (func) dlsym(handle, "LogInfo");

    TH_ASSERT(NULL == dlerror());

    TH_ASSERT(log == f());

    dlclose(handle);

    sleep(1);

    (void) error;
}

void TestWarn()
{
    Logger *log = Handleton<Logger>::GetInstance();

    void *handle = dlopen(test_plugin, RTLD_LAZY);

    TH_ASSERT(handle);

    dlerror();

    char *error;

    func f = (func) dlsym(handle, "LogWarn");

    TH_ASSERT(NULL == dlerror());

    TH_ASSERT(log == f());

    dlclose(handle);

    sleep(1);

    (void) error;
}

void TestError()
{
    Logger *log = Handleton<Logger>::GetInstance();

    void *handle = dlopen(test_plugin, RTLD_LAZY);

    TH_ASSERT(handle);

    dlerror();

    char *error;

    func f = (func) dlsym(handle, "LogError");

    TH_ASSERT(NULL == dlerror());

    TH_ASSERT(log == f());

    dlclose(handle);

    sleep(1);

    (void) error;
}

void TestDebug()
{
    Logger *log = Handleton<Logger>::GetInstance();

    void *handle = dlopen(test_plugin, RTLD_LAZY);

    TH_ASSERT(handle);

    dlerror();

    char *error;

    func f = (func) dlsym(handle, "LogDebug");

    TH_ASSERT(NULL == dlerror());

    TH_ASSERT(log == f());

    dlclose(handle);

    sleep(1);

    (void) error;
}

} // namespace