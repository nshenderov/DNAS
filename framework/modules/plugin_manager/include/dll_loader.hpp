/*******************************************************************************
*
* FILENAME : dll_loader.hpp
* 
* AUTHOR : Nikita Shenderov
*
* DATE : 28.09.2023
* 
*******************************************************************************/

#ifndef NSRD_DLL_LOADER_HPP
#define NSRD_DLL_LOADER_HPP

#include <string> // std::string
#include <unordered_map> // std::unordered_map

#include "dir_monitor.hpp" // DirEventType, DirEvent
#include "dispatcher.hpp" // nsrd::Dispatcher

namespace nsrd
{
class DllLoader;

class LoaderCallback : public Callback<DirMonitor::DirEvent>
{
public:
    explicit LoaderCallback(const DllLoader &owner);
    ~LoaderCallback() noexcept;
    
    LoaderCallback(LoaderCallback &other) = delete;
    LoaderCallback(LoaderCallback &&other) = delete;
    LoaderCallback &operator=(LoaderCallback &other) = delete;
    LoaderCallback &operator=(LoaderCallback &&other) = delete;
    
private:
    using handler_func = std::function<void(const std::string &)>;

    void Notify(DirMonitor::DirEvent event_list);
    void NotifyOnDeath();
    void SetUpHandlers();

    const DllLoader &m_owner;
    std::unordered_map<DirMonitor::DirEventType, handler_func> m_handlers;
};

class DllLoader
{
public:
    DllLoader();
    ~DllLoader() noexcept;

    DllLoader(DllLoader &other) = delete;
    DllLoader(DllLoader &&other) = delete;
    DllLoader &operator=(DllLoader &other) = delete;
    DllLoader &operator=(DllLoader &&other) = delete;

    void LoadDLL(const std::string &dll_path) const; // may throw std::runtime_error
    void UpdateDLL(const std::string &dll_path) const; // may throw std::runtime_error
    void UnloadDLL(const std::string &dll_path) const; // may throw std::runtime_error
    
    LoaderCallback *GetCallback();

private:
    using dll_handle = void *;
    using loaded_dlls = std::unordered_map<std::string, dll_handle>;

    LoaderCallback m_callback;
    mutable loaded_dlls m_plugins;
};
} // namespace nsrd

#endif // NSRD_DLL_LOADER_HPP