/*******************************************************************************
*
* FILENAME : dnas.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 28.11.2023
* 
*******************************************************************************/

#define NOT_HANDLETON

#include <fstream>
#include <iostream>
#include <string>
#include <arpa/inet.h>

#include "framework.hpp"
#include "handleton.hpp"
#include "minion_manager.hpp"
#include "raid_manager.hpp"
#include "nbd_params.hpp"
#include "nbd_communicator.hpp"
#include "nbd_comm_proxy.hpp"
#include "minion_proxy.hpp"
#include "scheduler.hpp"


#define YELLOW "\033[0;33m"
#define RED "\033[0;31m"
#define NC "\033[0m"

using namespace nsrd;
namespace
{
enum NAS_ARG {DEV_PATH = 1, DEV_SIZE, PLUG_PATH, MINION_ADDRS_PATH, NAS_ARGS};
void ValidateArguments(int argc, char const *argv[]);
void ClosingDriver(Framework *fr);
std::size_t ComputeDevSize(std::size_t mb);
int ReadData(int fd, char *buf, std::size_t count);
std::pair<builder_id_t, ICommandParams * > HandleSocketRead(int fd);
void GetMinions(char const *argv[]);
} // namespace anonimus


int main(int argc, char const *argv[])
{
    ValidateArguments(argc, argv);

    std::size_t dev_size = ComputeDevSize(std::stoul(argv[DEV_SIZE]));

    Scheduler *scheduler = Handleton<Scheduler>::GetInstance();

    GetMinions(argv);

    nsrd::NBDCommProxy::SetNBDCommProxy(argv[DEV_PATH], dev_size);
    nsrd::NBDCommProxy *nbd = nsrd::Handleton<nsrd::NBDCommProxy>::GetInstance();

    std::cout << YELLOW "NBD intialized" << NC << std::endl;

    nsrd::RaidManager *raid_manager = nsrd::Handleton<nsrd::RaidManager>::GetInstance();

    Framework fr(argv[PLUG_PATH]);

    fr.AddSocketHandler(nbd->GetNBDFileDescriptor(), SocketEventType::READ, HandleSocketRead);

    std::cout
    << YELLOW "Framework intialized\n"
    << "TYPE 'run' TO RUN THE PROGRAM\n"
    << "NOTE THAT YOU NEED TO ADD SOME PLUGINS FIRST"
    << NC << std::endl;

    for (size_t i = 0; i < 10; ++i)
    {
        sleep(1);
    };
    
    std::string s;

    while ("run" != s)
    {
        std::cin >> s;
    }

    std::thread stop_thread = std::thread(&ClosingDriver, &fr);

    sleep(2);

    fr.Run();

    stop_thread.join();

    return (0);
    (void) raid_manager;
    (void) scheduler;
}

namespace
{
void ValidateArguments(int argc, char const *argv[])
{
    if (NAS_ARGS != argc || !argv[DEV_PATH] || !argv[DEV_SIZE] || !argv[PLUG_PATH] || !argv[MINION_ADDRS_PATH])
    {
        std::cout
        << RED
        << "You must provide neccessary arguments\n"
        << "1: path to the device\n"
        << "2: size of the device in mb\n"
        << "3: path to the plugins folder\n"
        << "4: path to minion addresses file (ip:port, amount > 4 && 0 == amount % 2)\n"
        << "example:\n"
        << "./dnas.out /dev/nbd0 128 ./plugins/ ./minion_addrs.txt" 
        << NC << std::endl;

        exit(-1);
    }
}

void ClosingDriver(Framework *fr)
{
    std::cout
    << YELLOW
    << "THE PROGRAM IS RUNNING\n"
    << "TYPE 'stop' TO STOP THE EXECUTION"
    << NC << std::endl;

    std::string s;
    
    while ("stop" != s)
    {
        std::cin >> s;
    }
    
    fr->Stop();
}

inline std::size_t ComputeDevSize(std::size_t mb)
{
    return (mb * (1024 * 1024));
}

int ReadData(int fd, char *buf, std::size_t count)
{
  int bytes_read = 0;

  while (0 < count && 0 < (bytes_read = read(fd, buf, count)))
  {
    buf += bytes_read;
    count -= bytes_read;
  }

  return (bytes_read);
}

std::pair<builder_id_t, ICommandParams * > HandleSocketRead(int fd)
{
    #ifndef NDEBUG
    std::cout << "[HANDLE_SOCKET_READ] Processing socket" << std::endl;
    #endif

    NBDCommunicator::Event event;

    if (-1 == read(fd, &event, sizeof(NBDCommunicator::Event)))
    {
        std::cerr << "[HANDLE_SOCKET_READ] Error reading socket" << std::endl;
    }

    NBDParams *params = new NBDParams();
    params->m_len = event.m_length;
    params->m_offset = event.m_offset;
    std::memcpy(params->m_event_id, event.m_event_id, sizeof(sizeof(char) * 8));

    if (NBDCommunicator::WRITE == event.m_type)
    {
        params->m_data = static_cast<char *>(operator new(params->m_len));
        ReadData(fd, params->m_data, params->m_len);
    }

    return (std::pair<builder_id_t, ICommandParams*>(event.m_type, params));
}

void GetMinions(char const *argv[])
{
    std::ifstream minion_list_stream(argv[MINION_ADDRS_PATH]);

    if (!minion_list_stream.is_open())
    {
        std::cerr << "Couldn't open minion addresses list file" << std::endl;
        exit(-1);
    }

    nsrd::MinionManager *manager = nsrd::Handleton<nsrd::MinionManager>::GetInstance();

    std::size_t minion_counter = 0;

    for (std::string address; std::getline(minion_list_stream, address);)
    {
        std::istringstream iss(address);
        std::size_t delim_pos = address.find_first_of(':');
        if (std::string::npos == delim_pos)
        {
            std::cerr << "Wrong address" << std::endl;
            exit(-1);
        }
        
        std::string str_port = address.substr(delim_pos+1),
                    str_ip = address.substr(0, delim_pos);

        if (0 == str_port.length() || 0 == str_ip.length())
        {
            std::cerr << "Wrong address" << std::endl;
            exit(-1);
        }
        

        unsigned char buf[sizeof(in_addr)] = {0};
        int pton_status = inet_pton(AF_INET, str_ip.c_str(), buf);
        if (-1 == pton_status || 0 == pton_status)
        {
            std::cerr << "Wrong address" << std::endl;
            exit(-1);
        }

        std::shared_ptr<MinionProxy> minion(new MinionProxy(str_ip.c_str(), std::stoul(str_port)));
        manager->AddMinion(minion_counter++, minion);

        (void) buf;
    }

    if (minion_counter < 4 || 0 != minion_counter % 2)
    {
        std::cerr << "minion_amount > 4 && 0 == minion_amount % 2" << std::endl;
        exit(-1);
    }

    nsrd::RaidManager::SetMinionCount(minion_counter);
}
} // namespace anonimus