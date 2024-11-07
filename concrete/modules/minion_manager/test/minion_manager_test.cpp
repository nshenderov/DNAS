/*******************************************************************************
*
* FILENAME : minion_manager_test.cpp
*
* AUTHOR : Nick Shenderov
*
* DATE : 11.11.2023
* 
*******************************************************************************/

#include <string> // std::string

#include <cstring> // std::memcpy

#include "testing.hpp" // testing::Testscmp, testing::UnitTest

#include "minion_manager.hpp" // nsrd::MinionManager

using namespace nsrd;
using namespace nsrd::testing;

namespace
{
void TestMinionManager();

class MinionProxy : public nsrd::MinionManager::IMinionProxy
{
public:
    explicit MinionProxy(const std::string &minion_address, std::size_t minion_size);
    ~MinionProxy();

    bool Write(nsrd::MinionManager::CommandParams params);
    bool Read(nsrd::MinionManager::CommandParams params);

private:
    std::string m_address;
    std::size_t m_size;
    std::shared_ptr<char> m_storage;
};
} // namespace

int main()
{
    Testscmp cmp;
    cmp.AddTest(UnitTest("General", TestMinionManager));
    cmp.Run();

    return (0);
}

namespace
{
/* MinionProxy */
/* ************************************************************************** */


MinionProxy::MinionProxy(const std::string &minion_address, std::size_t minion_size)
    : m_address(minion_address),
      m_size(minion_size),
      m_storage(static_cast<char *>(operator new(minion_size)))
{
    std::memset(m_storage.get(), 0, minion_size);
}

MinionProxy::~MinionProxy()
{}

bool MinionProxy::Write(nsrd::MinionManager::CommandParams params)
{
    std::memcpy(m_storage.get() + params.offset, params.buffer, params.length);
    return (true);
}

bool MinionProxy::Read(nsrd::MinionManager::CommandParams params)
{
    std::memcpy(params.buffer, m_storage.get() + params.offset, params.length);
    return (true);
}

void TestMinionManager()
{
    MinionManager *manager = nsrd::Handleton<MinionManager>::GetInstance();

    std::shared_ptr<MinionProxy> min1(new MinionProxy("192.213.123::5000", 20));
    std::shared_ptr<MinionProxy> min2(new MinionProxy("192.213.123::5000", 20));
    std::shared_ptr<MinionProxy> min3(new MinionProxy("192.213.123::5000", 20));
    std::shared_ptr<MinionProxy> min4(new MinionProxy("192.213.123::5000", 20));

    manager->AddMinion(0, min1);
    manager->AddMinion(1, min2);
    manager->AddMinion(2, min3);
    manager->AddMinion(3, min4);

    char buf0[] = "Hello from Minion 0";
    char buf1[] = "Hello from Minion 1";
    char buf2[] = "Hello from Minion 2";
    char buf3[] = "Hello from Minion 3";

    manager->PerformCommand(0, MinionManager::CommandParams{20, 0, buf0}, MinionManager::CommandType::WRITE_CMD);
    manager->PerformCommand(1, MinionManager::CommandParams{20, 0, buf1}, MinionManager::CommandType::WRITE_CMD);
    manager->PerformCommand(2, MinionManager::CommandParams{20, 0, buf2}, MinionManager::CommandType::WRITE_CMD);
    manager->PerformCommand(3, MinionManager::CommandParams{20, 0, buf3}, MinionManager::CommandType::WRITE_CMD);

    char buf[20];
    manager->PerformCommand(0, {20, 0, buf}, MinionManager::CommandType::READ_CMD);
    TH_ASSERT(std::string(buf) == std::string(buf0));
    manager->PerformCommand(1, {20, 0, buf}, MinionManager::CommandType::READ_CMD);
    TH_ASSERT(std::string(buf) == std::string(buf1));
    manager->PerformCommand(2, {20, 0, buf}, MinionManager::CommandType::READ_CMD);
    TH_ASSERT(std::string(buf) == std::string(buf2));
    manager->PerformCommand(3, {20, 0, buf}, MinionManager::CommandType::READ_CMD);
    TH_ASSERT(std::string(buf) == std::string(buf3));
}
} // namespace