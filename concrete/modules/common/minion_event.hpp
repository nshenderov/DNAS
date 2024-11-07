/*******************************************************************************
*
* FILENAME : minion_event.hpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 19.11.2023
* 
*******************************************************************************/

#ifndef NSRD_MINION_EVENT_HPP
#define NSRD_MINION_EVENT_HPP

#include <cstddef>
#include <string>

namespace nsrd
{
enum class MinionEventType
{
    READ = 0,
    WRITE,
    START_COMMUNICATE,
    STOP_COMMUNICATE,
    RESPONSE_SUCCESS,
    RESPONSE_FAIL
};

const unsigned MINION_EVENT_MAGIC = 0xFA1AFE1;

struct MinionEvent
{
    MinionEventType m_type;
    unsigned m_magic;
    std::size_t m_event_id;
    std::size_t m_offset;
    std::size_t m_length;
};
// minion proxy can send READ, WRITE, COMMUNICATE_ONLY_WITH_ME or STOP_COMMUNICATE
// if minion proxy sends WRITE he should send data after it (data number of bytes == m_length)
// minion can send RESPONSE_SUCCESS or RESPONSE_FAIL
// if minion sends RESPONSE_SUCCESS on READ event, he should send data after it (data number of bytes == m_length)
// m_offset in minion response migth be any value, it will have no effect

} // namespace nsrd

#endif // NSRD_MINION_EVENT_HPP