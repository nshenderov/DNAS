/*******************************************************************************
*
* FILENAME : dll_loader.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 28.09.2023
* 
*******************************************************************************/

#include <cassert> // assert
#include <dlfcn.h> // dlopen, dlclose
#include <iostream> // std::cerr, std::endl

#include "dll_loader.hpp" // nsrd::DirMonitor

using namespace nsrd;

DllLoader::DllLoader()
    : m_callback(*this), m_plugins()
{}

DllLoader::~DllLoader()
{
    for (auto pair : m_plugins)
    {
        if (0 != dlclose(pair.second))
        {
            std::cerr << "dlclose failed " << dlerror() << std::endl;
        }
    }
}

void DllLoader::LoadDLL(const std::string &dll_path) const
{
    #ifndef NDEBUG
    std::cout << dll_path.c_str() << std::endl;
    #endif
    
    if (!(m_plugins[dll_path] = dlopen(dll_path.c_str(), RTLD_LAZY)))
    {
        std::cerr << "dlopen failed " << dlerror() << std::endl;
	}
}

// void DllLoader::UpdateDLL(const std::string &dll_path) const
// {
//     UnloadDLL(dll_path);
//     LoadDLL(dll_path);
// }

void DllLoader::UnloadDLL(const std::string &dll_path) const
{
    dll_handle plugin = m_plugins[dll_path];
    if (nullptr != plugin)
    {
        if (0 != dlclose(m_plugins[dll_path]))
        {
            std::cerr << "dlclose failed " << dlerror() << std::endl;
        }

        m_plugins.erase(dll_path);
    }
}

LoaderCallback *DllLoader::GetCallback()
{
    return (&m_callback);
}

/* LoaderCallback */
/* ************************************************************************** */

LoaderCallback::LoaderCallback(const DllLoader &owner)
    : m_owner(owner), m_handlers()
{
    SetUpHandlers();
}

LoaderCallback::~LoaderCallback()
{}

void LoaderCallback::Notify(DirMonitor::DirEvent event_list)
{
    for (auto pair : event_list)
    {
        m_handlers[pair.second](pair.first);
    }
}

void LoaderCallback::NotifyOnDeath()
{}

void LoaderCallback::SetUpHandlers()
{
    using std::placeholders::_1;
    using EVENT = DirMonitor::DirEventType;

    m_handlers[EVENT::ADDED] = std::bind(&DllLoader::LoadDLL, &m_owner, _1);
    // m_handlers[EVENT::UPDATED] = std::bind(&DllLoader::UpdateDLL, &m_owner, _1);
    m_handlers[EVENT::REMOVED] = std::bind(&DllLoader::UnloadDLL, &m_owner, _1);
}