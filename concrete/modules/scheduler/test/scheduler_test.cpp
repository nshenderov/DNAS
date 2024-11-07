/*******************************************************************************
*
* FILENAME : scheduler_test.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 03.10.2023
* 
*******************************************************************************/

#include "testing.hpp" // testing::Testscmp, testing::UnitTest

#include "handleton.hpp" // nsrd::Handleton
#include "scheduler.hpp" // nsrd::Scheduler

using namespace nsrd;
using namespace nsrd::testing;

namespace
{
void TestGeneral();
} // namespace

int main()
{
    Testscmp cmp;
    cmp.AddTest(UnitTest("General", TestGeneral));
    cmp.Run();

    return (0);
}

namespace
{
class TestTask : public Scheduler::Task
{
public:
    TestTask(int &data, int val): m_data(data), m_val(val){};
    ~TestTask(){};
    void operator()(){ m_data += m_val; };

private:
    int &m_data;
    int m_val;
};

void SleepFor(std::size_t secs)
{
    for (std::size_t i = 0; i < secs; ++i)
    {
        sleep(1);
    }
}

void TestGeneral()
{
    using sclock = std::chrono::system_clock;
    using secs = std::chrono::seconds;

    Scheduler *scheduler = Handleton<Scheduler>::GetInstance();

    int data = 0;

    std::shared_ptr<Scheduler::Task> task(new TestTask(data, 5));

    scheduler->Add(task, sclock::now() + secs(1));
    scheduler->Add(task, sclock::now() + secs(1));
    scheduler->Add(task, sclock::now() + secs(1));

    SleepFor(4);

    TH_ASSERT(15 == data);

    data = 0;

    Scheduler::timepoint_t deadline = sclock::now() + secs(1);

    std::shared_ptr<Scheduler::Task> task1(new TestTask(data, 1));
    std::shared_ptr<Scheduler::Task> task2(new TestTask(data, 2));
    std::shared_ptr<Scheduler::Task> task3(new TestTask(data, 3));

    scheduler->Add(task1, deadline);
    scheduler->Add(task2, deadline);
    scheduler->Add(task3, deadline);

    SleepFor(2);

    TH_ASSERT(6 == data);
}
} // namespace