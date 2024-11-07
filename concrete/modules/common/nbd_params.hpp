/*******************************************************************************
*
* FILENAME : nbd_params.hpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 02.11.2023
* 
*******************************************************************************/

#ifndef NSRD_NBD_PARAMS_HPP
#define NSRD_NBD_PARAMS_HPP

#include "ICommand.hpp" // nsrd::ICommandParams

namespace nsrd
{
// const std::size_t MINIONS_AMOUNT = 6;

struct NBDParams: public ICommandParams
{
    char *m_data;
    unsigned m_len;
    std::size_t m_offset;
    char m_event_id[8];
};
} // namespace nsrd

#endif // NSRD_NBD_PARAMS_HPP