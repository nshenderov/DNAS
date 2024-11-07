#include <iostream>
#include <unistd.h>

#include "plugin_tools.hpp"

using namespace nsrd;

void PluginLoad() __attribute__((constructor));
void PluginUnload() __attribute__((destructor));

class TestPlugin: public nsrd::ICommand
{
public:
    TestPlugin(nsrd::ICommandParams *params);
    void operator()();
    TestParams *m_params;
};

TestPlugin::TestPlugin(ICommandParams *params)
    : m_params(dynamic_cast<TestParams *>(params))
{}

void TestPlugin::operator()()
{
    std::cout << "[TESTING_PLUGIN] Called command's operator()" << std::endl;
    std::cout << "[TESTING_PLUGIN] Has been readed from socket: " << m_params->m_message << std::endl;
}

nsrd::ICommand *ReadEvtCommandBuilder(ICommandParams *params)
{
    return (new TestPlugin(params));
}

void PluginLoad()
{
    std::cout << "[TESTING_PLUGIN] Plugin load" << std::endl;

    nsrd::RegisterBuilder(ReadEvtCommandBuilder, 0);
}

void PluginUnload()
{
    std::cout << "[TESTING_PLUGIN] Plugin unload" << std::endl;

    nsrd::RemoveBuilder(0);
}
