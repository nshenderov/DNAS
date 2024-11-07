/*******************************************************************************
*
* FILENAME : nbd_communicator_test.hpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 19.10.2023
* 
*******************************************************************************/

#ifndef NSRD_NBD_COMMUNICATOR_HPP
#define NSRD_NBD_COMMUNICATOR_HPP

#include <string> // std::string
#include <thread> // std::thread
#include <mutex> // std::mutex

#include <unordered_map> // std::unordered_map
#include <functional> // std::function, std::bind

namespace nsrd
{
class NBDCommunicator
{
public:
    enum EventType: int {READ, WRITE};

    struct Event
    {
        EventType m_type;
        std::size_t m_offset;
        unsigned m_length;
        char m_event_id[8];
    }; 

    enum StatusType 
    {
        SUCCESS, 
        INVALID_REQUEST, // error in request (for example, requested offset is out of bounds)
        OUT_OF_SPACE, // not enough free space on the device
        IO_ERROR // read or write error because of the device fail
    };

    class NBDCommOpenChannelsExc : public std::exception
    {
    public:
        const char *what() const noexcept;
    };

    explicit NBDCommunicator(const std::string &dev_file_path, std::size_t nbd_size);
    ~NBDCommunicator() noexcept;

    NBDCommunicator(NBDCommunicator &) = delete;
    NBDCommunicator(NBDCommunicator &&) = delete;
    NBDCommunicator &operator=(NBDCommunicator &) = delete; 
    NBDCommunicator &operator=(NBDCommunicator &&) = delete;
    
    /*
        User can read objects of type Event from the fd
        If EventType is WRITE, user needs to do a second read of m_length bytes from the fd
    */
    int GetNBDFileDescriptor() noexcept;
    
    /*
        Specify length and buf when replying to a READ event with the status SUCCESS
        In other cases set length to 0 and buf to nullptr
    */
    void Reply(StatusType status,  const char event_id[8], std::size_t length = 0UL, const void *buf = nullptr); 

private:
    enum NBD_COMM_STATUS {NBD_COMM_FAILURE = -1, NBD_COMM_SUCCESS};
    enum NBD_COMM_SOCKETS {SERVER_SOCK, TRANSLATOR_SOCK, NBD_COMM_SOCKETS_AMOUNT};
    enum COMM_SOCKETS {FROM_COMMUNICATOR_SOCK, TO_USER_SOCK, COMM_SOCKETS_AMOUNT};
    enum NBD_RES_STATUS_CODES
    {
        NBD_SUCCESS = 0, 
        NBD_EIO = 5,
        NBD_EINVAL = 22,
        NBD_ENOSPC = 28,
    };

    typedef struct nbd_request nbd_request_t;
    typedef struct nbd_reply nbd_reply_t;

    std::string m_dev_file_path;
    std::size_t m_nbd_size;
    int m_nbd_fd;
    int m_nbd_sockets[NBD_COMM_SOCKETS_AMOUNT];
    int m_comm_sockets[COMM_SOCKETS_AMOUNT];
    std::mutex m_reply_mutex;

    std::thread m_nbd_server_thread;
    std::thread m_nbd_translator_thread;

    NBD_COMM_STATUS  SetupChannels();
    void CloseChannels() noexcept;
    void ServerDriver();
    void TranslatorDriver();

    NBD_COMM_STATUS TranslateReqType(uint32_t nbd_type, EventType &event_type);
    NBD_COMM_STATUS TranslateResType(StatusType type, uint32_t &nbd_type);

    NBD_COMM_STATUS HandleRead(const nbd_request_t &request);
    NBD_COMM_STATUS HandleWrite(const nbd_request_t &request);
    void HandleDisc() noexcept;

    void StopCommunicator() noexcept;
};
}

#endif // NSRD_NBD_COMMUNICATOR_HPP