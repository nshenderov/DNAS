/*******************************************************************************
*
* FILENAME : ICommand.hpp
* 
* AUTHOR : Nikita Shenderov
*
* DATE : 24.10.2023
* 
*******************************************************************************/

#ifndef NSRD_ICOMMAND_HPP
#define NSRD_ICOMMAND_HPP

#ifndef NDEBUG
#include <string>
#endif

namespace nsrd
{
class ICommand
{
public:
    virtual ~ICommand() =0;
    
    virtual void operator()() =0;
};

class ICommandParams
{
public:
    virtual ~ICommandParams() =0;
};


typedef ICommand *(*builder_t)(ICommandParams *);
typedef int builder_id_t;

#ifndef NDEBUG
struct TestParams: public ICommandParams
{
    std::string m_message;
};
#endif
} // namespace nsrd

#endif // NSRD_ICOMMAND_HPP
