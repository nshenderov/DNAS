/*******************************************************************************
*
* FILENAME : nbd_write_command.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 02.11.2023
* 
*******************************************************************************/


#define NOT_HANDLETON

#include <iostream>
#include <unistd.h>
#include <cstring> // std::memcpy
#include <memory> // std::shared_ptr

#include "handleton.hpp"
#include "plugin_tools.hpp"

#include "minion_manager.hpp"
#include "raid_manager.hpp"
#include "nbd_params.hpp"
#include "nbd_comm_proxy.hpp"
#include "nbd_communicator.hpp"

void PluginLoad() __attribute__((constructor));
void PluginUnload() __attribute__((destructor));

namespace
{
class NBDWriteCommand: public nsrd::ICommand
{
public:
    NBDWriteCommand(nsrd::ICommandParams *params);
    ~NBDWriteCommand();

    void operator()();

private:
    nsrd::NBDParams *m_params;
};

NBDWriteCommand::NBDWriteCommand(nsrd::ICommandParams *params)
    : m_params(dynamic_cast<nsrd::NBDParams *>(params))
{}

NBDWriteCommand::~NBDWriteCommand()
{
    delete m_params->m_data;
    m_params->m_data = nullptr;

    delete m_params;
    m_params = nullptr;
}

nsrd::ICommand *NBDWriteCommandBuilder(nsrd::ICommandParams *params)
{
    return (new NBDWriteCommand(params));
}


void NBDWriteCommand::operator()()
{
    #ifndef NDEBUG
    std::cout << "[NBD_WRITE_COMMAND] Called command's operator()" << std::endl;
    std::cout << "m_Offset = " << m_params->m_offset << std::endl;
    std::cout << "m_Length = " << m_params->m_len << std::endl;
    #endif

    nsrd::RaidManager::write_callback_t write_handler = [&](std::size_t minion_idx, const nsrd::RaidManager::CommandParams &params)->bool
    {

        nsrd::MinionManager *manager = nsrd::Handleton<nsrd::MinionManager>::GetInstance();
        return (manager->PerformCommand(minion_idx, {params.length, params.offset, params.buffer}, nsrd::MinionManager::CommandType::WRITE_CMD)
               );
    };

    nsrd::RaidManager *raid_manager = nsrd::Handleton<nsrd::RaidManager>::GetInstance();
    raid_manager->Write({m_params->m_len, m_params->m_offset, m_params->m_data}, &write_handler);

    nsrd::NBDCommProxy *nbd = nsrd::Handleton<nsrd::NBDCommProxy>::GetInstance();
    nbd->Reply(nsrd::NBDCommunicator::StatusType::SUCCESS, m_params->m_event_id);

    delete m_params->m_data;
    m_params->m_data = nullptr;
}

} // anonimus namespace

void PluginLoad()
{
    std::cout << "[NBD_WRITE_COMMAND] Plugin load" << std::endl;

    nsrd::RegisterBuilder(NBDWriteCommandBuilder, nsrd::NBDCommunicator::EventType::WRITE);
}

void PluginUnload()
{
    std::cout << "[NBD_WRITE_COMMAND] Plugin unload" << std::endl;

    nsrd::RemoveBuilder(nsrd::NBDCommunicator::EventType::WRITE);
}