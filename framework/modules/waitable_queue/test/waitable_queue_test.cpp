/*******************************************************************************
*
* FILENAME : waitable_queue_test.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 07.09.2023
* 
*******************************************************************************/

#include <queue> // std::queue
#include <thread> // std::thread
#include <chrono> // std::chrono

#include "testing.hpp" // nsrd::testing::Testscmp, nsrd::testing::UnitTest
#include "waitable_queue.hpp" // nsrd::WaitableQueue

using namespace nsrd;
using namespace nsrd::testing;

namespace tests
{
void TestGeneral(void);
} // namespace tests

int main()
{
    Testscmp cmp;

    cmp.AddTest(UnitTest("General", tests::TestGeneral));
    cmp.Run();

    return (0);
}
namespace tests
{
template <typename T>
void Produce(WaitableQueue<T> &wq, int start)
{
    int val = start;

    for (size_t i = 0; i < 10; ++i)
    {
        wq.Push(val);

        std::cout << "Produced: " << val << std::endl;
        ++val;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "Producer stoped" << val << std::endl;
}

template <typename T>
void Consume(WaitableQueue<T> &wq)
{
    int val = -1;
    while (wq.Pop(val, std::chrono::seconds(7)))
    {
        std::cout << "Consumed: " << val << std::endl;
    }

    std::cout << "Consumer stoped" << std::endl;
}

void TestGeneral(void)
{
    WaitableQueue<int> wq;

    std::thread Producer1(Produce<int>, std::ref(wq), 1000);
    std::thread Producer2(Produce<int>, std::ref(wq), 0);

    std::thread Consumer1(Consume<int>, std::ref(wq));
    std::thread Consumer2(Consume<int>, std::ref(wq));
    std::thread Consumer3(Consume<int>, std::ref(wq));

    Producer1.join();
    Producer2.join();
    Consumer1.join();
    Consumer2.join();
    Consumer3.join();
}

} // namespace tests
