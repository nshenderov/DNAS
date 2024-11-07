/*******************************************************************************
*
* FILENAME : nbd_read_command.cpp
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
class NBDReadCommand: public nsrd::ICommand
{
public:
    NBDReadCommand(nsrd::ICommandParams *params);
    ~NBDReadCommand();

    void operator()();

private:
    nsrd::NBDParams *m_params;
};

NBDReadCommand::NBDReadCommand(nsrd::ICommandParams *params)
    : m_params(dynamic_cast<nsrd::NBDParams *>(params))
{}

NBDReadCommand::~NBDReadCommand()
{
    delete m_params->m_data;
    m_params->m_data = nullptr;

    delete m_params;
    m_params = nullptr;
}

nsrd::ICommand *NBDReadCommandBuilder(nsrd::ICommandParams *params)
{
    return (new NBDReadCommand(params));
}

void NBDReadCommand::operator()()
{
    #ifndef NDEBUG
    std::cout << "[NBD_READ_COMMAND] Called command's operator()" << std::endl;
    std::cout << "m_Offset = " << m_params->m_offset << std::endl;
    std::cout << "m_Length = " << m_params->m_len << std::endl;
    #endif
    
    m_params->m_data = static_cast<char *>(operator new(m_params->m_len));
    std::memset(m_params->m_data, 0, m_params->m_len);

    nsrd::RaidManager::read_callback_t read_handler = [&](std::size_t minion_idx, nsrd::RaidManager::CommandParams *params)->bool
    {
        nsrd::MinionManager *manager = nsrd::Handleton<nsrd::MinionManager>::GetInstance();
        return (manager->PerformCommand(minion_idx, {params->length, params->offset, params->buffer}, nsrd::MinionManager::CommandType::READ_CMD));
    };

    nsrd::RaidManager *raid_manager = nsrd::Handleton<nsrd::RaidManager>::GetInstance();
    raid_manager->Read({m_params->m_len, m_params->m_offset, m_params->m_data}, &read_handler);

    nsrd::NBDCommProxy *nbd = nsrd::Handleton<nsrd::NBDCommProxy>::GetInstance();
    nbd->Reply(nsrd::NBDCommunicator::StatusType::SUCCESS, m_params->m_event_id, m_params->m_len, m_params->m_data);

    delete m_params->m_data;
    m_params->m_data = nullptr;
}

} // anonimus namespace

void PluginLoad()
{
    std::cout << "[NBD_READ_COMMAND] Plugin load" << std::endl;

    nsrd::RegisterBuilder(NBDReadCommandBuilder, nsrd::NBDCommunicator::EventType::READ);
}

void PluginUnload()
{
    std::cout << "[NBD_READ_COMMAND] Plugin unload" << std::endl;

    nsrd::RemoveBuilder(nsrd::NBDCommunicator::EventType::READ);
}