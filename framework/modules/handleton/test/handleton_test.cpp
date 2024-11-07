/*******************************************************************************
*
* FILENAME : handleton_test.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 20.09.2023
* 
*******************************************************************************/

#include <iostream> // std::cout, std::endl
#include <dlfcn.h> // dlopen, dlsym, dlclose, dlerror

#include "testing.hpp" // nsrd::testing::Testscmp, nsrd::testing::UnitTest

#define NOT_HANDLETON

#include "base.hpp"
#include "handleton.hpp" // nsrd::Handleton

using nsrd::Handleton;
using namespace nsrd::testing;

namespace
{
void TestHandleton();
} // namespace

int main()
{  
    Testscmp cmp;
    cmp.AddTest(UnitTest("Hadnleton", TestHandleton));
    cmp.Run();
    
    return (0);
}

namespace
{
void TestHandleton()
{
    using func = Base *(*)();

    Base *s = Handleton<Base>::GetInstance();

    std::cout << "Handleton in process: " << s << std::endl;
    
    s->p1 = 1;
    s->p2 = 2;

    void *handle = dlopen("./test/libplugin.so", RTLD_LAZY);

    TH_ASSERT(handle);

    dlerror();

    char *error;

    func f = (func) dlsym(handle, "instance");

    TH_ASSERT(NULL == dlerror());

    f();

    dlclose(handle);

    TH_ASSERT(s->p1 == 11);
    TH_ASSERT(s->p2 == 22);

    (void) error;
}
} // namespace