/*******************************************************************************
*
* FILENAME : nbd_comm_proxy.hpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 01.11.2023
* 
*******************************************************************************/

#ifndef NSRD_NBDCOMMPROXY_HPP
#define NSRD_NBDCOMMPROXY_HPP

#include "handleton.hpp" // nsrd::Handleton
#include "nbd_communicator.hpp" // nsrd::NBDCommunicator

namespace nsrd
{
class NBDCommProxy
{
public:
    static void SetNBDCommProxy(const std::string &dev_file_path, std::size_t nbd_size) noexcept;

    NBDCommProxy(const NBDCommProxy &) = delete;
    NBDCommProxy(NBDCommProxy&&) = delete;
    NBDCommProxy &operator=(const NBDCommProxy &) = delete;
    NBDCommProxy &operator=(NBDCommProxy&&) = delete;

    int GetNBDFileDescriptor() noexcept;
    void Reply(NBDCommunicator::StatusType status,  const char event_id[8], std::size_t length = 0UL, const void *buf = nullptr); 

private:
    explicit NBDCommProxy();
    ~NBDCommProxy();
    friend class Handleton<NBDCommProxy>;

    static NBDCommunicator *s_nbd;
};
} // namespace nsrd

#endif // NSRD_NBDCOMMPROXY_HPP
