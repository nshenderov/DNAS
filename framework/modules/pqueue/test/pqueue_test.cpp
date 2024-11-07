/*******************************************************************************
*
* FILENAME : pqueue_test.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 10.09.2023
* 
*******************************************************************************/

#include <vector> // std::vector
#include <functional> // std::greather

#include "testing.hpp" // nsrd::testing::Testscmp, nsrd::testing::UnitTest
#include "pqueue.hpp"

using namespace nsrd;
using namespace nsrd::testing;

namespace
{
void TestGeneralBehavior(void);
} // namespace

int main()
{
    Testscmp cmp;
    cmp.AddTest(UnitTest("General behavior", TestGeneralBehavior));
    cmp.Run();

    return (0);
}
namespace
{
void TestGeneralBehavior(void)
{
    PriorityQueue<int> pq1;

    TH_ASSERT(true == pq1.empty());

    for (int i = 100; i >= 0; --i)
    {
        pq1.push_back(i);
    }

    TH_ASSERT(false == pq1.empty());

    for (int i = 100; i >= 0; --i)
    {
        TH_ASSERT(i == pq1.front());
        pq1.pop_front();
    }

    TH_ASSERT(true == pq1.empty());

    PriorityQueue<int, std::vector<int>, std::greater<int> > pq2;

    TH_ASSERT(true == pq2.empty());

    for (int i = 100; i >= 0; --i)
    {
        pq2.push_back(i);
    }

    TH_ASSERT(false == pq2.empty());

    for (int i = 0; i <= 100; ++i)
    {
        TH_ASSERT(i == pq2.front());
        pq2.pop_front();
    }

    TH_ASSERT(true == pq2.empty());
}
} // namespace
