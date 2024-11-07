/*******************************************************************************
*
* FILENAME : thread_pool_test.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 14.09.2023
* 
*******************************************************************************/

#include "testing.hpp" // nsrd::testing::Testscmp, nsrd::testing::UnitTest
#include "thread_pool.hpp" // nsrd::ThreadPool

using namespace nsrd;
using namespace nsrd::testing;

namespace
{
void TestAddTask();
void TestPauseResume();
void TestSetNThreads();
void TestStop();
void TestFuture();
} // namespace

int main()
{
    Testscmp cmp;
    cmp.AddTest(UnitTest("AddTask", TestAddTask));
    cmp.AddTest(UnitTest("PauseResume", TestPauseResume));
    cmp.AddTest(UnitTest("SetNThreads", TestSetNThreads));
    cmp.AddTest(UnitTest("Stop", TestStop));
    cmp.AddTest(UnitTest("Future", TestFuture));
    cmp.Run();

    return (0);
}
namespace
{
class MyTask : public Task
{
public:
    MyTask(std::size_t idx, std::size_t arr[]) : m_idx(idx), m_arr(arr){};
    ~MyTask(){};
    void operator()(){ m_arr[m_idx] = m_idx; }
private:
    std::size_t m_idx;
    std::size_t *m_arr;
};

void TestAddTask()
{
    ThreadPool tp(5);

    TH_ASSERT(true == tp.QIsEmpty());
    TH_ASSERT(5 == tp.GetNThreads());

    std::size_t arr[100] = {0};

    for (std::size_t i = 0; i < 100; ++i)
    {
        std::shared_ptr<Task> task(new MyTask(i, arr));
        tp.Add(task, ThreadPool::LOW);
    }

    sleep(1);

    for (std::size_t i = 0; i < 100; ++i)
    {
        TH_ASSERT(i == arr[i]);
    }
}

std::size_t MyFutureFunc()
{
    sleep(3);
    return (1);
}

void TestPauseResume()
{
    ThreadPool tp(5);

    std::size_t arr[100] = {0};

    tp.Pause();

    sleep(1);

    TH_ASSERT(true == tp.QIsEmpty());
    TH_ASSERT(5 == tp.GetNThreads());

    for (std::size_t i = 0; i < 100; ++i)
    {
        std::shared_ptr<Task> task(new MyTask(i, arr));
        tp.Add(task, ThreadPool::LOW);
    }

    TH_ASSERT(false == tp.QIsEmpty());

    sleep(1);

    for (std::size_t i = 0; i < 100; ++i)
    {
        TH_ASSERT(0 == arr[i]);
    }

    tp.Resume();

    sleep(1);

    TH_ASSERT(true == tp.QIsEmpty());

    for (std::size_t i = 0; i < 100; ++i)
    {
        TH_ASSERT(i == arr[i]);
    }
}

void TestSetNThreads()
{
    ThreadPool tp(5);

    tp.SetNumOfThreads(5);
    sleep(1);
    TH_ASSERT(5 == tp.GetNThreads());

    tp.SetNumOfThreads(15);
    sleep(1);
    TH_ASSERT(15 == tp.GetNThreads());

    tp.SetNumOfThreads(2);
    sleep(1);
    TH_ASSERT(2 == tp.GetNThreads());

    tp.Pause();
    std::size_t arr[100] = {0};
    sleep(1);
    TH_ASSERT(true == tp.QIsEmpty());
    for (std::size_t i = 0; i < 100; ++i)
    {
        std::shared_ptr<Task> task(new MyTask(i, arr));
        tp.Add(task, ThreadPool::LOW);
    }
    tp.SetNumOfThreads(22);
    TH_ASSERT(false == tp.QIsEmpty());
    tp.Resume();
    sleep(1);
    TH_ASSERT(true == tp.QIsEmpty());
    for (std::size_t i = 0; i < 100; ++i)
    {
        TH_ASSERT(i == arr[i]);
    }
    TH_ASSERT(22 == tp.GetNThreads());

    tp.Pause();
    tp.SetNumOfThreads(0);
    tp.Resume();
    sleep(1);
    TH_ASSERT(0 == tp.GetNThreads());
}

void TestStop()
{
    ThreadPool tp(5);

    tp.Stop();
    TH_ASSERT(true == tp.QIsEmpty());
    TH_ASSERT(0 == tp.GetNThreads());

    tp.Pause();
    tp.SetNumOfThreads(5);
    std::size_t arr[100] = {0};
    for (std::size_t i = 0; i < 100; ++i)
    {
        std::shared_ptr<Task> task(new MyTask(i, arr));
        tp.Add(task, ThreadPool::LOW);
    }
    TH_ASSERT(false == tp.QIsEmpty());
    TH_ASSERT(5 == tp.GetNThreads());
    tp.Stop();
    TH_ASSERT(true == tp.QIsEmpty());
    TH_ASSERT(0 == tp.GetNThreads());

    tp.SetNumOfThreads(5);
    for (std::size_t i = 0; i < 100; ++i)
    {
        std::shared_ptr<Task> task(new MyTask(i, arr));
        tp.Add(task, ThreadPool::LOW);
    }
    sleep(1);
    for (std::size_t i = 0; i < 100; ++i)
    {
        TH_ASSERT(i == arr[i]);
    }
}

void TestFuture()
{
    std::function<std::size_t()> ft_func = MyFutureFunc;
    FutureTask<std::size_t> *future_task1 = new FutureTask<std::size_t>(ft_func);
    FutureTask<std::size_t> *future_task2 = new FutureTask<std::size_t>(ft_func);
    std::shared_ptr<Task> future1(future_task1);
    std::shared_ptr<Task> future2(future_task2);

    ThreadPool tp(5);
    
    TH_ASSERT(false == future_task1->IsReady());
    tp.Add(future1, ThreadPool::LOW);
    TH_ASSERT(1 == future_task1->Get());

    TH_ASSERT(true == future_task1->IsReady());
    tp.Add(future1, ThreadPool::LOW);
    TH_ASSERT(1 == future_task1->Get());

    tp.Add(future2, ThreadPool::LOW);
    TH_ASSERT(false == future_task2->IsReady());
    future_task2->Wait();
    TH_ASSERT(true == future_task2->IsReady());
    TH_ASSERT(1 == future_task2->Get());
}
} // namespace
