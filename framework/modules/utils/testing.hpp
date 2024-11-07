/*******************************************************************************
*
* FILENAME : testing.hpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 21.08.2023
* 
*******************************************************************************/

#ifndef NSRD_TESTING_HPP
#define NSRD_TESTING_HPP

#include <iostream> //std::cout, std::endl
#include <list> // list

#if __cplusplus <= 199711L
	#define noexcept
#endif

#define TH_ASSERT(res) TH_assert(res, __LINE__)

#define TH_YELLOW "\033[0;33m"
#define TH_GREEN "\033[0;32m"
#define TH_RED "\033[0;31m"
#define TH_NC "\033[0m"
#define TH_UNDERLINE_ON "\033[4m" 
#define TH_UNDERLINE_OFF "\033[0m" 

namespace nsrd
{
namespace testing
{
static int s_asserts_failed = 0;
static int s_asserts_passed = 0;
static bool s_is_curr_passed = false;

class Test
{
public:
    inline explicit Test(const std::string &message, void (*TestFunc)());
    inline virtual void Run() const noexcept;
    inline virtual ~Test() noexcept;
    inline virtual const std::string& GetInfo() const noexcept;

private:
    void (*TestFunction)();
    std::string m_info_message;
};

inline testing::Test::Test(const std::string &message, void (*TestFunc)())
    : TestFunction(TestFunc), m_info_message(message)
{}

inline testing::Test::~Test()
{}

inline void testing::Test::Run() const noexcept
{
    TestFunction();
}

inline const std::string &testing::Test::GetInfo() const noexcept
{
    return (m_info_message);
}

inline void TH_assert(bool res, int line)
{
    if (res)
    {
        ++s_asserts_passed;
    }
    else
    {
        std::cout << TH_RED TH_UNDERLINE_ON "ASSERTION ON THE LINE " 
                  << line << TH_UNDERLINE_OFF TH_NC << std::endl;

        ++s_asserts_failed;
        s_is_curr_passed = false;
    }
}

class Testscmp: public Test
{
public:
    inline explicit Testscmp();
    friend inline void TH_assert(bool, int line);

    inline void Run() const noexcept;
    inline void AddTest(const Test& test);

private:
    std::list<Test> m_tests;
};

inline void testing::Testscmp::AddTest(const testing::Test &test)
{
    m_tests.push_back(test);
}

inline testing::Testscmp::Testscmp()
    : Test("", 0), m_tests(std::list<Test>())
{}

inline void testing::Testscmp::Run() const noexcept
{
    std::size_t tests_passed = 0;
    std::size_t tests_failed = 0;

    std::list<Test>::const_iterator it;
    for (it = m_tests.begin(); it != m_tests.end(); ++it)
    {
        std::cout << "[" << TH_YELLOW "TEST: " TH_NC << (*it).GetInfo() << "]" << std::endl;
    
        s_is_curr_passed = true;

        (*it).Run();

        if (s_is_curr_passed)
        {
            std::cout << TH_GREEN "[SUCCESS]\n" TH_NC << std::endl;
            ++tests_passed;
        }
        else
        {
            std::cout << TH_RED "[FAILURE] " TH_NC << std::endl;
            ++tests_failed;
        }
    }

    std::cout << TH_YELLOW "###" TH_NC << " SUMMARY " << TH_YELLOW "###" TH_NC << std::endl;
    std::cout << "TESTS PASSED: " TH_GREEN << tests_passed << TH_NC << std::endl;
    std::cout << "TESTS FAILED: " TH_RED << tests_failed << TH_NC << std::endl;
    std::cout << "ASSERTS PASSED: " TH_GREEN << s_asserts_passed << TH_NC << std::endl;
    std::cout << "ASSERTS FAILED: " TH_RED << s_asserts_failed << TH_NC << std::endl;
}

class UnitTest: public Test
{
public:
    inline explicit UnitTest(const std::string &message, void (*TestFunc)());
    inline void Run() const noexcept;
    inline ~UnitTest() noexcept;
    
private:
};

inline testing::UnitTest::UnitTest(const std::string &message, void (*TestFunc)())
    : Test(message, TestFunc)
{}

inline testing::UnitTest::~UnitTest()
{}

inline void testing::UnitTest::Run() const noexcept
{
    Test::Run();
}

} // namespace testing;
} // namespace nsrd

#endif // NSRD_TESTING_HPP