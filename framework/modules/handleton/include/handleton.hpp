/*******************************************************************************
*
* FILENAME : handleton.hpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 20.09.2023
* 
*******************************************************************************/

#ifndef NSRD_HANDLETON_HPP
#define NSRD_HANDLETON_HPP

#include <mutex> // std::mutex
#include <atomic> // std::atomic

namespace nsrd
{
// T must has Default constructor
template <typename T>
class Handleton
{
public:
    static T *GetInstance();

    Handleton(Handleton &other) = delete;
    void operator=(const Handleton &) = delete;

private:
    Handleton() = default;

    static void DeleteInstance();

    static std::mutex s_mutex;
    static std::atomic<T *> s_instance;
    static bool m_is_destroyed;
};

#ifndef NOT_HANDLETON

template <typename T>
std::mutex Handleton<T>::s_mutex;

template <typename T>
bool Handleton<T>::m_is_destroyed = false;

template <typename T>
std::atomic<T *> Handleton<T>::s_instance(0);

template <typename T>
T *Handleton<T>::GetInstance()
{
    if (0 == s_instance)
    {
        if (m_is_destroyed)
        {
            throw std::runtime_error("instantiation of the deleted singleton");
        }

        const std::lock_guard<std::mutex> lock(s_mutex);

        if (0 == s_instance)
        {
            s_instance = new T;
            std::atexit(DeleteInstance);
        }
    }

    return (s_instance);
}

template <typename T>
void Handleton<T>::DeleteInstance()
{
    m_is_destroyed = true;
    delete s_instance;
    s_instance = 0;
}
#endif
} // namespace nsrd

#endif // NSRD_HANDLETON_HPP