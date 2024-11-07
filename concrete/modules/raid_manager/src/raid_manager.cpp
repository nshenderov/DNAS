/*******************************************************************************
*
* FILENAME : raid_manager.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 8.11.2023
* 
*******************************************************************************/

#include <iostream> // std::cerr, std::endl
#include <cstring> // std::memcpy
#include <unistd.h> // sysconf

#include "raid_manager.hpp" // nsrd::RaidManager

using namespace nsrd;

template class nsrd::Handleton<nsrd::RaidManager>;

namespace
{
    #ifdef TEST_FILE
    const std::size_t BLOCK_SIZE = 4;
    #else
    const long PAGE_SIZE = sysconf(_SC_PAGESIZE);
    const std::size_t BLOCK_SIZE = static_cast<std::size_t>(PAGE_SIZE);
    #endif
}

std::size_t nsrd::RaidManager::MINION_COUNT = 6;

RaidManager::RaidManager(std::size_t minion_count)
    : m_minion_count(minion_count),
      m_mirrors_first_idx(m_minion_count / 2)
{}

RaidManager::~RaidManager()
{}

bool RaidManager::Write(CommandParams params, write_callback_t *callback)
{
    minions_arrangements_t arrangements(m_mirrors_first_idx, std::vector<CommandParams>());
    GetArrangements(params, arrangements);
    char *arranged_buf = new char[params.length]();
    bool status = true;

    for (std::size_t i = 0; (i < m_mirrors_first_idx) && (false != status); ++i)
    {
        if (0 != arrangements[i].size())
        {
            CommandParams cmd = CombineDataForWriting(arranged_buf, arrangements[i]);
            status = callback->operator()(i, cmd) && callback->operator()(i + m_mirrors_first_idx, cmd);
        }
    }

    delete[] arranged_buf;

    return (status);
}

bool RaidManager::Read(RaidManager::CommandParams params, RaidManager::read_callback_t *callback)
{
    minions_arrangements_t arrangements(m_mirrors_first_idx, std::vector<CommandParams>());
    GetArrangements(params, arrangements);
    char *arranged_buf = new char[params.length]();
    bool status = true;

    for (std::size_t i = 0; (i < m_mirrors_first_idx) && (false != status); ++i)
    {
        if (0 != arrangements[i].size())
        {
            CommandParams cmd = CombineCommandsForReading(arranged_buf, arrangements[i]);

            if ((status = CallReadCallbacks(i, &cmd, callback)))
            {
                UncombineData(arranged_buf, arrangements[i]);
            }
        }
    }

    delete[] arranged_buf;

    return (status);
}

void RaidManager::SetMinionCount(std::size_t count)
{
    RaidManager::MINION_COUNT = count;
}

void RaidManager::GetArrangements(const CommandParams &params, minions_arrangements_t &arrangements)
{
    std::size_t length = params.length;
    std::size_t offset = params.offset;
    char *data_runner = static_cast<char *>(params.buffer);

    while (0 != length)
    {
        std::size_t block_idx = offset / BLOCK_SIZE;
        std::size_t block_offset = offset % BLOCK_SIZE;
        std::size_t minion_idx = block_idx % m_mirrors_first_idx;
        std::size_t minion_block_idx = block_idx / m_mirrors_first_idx;
        std::size_t minion_block_offset = minion_block_idx * BLOCK_SIZE + block_offset;
        std::size_t length_written = BLOCK_SIZE - block_offset > length ? length : BLOCK_SIZE - block_offset;

        arrangements[minion_idx].push_back(CommandParams{length_written, minion_block_offset, data_runner});

        data_runner += length_written;
        offset += length_written;
        length -= length_written;
    }
}

RaidManager::CommandParams RaidManager::CombineDataForWriting(char *buf, std::vector<CommandParams> &minion_blocks)
{
    CommandParams minion_command = {0, minion_blocks[0].offset, buf};
    
    for(auto block : minion_blocks)
    {
        std::memcpy(buf, block.buffer, block.length);
        minion_command.length += block.length;
        buf += block.length;
    }

    return (minion_command);
}

RaidManager::CommandParams RaidManager::CombineCommandsForReading(char *buf, std::vector<CommandParams> &minion_blocks)
{
    CommandParams minion_command = {0, minion_blocks[0].offset, buf};
    
    for(auto block : minion_blocks)
    {
        minion_command.length += block.length;
    }

    return (minion_command);
}

void RaidManager::UncombineData(char *buf, std::vector<CommandParams> &minion_blocks)
{
    for(auto block : minion_blocks)
    {
        std::memcpy(block.buffer, buf, block.length);
        buf += block.length;
    }
}

bool RaidManager::CallReadCallbacks(std::size_t minion_idx, CommandParams *cmd, RaidManager::read_callback_t *callback)
{
    bool status = callback->operator()(minion_idx, cmd);

    if (!status)
    {
        std::cerr << "Read on the main failed, trying to read from the mirror" << std::endl;
        status = callback->operator()(minion_idx + m_mirrors_first_idx, cmd);
    }

    if (!status)
    {
        std::cerr << "Read from the mirror also failed, return" << std::endl;
    }
    
    return (status);
}