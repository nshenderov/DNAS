/*******************************************************************************
*
* FILENAME : nbd_comm_proxy.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 01.11.2023
* 
*******************************************************************************/

#include <iostream> // std::cerr, std::endl
#include "nbd_comm_proxy.hpp" // nsrd::NBDCommProxy

#define RED "\033[0;31m"
#define NC "\033[0m"

template class nsrd::Handleton<nsrd::NBDCommProxy>;

nsrd::NBDCommunicator *nsrd::NBDCommProxy::s_nbd = nullptr;

void nsrd::NBDCommProxy::SetNBDCommProxy(const std::string &dev_file_path, std::size_t nbd_size) noexcept
{
    try
    {
        s_nbd = new NBDCommunicator(dev_file_path, nbd_size);
    }
    catch (const NBDCommunicator::NBDCommOpenChannelsExc &e)
    {
        std::cerr
        << RED
        << "Make sure that the nbd device is exist\n"
        << "and you run the program with 'sudo'"
        << NC << std::endl;

        exit(-1);
    }
}

nsrd::NBDCommProxy::NBDCommProxy()
{}

nsrd::NBDCommProxy::~NBDCommProxy()
{
    delete s_nbd;
}

int nsrd::NBDCommProxy::GetNBDFileDescriptor() noexcept
{
    return (s_nbd->GetNBDFileDescriptor());
}

void nsrd::NBDCommProxy::Reply(nsrd::NBDCommunicator::StatusType status, const char *event_id, std::size_t length, const void *buf)
{
    s_nbd->Reply(status, event_id, length, buf);
}