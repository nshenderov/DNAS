/*******************************************************************************
*
* FILENAME : reactor_test.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 01.10.2023
* 
*******************************************************************************/
#include <iostream> // std::cout, std::endl
#include <thread> // std::thread
#include <cstring> // std::strlen
#include <unistd.h> // pipe

#include "testing.hpp" // nsrd::testing::Testscmp, nsrd::testing::UnitTest

#include "socket_event_type.hpp" // nsrd::SocketEventType

#include "reactor.hpp" // nsrd::Reactor
#include "listener.hpp" // nsrd::Listener


using namespace nsrd;
using namespace nsrd::testing;

namespace
{
void TestGeneral();
void TestException();
void TestMultipleRunCalls();
void TestRemoveCallback();
} // namespace

int main()
{
    Testscmp cmp;
    cmp.AddTest(UnitTest("General", TestGeneral));
    cmp.AddTest(UnitTest("Exception", TestException));
    cmp.AddTest(UnitTest("Multiple calls to run()", TestMultipleRunCalls));
    cmp.AddTest(UnitTest("Remove callback", TestRemoveCallback));
    cmp.Run();

    return (0);

    (void) TestGeneral;
}


namespace
{
void HandlerRead(int fd)
{
    std::cout << "Received:" << std::endl;

    char buf[100];

    TH_ASSERT(-1 != read(fd, buf, 100));

    std::cout << buf << std::endl;
}

void HandlerClose(Reactor *reactor)
{
    reactor->Stop();
}

void ReactorDriver(Reactor *reactor)
{
    reactor->Run();
}

void TestGeneral()
{
    Reactor reactor;

    int fd_writeread[2];
    TH_ASSERT(-1 != pipe(fd_writeread));
    int fd_close[2];
    TH_ASSERT(-1 != pipe(fd_close));

    reactor.Add(fd_writeread[0], std::bind(HandlerRead, fd_writeread[0]), SocketEventType::READ);
    reactor.Add(fd_close[0], std::bind(HandlerClose, &reactor) , SocketEventType::READ);
    
    std::thread t(ReactorDriver, &reactor);

    TH_ASSERT(-1 != write(fd_writeread[1], "Message 1", 10));

    sleep(1);

    TH_ASSERT(-1 != write(fd_writeread[1], "Message 2", 10));

    sleep(1);

    std::cout << "Going to close.." << std::endl;
    TH_ASSERT(-1 != write(fd_close[1], "", 1));

    t.join();
}

void TestException()
{
    Reactor reactor;
    
    reactor.Add(9, std::bind(HandlerRead, 0), SocketEventType::READ);

    std::thread t([&]() {
        try
        {
            ReactorDriver(&reactor);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Caught exception: " << e.what() << std::endl;
        }
    });

    t.join();    
}

void HandlerCallRun(Reactor *reactor)
{
    reactor->Run();
}

void TestMultipleRunCalls()
{
    Reactor reactor;

    int fd_writeread[2];
    TH_ASSERT(-1 != pipe(fd_writeread));
    int fd_run[2];
    TH_ASSERT(-1 != pipe(fd_run));
    int fd_close[2];
    TH_ASSERT(-1 != pipe(fd_close));

    reactor.Add(fd_writeread[0], std::bind(HandlerRead, fd_writeread[0]), SocketEventType::READ);
    reactor.Add(fd_run[0], std::bind(HandlerCallRun, &reactor) , SocketEventType::READ);
    reactor.Add(fd_close[0], std::bind(HandlerClose, &reactor) , SocketEventType::READ);
    
    std::thread t(ReactorDriver, &reactor);

    TH_ASSERT(-1 != write(fd_writeread[1], "Message 1", 10));

    TH_ASSERT(-1 != write(fd_run[1], "", 1));
    
    sleep(1);

    TH_ASSERT(-1 != write(fd_run[1], "", 1));

    TH_ASSERT(-1 != write(fd_writeread[1], "Message 2", 10));

    sleep(1);

    TH_ASSERT(-1 != write(fd_run[1], "", 1));

    std::cout << "Going to close.." << std::endl;
    TH_ASSERT(-1 != write(fd_close[1], "", 1));

    t.join();
}

void HandlerRemove(int fd, SocketEventType type, Reactor *reactor)
{
    reactor->Remove(fd, type);
}

void TestRemoveCallback()
{
    Reactor reactor;

    int fd_writeread[2];
    TH_ASSERT(-1 != pipe(fd_writeread));

    int fd_remove_HandlerRead[2];
    TH_ASSERT(-1 != pipe(fd_remove_HandlerRead));

    int fd_remove_itself[2];
    TH_ASSERT(-1 != pipe(fd_remove_itself));

    int fd_close[2];
    TH_ASSERT(-1 != pipe(fd_close));

    reactor.Add(fd_writeread[0], std::bind(HandlerRead, fd_writeread[0]), SocketEventType::READ);
    
    reactor.Add(fd_remove_HandlerRead[0], 
    std::bind(HandlerRemove, fd_writeread[0], SocketEventType::READ, &reactor) , SocketEventType::READ);

    reactor.Add(fd_remove_itself[0], 
    std::bind(HandlerRemove, fd_remove_itself[0], SocketEventType::READ, &reactor) , SocketEventType::READ);

    reactor.Add(fd_close[0], std::bind(HandlerClose, &reactor) , SocketEventType::READ);
    
    std::thread t(ReactorDriver, &reactor);

    TH_ASSERT(-1 != write(fd_writeread[1], "Message 1", 10));
    
    sleep(1);

    TH_ASSERT(-1 != write(fd_remove_HandlerRead[1], "", 1));

    TH_ASSERT(-1 != write(fd_writeread[1], "Message 2", 10));

    TH_ASSERT(-1 != write(fd_remove_itself[1], "", 1));

    sleep(1);

    TH_ASSERT(-1 != write(fd_remove_itself[1], "", 1));

    std::cout << "Going to close.." << std::endl;
    TH_ASSERT(-1 != write(fd_close[1], "", 1));

    t.join();
}
} // namespace