/*******************************************************************************
*
* FILENAME : framework_test.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 26.10.2023
* 
*******************************************************************************/

#include <string> // std::string

#include "testing.hpp" // testing::Testscmp, testing::UnitTest

#include "framework.hpp" // nsrd::Framework

using namespace nsrd;
using namespace testing;

namespace
{
void TestFramework();

std::string test_plugin_path("./tests/libtest_plugin_dbg.so");
std::string copy_test_plugin_path("./tests/plugins/libtest_plugin_dbg.so");
} // namespace

int main()
{
    Testscmp cmp;
    cmp.AddTest(UnitTest("General", TestFramework));
    cmp.Run();

    return (0);

    (void) TestFramework;
}

namespace
{
void MoveFile(const std::string &from, const std::string &to)
{
    TH_ASSERT(-1 != rename(from.c_str(), to.c_str()));
}

void ClosingDriver(Framework *fr, int write_fd)
{
    std::cout << "TYPE 'stop' TO STOP THE PROGRAM" << std::endl;

    std::string s;
    
    while ("stop" != s)
    {
        std::cin >> s;

        if (-1 == write(write_fd, s.c_str(), s.length() + 1))
        {
            std::cerr << "Error writing to fd" << std::endl;
        }
    }

    fr->Stop();
}

enum PIPE_PAIR {READ_END = 0, WRITE_END, PIPE_PAIR};

std::pair<builder_id_t, ICommandParams * > HandleSocketRead(int fd)
{
    std::cout << "[HANDLE_SOCKET_READ] Processing socket" << std::endl;

    char buf[1024];

    if (-1 == read(fd, buf, 1024))
    {
        std::cerr << "[HANDLE_SOCKET_READ] Error processing socket" << std::endl;
        throw std::runtime_error("[HANDLE_SOCKET_READ] Error processing socket");
    }

    TestParams *p = new TestParams();
    p->m_message = buf;

    return (std::pair<builder_id_t, ICommandParams*>(0, p));
}

void TestFramework()
{
    int pipe_to_plugin[PIPE_PAIR];

    TH_ASSERT(-1 != pipe(pipe_to_plugin));
    
    Framework fr("./tests/plugins/");
    
    fr.AddSocketHandler(pipe_to_plugin[READ_END], SocketEventType::READ, HandleSocketRead);

    MoveFile(test_plugin_path, copy_test_plugin_path);

    std::cout << "Test plugin has been moved to 'plugins'" << std::endl;
    std::cout << "Every type in terminal will be readed by the plugin" << std::endl;
    std::cout << "Type something!" << std::endl;

    sleep(1);

    std::thread stop_thread = std::thread(&ClosingDriver, &fr, pipe_to_plugin[WRITE_END]);

    sleep(1);

    fr.Run();

    MoveFile(copy_test_plugin_path, test_plugin_path);

    sleep(1);
    
    stop_thread.join();
}
} // namespace