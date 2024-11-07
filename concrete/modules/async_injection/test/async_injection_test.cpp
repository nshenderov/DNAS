/*******************************************************************************
*
* FILENAME : async_injection_test.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 05.10.2023
* 
*******************************************************************************/

#include "testing.hpp" // testing::Testscmp, testing::UnitTest

#include "handleton.hpp" // nsrd::Handleton
#include "async_injection.hpp" // nsrd::AsyncInjection

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
class TestAction
{
public:
    TestAction(int &data, int val, int max): m_data(data), m_val(val), m_max(max){};
    ~TestAction(){};

    bool operator()()
    {
        m_data += m_val;
        return (m_data >= m_max);
    };

private:
    int &m_data;
    int m_val;
    int m_max;
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
    int data = 0;

    AsyncInjection::Inject(TestAction(data, 5, 15), AsyncInjection::interval_t(1000));

    SleepFor(5);

    TH_ASSERT(15 == data);
}
} // namespace