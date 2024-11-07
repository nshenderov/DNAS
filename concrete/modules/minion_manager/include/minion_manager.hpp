/*******************************************************************************
*
* FILENAME : minion_manager.hpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 11.11.2023
* 
*******************************************************************************/

#ifndef NSRD_MINION_MANAGER_HPP
#define NSRD_MINION_MANAGER_HPP

#include <unordered_map> //std::unordered_map
#include <memory> // std::shared_ptr

#include "handleton.hpp" // nsrd::Handleton

namespace nsrd
{
class MinionManager
{
public:
    enum CommandType {READ_CMD = 0, WRITE_CMD};
    
    struct CommandParams
    {
        std::size_t length;
        std::size_t offset;
        void *buffer;
    };

    class IMinionProxy
    {
    public:
        virtual ~IMinionProxy() =0;
        virtual bool Read(CommandParams params) =0;
        virtual bool Write(CommandParams params) =0;
    };
    
    void AddMinion(std::size_t minion_id, std::shared_ptr<IMinionProxy> proxy);
    bool PerformCommand(std::size_t minion_id, CommandParams params, CommandType cmd_type);

    MinionManager(const MinionManager &) =delete;
    MinionManager(const MinionManager &&) =delete;
    MinionManager &operator=(const MinionManager &) =delete;
    MinionManager &operator=(const MinionManager &&) =delete;

private:
    friend class Handleton<MinionManager>;

    explicit MinionManager();
    ~MinionManager();

    std::unordered_map<std::size_t, std::shared_ptr<IMinionProxy> > m_minions;
};
} // namespace nsrd

#endif // NSRD_MINION_MANAGER_HPP