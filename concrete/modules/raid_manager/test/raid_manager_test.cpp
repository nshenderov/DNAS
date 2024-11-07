/*******************************************************************************
*
* FILENAME : raid_manager_test.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 8.11.2023
* 
*******************************************************************************/

#include <iostream> // std::cout, std::endl
#include <memory> // std::shared_ptr
#include <cstring> // std::memcpy

#include "testing.hpp" // testing::Testscmp, testing::UnitTest

#include "raid_manager.hpp" // nsrd::RaidManager

using namespace nsrd;
using namespace nsrd::testing;

namespace
{
class Minion
{
public:
    explicit Minion(std::size_t storage_size);

    bool operator()(const RaidManager::CommandParams &params);
    bool operator()(RaidManager::CommandParams *params);

private:
    std::shared_ptr<char> m_storage;
};
void TestRaidManager();
} // namespace

int main()
{
    Testscmp cmp;
    cmp.AddTest(UnitTest("General", TestRaidManager));
    cmp.Run();

    return (0);
}

namespace
{
void TestRaidManager()
{
    RaidManager *manager = nsrd::Handleton<RaidManager>::GetInstance();

    Minion minions[6] =
    {
        Minion(6000), Minion(6000), Minion(6000),
        Minion(6000), Minion(6000), Minion(6000)
    };

    RaidManager::write_callback_t write_handler = [&](std::size_t minion_idx, const RaidManager::CommandParams &params)->bool
    {
        std::cout << "\n    Write callback:"<< std::endl;
        std::cout << "    Minion idx = " << minion_idx << std::endl;
        return (minions[minion_idx](params));
    };

    RaidManager::read_callback_t read_handler = [&](std::size_t minion_idx, RaidManager::CommandParams *params)->bool
    {
        std::cout << "\n    Read callback:"<< std::endl;
        std::cout << "    Minion idx = " << minion_idx << std::endl;
        return (minions[minion_idx](params));
    };
    
    char arr_to_write[] = "010102020303040405050606070708080909101011111212";

    manager->Write({48, 0, arr_to_write}, &write_handler);
    std::cout << "\n\n" << std::endl;
    manager->Write({46, 2, (arr_to_write + 2)}, &write_handler);
    std::cout << "\n\n" << std::endl;
    manager->Write({44, 2, (arr_to_write + 2)}, &write_handler);

    std::cout << "\n\n" << std::endl;
    char arr_to_read[49] = {0};
    manager->Read({48, 0, arr_to_read}, &read_handler);

    TH_ASSERT(std::string(arr_to_write) == std::string(arr_to_read));
}

Minion::Minion(std::size_t storage_size)
    : m_storage(static_cast<char *>(operator new(storage_size)))
{}

bool Minion::operator()(const RaidManager::CommandParams &params)
{
    std::cout << "    offset = " << params.offset << std::endl;
    std::cout << "    length = " << params.length << std::endl;
    std::cout << "    data = ";
    char *runner = static_cast<char*>(params.buffer);
    for (std::size_t j = 0; j < params.length; ++j)
    {
        std::cout << runner[j];
    }
    std::cout << std::endl;

    std::memcpy(m_storage.get() + params.offset, params.buffer, params.length);
    
    return (true);
}

bool Minion::operator()(RaidManager::CommandParams *params)
{
    std::cout << "    offset = " << params->offset << std::endl;
    std::cout << "    length = " << params->length << std::endl;
    std::cout << "    data = ";
    char *runner = static_cast<char *>(operator new(params->length + 1));
    std::memset(runner, 0, params->length + 1);

    std::memcpy(runner, m_storage.get() + params->offset, params->length);
    std::cout << runner << std::endl;
    delete runner;

    std::memcpy(params->buffer, m_storage.get() + params->offset, params->length);

    return (true);
}
} // namespace