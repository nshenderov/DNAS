/*******************************************************************************
*
* FILENAME : nbd_communicator.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 19.10.2023
* 
*******************************************************************************/

#include <sys/types.h> // sockets
#include <sys/socket.h> // sockets
#include <sys/stat.h> //open
#include <fcntl.h> //open
#include <sys/ioctl.h> // ioctl
#include <linux/nbd.h> // nbd
#include <unistd.h> // pipe
#include <arpa/inet.h> // ntohl, htonl
#include <cassert> // assert
#include <iostream> // std::cerr, std::endl
#include <cstring> // std::memcpy

#include <pthread.h> // pthread_sigmask
#include <signal.h> // sigemptyset, sigaddset

#include "nbd_communicator.hpp" // nsrd::NBDCommunicator


using namespace nsrd;

namespace
{
int WriteData(int fd, const char *buf, std::size_t count);
int ReadData(int fd, char *buf, std::size_t count);
u_int64_t ntohll(u_int64_t a);
#define htonll ntohll
} // anonimus

NBDCommunicator::NBDCommunicator(const std::string &dev_file_path, std::size_t nbd_size)
    : m_dev_file_path(dev_file_path),
      m_nbd_size(nbd_size),
      m_nbd_fd(),
      m_nbd_sockets(),
      m_comm_sockets(),
      m_reply_mutex(),
      m_nbd_server_thread(),
      m_nbd_translator_thread()
{
    if (NBD_COMM_FAILURE == SetupChannels())
    {
        CloseChannels();
        throw NBDCommOpenChannelsExc();
    }
    
    m_nbd_server_thread = std::thread(&NBDCommunicator::ServerDriver, this);
    m_nbd_translator_thread = std::thread(&NBDCommunicator::TranslatorDriver, this);
}

NBDCommunicator::~NBDCommunicator()
{
    StopCommunicator();
}

int NBDCommunicator::GetNBDFileDescriptor() noexcept
{
    return (m_comm_sockets[TO_USER_SOCK]);
}

void NBDCommunicator::Reply(StatusType status, const char *event_id, std::size_t length, const void *buf)
{
    uint32_t resp_status = 0;
    if (NBD_COMM_FAILURE == TranslateResType(status, resp_status))
    {
        StopCommunicator();
        std::cerr << "[Error] Reply: translating status" << std::endl;
    }

    nbd_reply_t reply({htonl(NBD_REPLY_MAGIC), htonl(resp_status), {0}});
    std::memcpy(reply.handle, event_id, sizeof(reply.handle));

    const std::lock_guard<std::mutex> lock(m_reply_mutex);

    if (-1 == write(m_nbd_sockets[TRANSLATOR_SOCK], &reply, sizeof(nbd_reply_t)))
    {
        std::cerr << "[Error] Reply: writing reply" << std::endl;
        StopCommunicator();
    }
    
    if(buf && -1 == WriteData(m_nbd_sockets[TRANSLATOR_SOCK], static_cast<const char*>(buf), length))
    {
        StopCommunicator();
        std::cerr << "[Error] Reply: writing data" << std::endl;
    }
}

void NBDCommunicator::StopCommunicator() noexcept
{
    if(-1 == ioctl(m_nbd_fd, NBD_DISCONNECT))
    {
      std::cerr << "[Error] Failed to request disconect on the nbd device" << std::endl;
    }

    m_nbd_translator_thread.join();
    m_nbd_server_thread.join();
    
    CloseChannels();
}

void NBDCommunicator::CloseChannels() noexcept
{
    close(m_nbd_sockets[SERVER_SOCK]);
    m_nbd_sockets[SERVER_SOCK] = -1;
    
    close(m_nbd_sockets[TRANSLATOR_SOCK]);
    m_nbd_sockets[TRANSLATOR_SOCK] = -1;
    
    close(m_comm_sockets[TO_USER_SOCK]);
    m_nbd_sockets[TO_USER_SOCK] = -1;
    
    close(m_comm_sockets[FROM_COMMUNICATOR_SOCK]);
    m_nbd_sockets[FROM_COMMUNICATOR_SOCK] = -1;

    close(m_nbd_fd);
    m_nbd_fd = -1;
}

NBDCommunicator::NBD_COMM_STATUS NBDCommunicator::SetupChannels()
{
    if(-1 == ioctl(m_nbd_fd, NBD_DISCONNECT))
    {
        std::cerr << "[Error] Failed to request disconect on the nbd device" << std::endl;
    }
    
    m_nbd_fd = open(m_dev_file_path.c_str(), O_RDWR);
    if (-1 == m_nbd_fd)
    {
        std::cerr << "[Error] SetupChannels: open" << std::endl;
        return (NBD_COMM_FAILURE);
    }

    if (-1 == ioctl(m_nbd_fd, NBD_CLEAR_SOCK))
    {
        std::cerr << "[Error] SetupChannels: NBD_CLEAR_SOCK" << std::endl;
        return (NBD_COMM_FAILURE);
    }

    if (-1 == ioctl(m_nbd_fd, NBD_SET_SIZE, m_nbd_size))
    {
        std::cerr << "[Error] SetupChannels: NBD_SET_SIZE" << std::endl;
        return (NBD_COMM_FAILURE);
    }

    if (-1 == socketpair(AF_UNIX, SOCK_STREAM, 0, m_nbd_sockets))
    {
        std::cerr << "[Error] SetupChannels: socketpair" << std::endl;
        return (NBD_COMM_FAILURE);
    }

    if (-1 == socketpair(AF_UNIX, SOCK_STREAM, 0, m_comm_sockets))
    {
        std::cerr << "[Error] SetupChannels: pipe" << std::endl;
        return (NBD_COMM_FAILURE);
    }

    return (NBD_COMM_SUCCESS);
}

void NBDCommunicator::ServerDriver()
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGRTMIN);

    if (0 != pthread_sigmask(SIG_BLOCK, &set, nullptr))
    {
        std::cerr << "[Error] ServerDriver: pthread_sigmask error" << std::endl;
    }

    if(-1 == ioctl(m_nbd_fd, NBD_SET_SOCK, m_nbd_sockets[SERVER_SOCK]))
    {
        StopCommunicator();
        std::cerr << "[Error] ServerDriver: NBD_SET_SOCK" << std::endl;
    }

    if (-1 == ioctl(m_nbd_fd, NBD_DO_IT))
    {
        StopCommunicator();
        std::cerr << "[Error] ServerDriver: NBD_DO_IT" << std::endl;
    }

    ioctl(m_nbd_fd, NBD_CLEAR_QUE);
    ioctl(m_nbd_fd, NBD_CLEAR_SOCK);

    close(m_nbd_sockets[SERVER_SOCK]);
}

void NBDCommunicator::TranslatorDriver()
{    
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGRTMIN);

    if (0 != pthread_sigmask(SIG_BLOCK, &set, nullptr))
    {
        std::cerr << "[Error] TranslatorDriver: pthread_sigmask error" << std::endl;
    }

    nbd_request_t request;

    while (true)
    {
        if (-1 == read(m_nbd_sockets[TRANSLATOR_SOCK], &request, sizeof(request)))
        {
            StopCommunicator();
            std::cerr << "[Error] TranslatorDriver: error reading nbd socket" << std::endl;
            return;
        }

        uint32_t req_type = ntohl(request.type);

        if (NBD_CMD_DISC == req_type)
        {
            HandleDisc();
            return;
        }
        else if ((NBD_CMD_READ == req_type && NBD_COMM_FAILURE == HandleRead(request))
               ||(NBD_CMD_WRITE == req_type && NBD_COMM_FAILURE == HandleWrite(request)))
        {
            StopCommunicator();
            return;
        }
    }
}

NBDCommunicator::NBD_COMM_STATUS NBDCommunicator::HandleRead(const nbd_request_t &request)
{
    unsigned len = ntohl(request.len);
    std::size_t from = ntohll(request.from);
    EventType req_type = READ;

    if (NBD_COMM_FAILURE == TranslateReqType(ntohl(request.type), req_type))
    {
        std::cerr << "[Error] Reply: translating type" << std::endl;
        return (NBD_COMM_FAILURE);
    }

    Event event({req_type, from, len, {0}});
    std::memcpy(event.m_event_id, request.handle, sizeof(request.handle));

    if (-1 == write(m_comm_sockets[FROM_COMMUNICATOR_SOCK], &event, sizeof(Event)))
    {
        std::cerr << "[Error] HandleRead: writing event" << std::endl;

        return (NBD_COMM_FAILURE);
    }

    return (NBD_COMM_SUCCESS);
}

NBDCommunicator::NBD_COMM_STATUS NBDCommunicator::HandleWrite(const nbd_request_t &request)
{
    unsigned len = ntohl(request.len);
    std::size_t from = ntohll(request.from);
    EventType req_type = READ;

    if (NBD_COMM_FAILURE == TranslateReqType(ntohl(request.type), req_type))
    {
        std::cerr << "[Error] Reply: translating type" << std::endl;
        return (NBD_COMM_FAILURE);
    }

    Event event({req_type, from, len, {0}});
    std::memcpy(event.m_event_id, request.handle, sizeof(request.handle));

    char *chunk = static_cast<char *>(operator new(len));

    if (-1 == ReadData(m_nbd_sockets[TRANSLATOR_SOCK], chunk, len))
    {
        operator delete(chunk);
        std::cerr << "[Error] HandleWrite: reading data" << std::endl;

        return (NBD_COMM_FAILURE);
    }
    
    if (-1 == write(m_comm_sockets[FROM_COMMUNICATOR_SOCK], &event, sizeof(Event)))
    {
        operator delete(chunk);
        std::cerr << "[Error] HandleWrite: writing event" << std::endl;

        return (NBD_COMM_FAILURE);
    }

    if (-1 == WriteData(m_comm_sockets[FROM_COMMUNICATOR_SOCK], chunk, len))
    {
        operator delete(chunk);
        std::cerr << "[Error] HandleWrite: writing data" << std::endl;

        return (NBD_COMM_FAILURE);
    }

    operator delete(chunk);

    return (NBD_COMM_SUCCESS);
}

void NBDCommunicator::HandleDisc() noexcept
{
    close(m_nbd_sockets[TRANSLATOR_SOCK]);
}

NBDCommunicator::NBD_COMM_STATUS NBDCommunicator::TranslateReqType(uint32_t nbd_type, EventType &event_type)
{
    switch (nbd_type)
    {
        case (NBD_CMD_READ):
        {
            event_type = READ;
            return (NBD_COMM_SUCCESS);
        }
        case (NBD_CMD_WRITE):
        {
            event_type = WRITE;
            return (NBD_COMM_SUCCESS);
        }
        default:
        {
            std::cerr << "[Error] TranslateReqType: unsupported type" << std::endl;
            return (NBD_COMM_FAILURE);
        }
    }
}

NBDCommunicator::NBD_COMM_STATUS NBDCommunicator::TranslateResType(StatusType type, uint32_t &nbd_type)
{
    switch (type)
    {
        case (SUCCESS):
        {
            nbd_type = NBD_SUCCESS;
            return (NBD_COMM_SUCCESS);
        }
        case (INVALID_REQUEST):
        {
            nbd_type = NBD_EINVAL;
            return (NBD_COMM_SUCCESS);
        }
        case (OUT_OF_SPACE):
        {
            nbd_type = NBD_ENOSPC;
            return (NBD_COMM_SUCCESS);
        }
        case (IO_ERROR):
        {
            nbd_type = NBD_EIO;
            return (NBD_COMM_SUCCESS);
        }
        default:
        {
            std::cerr << "[Error] TranslateReqType: unsupported type" << std::endl;
            return (NBD_COMM_FAILURE);
        }
    }
}

const char *NBDCommunicator::NBDCommOpenChannelsExc::what() const noexcept
{
    return ("Couldn't open channels");
}

namespace
{
int WriteData(int fd, const char *buf, std::size_t count)
{
    int bytes_written;

    assert(nullptr != buf);

    while (0 < count && 0 < (bytes_written = write(fd, buf, count)))
    {
        buf += bytes_written;
        count -= bytes_written;
    }

    return (bytes_written);
}

int ReadData(int fd, char *buf, std::size_t count)
{
  int bytes_read;

  assert(nullptr != buf);

  while (0 < count && 0 < (bytes_read = read(fd, buf, count)))
  {
    buf += bytes_read;
    count -= bytes_read;
  }

  return (bytes_read);
}

#ifdef WORDS_BIGENDIAN
u_int64_t ntohll(u_int64_t a)
{
  return (a);
}
#else
u_int64_t ntohll(u_int64_t a)
{
  u_int32_t lo = a & 0xffffffff;
  u_int32_t hi = a >> 32U;
  lo = ntohl(lo);
  hi = ntohl(hi);
  return (((u_int64_t) lo) << 32U | hi);
}
#endif
} // anonimus