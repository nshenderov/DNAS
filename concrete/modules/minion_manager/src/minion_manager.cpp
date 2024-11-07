/*******************************************************************************
*
* FILENAME : minion_manager.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 11.11.2023
* 
*******************************************************************************/

// #include "async_injection.hpp" // nsrd::AsyncInjection

#include "minion_manager.hpp" // nsrd::MinionManager

using namespace nsrd;

template class nsrd::Handleton<nsrd::MinionManager>;

MinionManager::MinionManager()
    : m_minions()
{}

MinionManager::~MinionManager()
{}

void MinionManager::AddMinion(std::size_t minion_id, std::shared_ptr<MinionManager::IMinionProxy> proxy)
{
    m_minions[minion_id] = proxy;
}

bool MinionManager::PerformCommand(std::size_t minion_id, MinionManager::CommandParams params, MinionManager::CommandType cmd_type)
{
    if (m_minions.find(minion_id) == m_minions.end())
    {
        throw std::runtime_error("Minion is not found");
    }

    switch (cmd_type)
    {
        case (WRITE_CMD):
        {
            return (m_minions[minion_id].get()->Write(params));
        }
        case (READ_CMD):
        {
            return (m_minions[minion_id].get()->Read(params));
        }
        default:
        {
            throw std::runtime_error("Received unsupported command");
        }
    }
}

MinionManager::IMinionProxy::~IMinionProxy()
{}