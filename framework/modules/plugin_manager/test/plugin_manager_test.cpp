/*******************************************************************************
*
* FILENAME : plugin_manager_test.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 28.09.2023
* 
*******************************************************************************/
#include <unistd.h> // sleep

#include "testing.hpp" // nsrd::testing::Testscmp, nsrd::testing::UnitTest

#include "dir_monitor.hpp" // nsrd::DirMonitor
#include "dll_loader.hpp" // nsrd::DllLoader

using namespace nsrd;
using namespace nsrd::testing;

extern "C" int Foo1();
extern "C" int Foo2();
extern "C" int Foo3();

namespace
{
void TestLoadUnload();
void TestUpdate();
} // namespace

int main()
{
    Testscmp cmp;
    cmp.AddTest(UnitTest("Load/Unload", TestLoadUnload));
    cmp.AddTest(UnitTest("Update", TestUpdate));
    cmp.Run();

    return (0);
}
namespace
{
const std::string local_path("./test/");
const std::string plugins_path("./test/plugins/");

const std::string plugin1("libtest_plugin1.so");
const std::string plugin2("libtest_plugin2.so");
const std::string plugin3("libtest_plugin3.so");
const std::string dummy("test_dummy.md");

const std::string local_plugin1(local_path + plugin1);
const std::string local_plugin2(local_path + plugin2);
const std::string local_plugin3(local_path + plugin3);
const std::string local_dummy(local_path + dummy);

const std::string plugins_plugin1(plugins_path + plugin1);
const std::string plugins_plugin2(plugins_path + plugin2);
const std::string plugins_plugin3(plugins_path + plugin3);
const std::string plugins_dummy(plugins_path + dummy);

void TestLoadUnload()
{
    DirMonitor dir(plugins_path);
    DllLoader loader;

    dir.AttachSub(loader.GetCallback());
    dir.Start();

    TH_ASSERT(-1 != rename(local_plugin1.c_str(), plugins_plugin1.c_str()));
    TH_ASSERT(-1 != rename(local_plugin2.c_str(), plugins_plugin2.c_str()));
    TH_ASSERT(-1 != rename(local_plugin3.c_str(), plugins_plugin3.c_str()));
    TH_ASSERT(-1 != rename(local_dummy.c_str(), plugins_dummy.c_str()));

    sleep(1);
    
    TH_ASSERT(1 == Foo1());
    TH_ASSERT(2 == Foo2());
    TH_ASSERT(3 == Foo3());

    sleep(1);

    TH_ASSERT(-1 != rename(plugins_dummy.c_str(), local_dummy.c_str()));
    TH_ASSERT(-1 != rename(plugins_plugin1.c_str(), local_plugin1.c_str()));
    TH_ASSERT(-1 != rename(plugins_plugin2.c_str(), local_plugin2.c_str()));
    TH_ASSERT(-1 != rename(plugins_plugin3.c_str(), local_plugin3.c_str()));

    sleep(1);
}

void TestUpdate()
{
    DirMonitor dir(plugins_path);
    DllLoader loader;

    dir.AttachSub(loader.GetCallback());
    dir.Start();

    TH_ASSERT(-1 != rename(local_plugin1.c_str(), plugins_plugin1.c_str()));

    sleep(3);

    std::string command = "touch " + std::string(plugins_plugin1);
    TH_ASSERT(0 == std::system(command.c_str()));

    sleep(1);

    TH_ASSERT(-1 != rename(plugins_plugin1.c_str(), local_plugin1.c_str()));

    TH_ASSERT(-1 != rename(local_dummy.c_str(), plugins_dummy.c_str()));

    sleep(3);

    command = "touch " + std::string(plugins_dummy);
    TH_ASSERT(0 == std::system(command.c_str()));

    sleep(1);

    TH_ASSERT(-1 != rename(plugins_dummy.c_str(), local_dummy.c_str()));
}
} // namespace