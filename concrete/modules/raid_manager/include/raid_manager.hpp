/*******************************************************************************
*
* FILENAME : raid_manager.hpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 8.11.2023
* 
*******************************************************************************/

#ifndef NSRD_RAID_MANAGER_HPP
#define NSRD_RAID_MANAGER_HPP

#include <functional> // std::function
#include <vector> // std::vector

#include "handleton.hpp" // nsrd::Handleton

namespace nsrd
{
class RaidManager
{
public:
    /*
        This struct is designed to encapsulate the details of a command's
        parameters for handling read/write operations.
    */
    struct CommandParams
    {
        std::size_t length;
        std::size_t offset;
        void *buffer;
    };

    /*
        To set non-default number of minions run this function before first call to Handleton<RaidManager>::GetInstance().
    */
    static void SetMinionCount(std::size_t count);
    
    /*
        DESCRIPTION
            These typedef definitions create aliases for function types that accept specific parameters
            and return a bool value status.
            They're intended to serve as callback functions for write and read operations with a minion.
        RETURN
            true: Indicates that the callback succeeded;
            false: Denotes that the callback failed;
        INPUT
            minion_idx: index of minion.
            params: encapsulating information about the IO operation to be executed with the minion. 
    */
    typedef std::function<bool(std::size_t minion_idx, const CommandParams &params)> write_callback_t;
    typedef std::function<bool(std::size_t minion_idx, CommandParams *params)> read_callback_t;
    
    /*
        Calls write_callback amount of minions times in a loop 
        with appropriate CommandParams and returns status.
        If at least 1 write_callback fails, returns failure
    */
    bool Write(CommandParams params, write_callback_t *callback);
    
    /*
        Calls read_callback amount of minion times in a loop
        Returns status if read was successfull or not
        Returns failure if 1 read failed from main storage minion and its mirror.
    */
    bool Read(CommandParams params, read_callback_t *callback);
    
    RaidManager(const RaidManager &) =delete;
    RaidManager(const RaidManager &&) =delete;
    RaidManager &operator=(const RaidManager &) =delete;
    RaidManager &operator=(const RaidManager &&) =delete;

private:
    friend class Handleton<RaidManager>;
    static std::size_t MINION_COUNT;

    explicit RaidManager(std::size_t minion_count = MINION_COUNT);
    ~RaidManager();
    
    typedef std::vector<std::vector<CommandParams>> minions_arrangements_t;
    
    std::size_t m_minion_count;
    std::size_t m_minion_size;
    std::size_t m_mirrors_first_idx;


    void GetArrangements(const CommandParams &params, minions_arrangements_t &arrangements);
    CommandParams CombineDataForWriting(char *buf, std::vector<CommandParams> &minion_blocks);
    CommandParams CombineCommandsForReading(char *buf, std::vector<CommandParams> &minion_blocks);
    void UncombineData(char *buf, std::vector<CommandParams> &minion_blocks);
    bool CallReadCallbacks(std::size_t minion_idx, CommandParams *cmd, RaidManager::read_callback_t *callback);
};

}

#endif // NSRD_RAID_MANAGER_HPP