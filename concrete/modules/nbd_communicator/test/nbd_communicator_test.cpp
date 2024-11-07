/*******************************************************************************
*
* FILENAME : nbd_communicator_test.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 19.10.2023
* 
*******************************************************************************/

#include <unistd.h> // write, read
#include <cstring> // std::memcpy
#include <poll.h> // poll

#include "testing.hpp" // testing::Testscmp, testing::UnitTest

#include "nbd_communicator.hpp" // nsrd::NBDCommunicator

using namespace nsrd;
using namespace nsrd::testing;

namespace
{
void TestNBD();
} // namespace

int main()
{
    Testscmp cmp;
    cmp.AddTest(UnitTest("NBD", TestNBD));
    cmp.Run();

    return (0);
}

namespace
{
char *data;

void NBDRead(char *buf, unsigned len, std::size_t offset)
{
  std::cout << "R - " << offset << ", " << len << std::endl;
  std::memcpy(buf, data + offset, len);
}

void NBDWrite(const char *buf, unsigned len, std::size_t offset)
{
  std::cout << "W - " << offset << ", " << len << std::endl;
  std::memcpy(data + offset, buf, len);
}

int ReadData(int fd, char *buf, std::size_t count)
{
  int bytes_read;

  while (0 < count && 0 < (bytes_read = read(fd, buf, count)))
  {
    buf += bytes_read;
    count -= bytes_read;
  }

  return (bytes_read);
}

void ClosingDriver(bool *stop_flag)
{
    std::cout << "TYPE 'stop' TO STOP THE PROGRAM" << std::endl;

    std::string s;
    
    while ("stop" != s)
    {
        std::cin >> s;
    }

    *stop_flag = true;
}

bool pollIn(struct pollfd *pfd)
{
    bool status = false;

    if (0 < poll(pfd, 1, 0))
    {
        if (pfd->revents & POLLIN)
        {
            status = true;
        }
    }

    return(status);
}

void TestNBD()
{
    std::size_t nbd_size = 128 * (1024 * 1024);
    std::string nbd_dev_path = "/dev/nbd0";

    data = static_cast<char *>(operator new(nbd_size));
    std::memset(data, 0, nbd_size);

    NBDCommunicator nbd(nbd_dev_path, nbd_size);
    int nbd_fd = nbd.GetNBDFileDescriptor();

    struct pollfd *pfd = new struct pollfd({nbd_fd, POLLIN, 0});
    
    bool stop = false;
    std::thread stop_thread = std::thread(&ClosingDriver, &stop);

    sleep(2);

    while(!stop)
    {
        if(pollIn(pfd))
        {
            NBDCommunicator::Event event;

            ssize_t r = read(nbd_fd, &event, sizeof(NBDCommunicator::Event));
            (void) r;

            unsigned len = event.m_length;
            std::size_t from = event.m_offset;

            switch (event.m_type)
            {
                case (NBDCommunicator::READ):
                {
                    char *chunk = static_cast<char *>(operator new(len));

                    NBDRead(chunk, len, from);
                    
                    nbd.Reply(NBDCommunicator::StatusType::SUCCESS, event.m_event_id, len, chunk);

                    operator delete(chunk);

                    break;
                }
                
                case (NBDCommunicator::WRITE):
                {
                    char *chunk = static_cast<char *>(operator new(len));

                    ReadData(nbd_fd, chunk, len);

                    NBDWrite(chunk, len, from);

                    operator delete(chunk);

                    nbd.Reply(NBDCommunicator::StatusType::SUCCESS, event.m_event_id);

                    break;
                }
            }
        }
    }
    
    stop_thread.join();

    operator delete(data);

    delete pfd;
}  
} // namespace