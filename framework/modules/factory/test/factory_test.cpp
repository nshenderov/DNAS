/*******************************************************************************
*
* FILENAME : factory_test.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 15.09.2023
* 
*******************************************************************************/

#include <string> // std::string
#include <memory> // std::shared_ptr

#include "testing.hpp" // nsrd::testing::Testscmp, nsrd::testing::UnitTest
#include "factory.hpp" // nsrd::Factory
#include "task.hpp" // nsrd::Task

using namespace nsrd;
using namespace nsrd::testing;

Task::~Task(){}

namespace
{
void TestFactory();
} // namespace

int main()
{
    Testscmp cmp;
    cmp.AddTest(UnitTest("Factory", TestFactory));
    cmp.Run();

    return (0);
}
namespace
{
struct Params
{
    Params(int &p1, int p2) : m_p1(p1), m_p2(p2) {};
    int &m_p1;
    int m_p2;
};

class TaskWrite : public Task
{
public:
    TaskWrite(int &var, int value) : m_var(var), m_value(value){};
    ~TaskWrite(){};
    void operator()()
    {
        m_var += m_value;
    }
private:
    int &m_var;
    int m_value;
};

class TaskRead : public Task
{
public:
    TaskRead(int &var, int value) : m_var(var), m_value(value){};
    ~TaskRead(){};
    void operator()()
    {
        m_var += m_value;
    }
private:
    int &m_var;
    int m_value;
};

Task *TaskCreatorWrite(Params params)
{
    TaskWrite *task = new TaskWrite(params.m_p1, params.m_p2);
    return (task);
}

Task *TaskCreatorRead(Params params)
{
    TaskRead *task = new TaskRead(params.m_p1, params.m_p2);
    return (task);
}

void TestFactory()
{
    Factory<Task, std::string, Params> f;

    f.AddCreator(TaskCreatorWrite, "write");
    f.AddCreator(TaskCreatorRead, "read");

    int t1 = 0;
    std::shared_ptr<Task> write_task(f.Create("write", Params(t1, 5)));
    (*write_task)();
    TH_ASSERT(5 == t1);

    int t2 = 10;
    std::shared_ptr<Task> read_task(f.Create("read", Params(t2, 55)));
    (*read_task)();
    TH_ASSERT(65 == t2);
}
} // namespace
