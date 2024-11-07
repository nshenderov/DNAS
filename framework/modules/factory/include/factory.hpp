/*******************************************************************************
*
* FILENAME : factory.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 15.09.2023
* 
*******************************************************************************/

#ifndef NSRD_FACTORY_HPP
#define NSRD_FACTORY_HPP

#include <map> // std::map
#include <stdexcept> // std::invalid_argument

namespace nsrd
{
template<typename BASE,
         typename KEY,
         typename PARAMS,
         typename BUILDER = BASE *(*)(PARAMS agrs)>
class Factory
{
public:
    Factory() = default;
    ~Factory() = default;

    Factory(Factory &other) = delete;
    Factory(Factory &&other) = delete;
    Factory &operator=(Factory &other) = delete;
    Factory &operator=(Factory &&other) = delete;

    void AddCreator(BUILDER creator, KEY id); // can throw invalid_argument
    void RemoveCreator(KEY id); // can throw any exceptions thrown by the id

    BASE *Create(KEY id, PARAMS args) const; // can throw invalid_argument

private:
    std::map<KEY, BUILDER> m_creators;
};

template<typename BASE, typename KEY, typename PARAMS, typename BUILDER>
void Factory<BASE, KEY, PARAMS, BUILDER>::AddCreator(BUILDER creator, KEY id)
{
    if (m_creators.find(id) != m_creators.end())
    {
        throw std::invalid_argument("id is already inside the factory");
    }

    m_creators[id] = creator;
}

template<typename BASE, typename KEY, typename PARAMS, typename BUILDER>
void Factory<BASE, KEY, PARAMS, BUILDER>::RemoveCreator(KEY id)
{
    m_creators.erase(id);
}

template<typename BASE, typename KEY, typename PARAMS, typename BUILDER>
BASE *Factory<BASE, KEY, PARAMS, BUILDER>::Create(KEY id, PARAMS args) const
{
    auto it = m_creators.find(id);
    if (it == m_creators.end())
    {
        throw std::invalid_argument("wrong id");
    }

    return (it->second(args));
}
} // namespace nsrd


#endif // NSRD_FACTORY_HPP