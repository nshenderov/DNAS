/*******************************************************************************
*
* FILENAME : minion_daemon.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 23.11.2023
* 
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <string>

#include "minion.hpp"

#define YELLOW "\033[0;33m"
#define RED "\033[0;31m"
#define NC "\033[0m"

using namespace nsrd;
namespace
{
enum MINION_ARG {PORT = 1, STORAGE_SIZE, MINION_ARGS};
void ValidateArguments(int argc, char const *argv[]);
void Demonize();
}

int main(int argc, char const *argv[])
{
    ValidateArguments(argc, argv);

    Demonize();

    Minion minion(std::stoul(argv[PORT]), std::stoul(argv[STORAGE_SIZE]));
    minion.Run();

    return (EXIT_SUCCESS);
}

namespace
{
void ValidateArguments(int argc, char const *argv[])
{
    if (MINION_ARGS != argc || !argv[PORT] || !argv[STORAGE_SIZE])
    {
        std::cout
        << RED
        << "You must provide neccessary arguments\n"
        << "1: minion port\n"
        << "2: minion size\n"
        << "example:\n"
        << "./minion_daemon_aarch64.out 1501 128"
        << NC << std::endl;

        exit(EXIT_FAILURE);
    }
}

void Demonize()
{
    pid_t pid;
    
    pid = fork();
    
    if (pid < 0)
    {
        exit(EXIT_FAILURE);
    }

    if (pid > 0)
    {
        exit(EXIT_SUCCESS);
    }

    if (setsid() < 0)
    {
        exit(EXIT_FAILURE);
    }

    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    
    pid = fork();
    
    if (pid < 0)
    {
        exit(EXIT_FAILURE);
    }

    if (pid > 0)
    {
        exit(EXIT_SUCCESS);
    }

    for (int x = sysconf(_SC_OPEN_MAX); x >= 0; --x)
    {
        close(x);
    }
}
} // namespace

