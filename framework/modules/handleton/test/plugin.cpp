/*******************************************************************************
*
* FILENAME : plugin.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 20.09.2023
* 
*******************************************************************************/

#include <iostream> // std::cout, std::endl

#define NOT_HANDLETON

#include "base.hpp"
#include "handleton.hpp"

using nsrd::Handleton;

extern "C" void instance()
{
    Base *s = Handleton<Base>::GetInstance();

    s->p1 += 10;
    s->p2 += 20;

    std::cout << "Handleton in plugin: " << s << std::endl;
}