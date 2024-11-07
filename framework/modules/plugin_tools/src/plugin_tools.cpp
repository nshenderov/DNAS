#define NOT_HANDLETON

#include "handleton.hpp" // nsrd::Handleton
#include "factory.hpp" // nsrd::Factory
#include "framework_factory.hpp" // concrete nsrd::Factory
#include "plugin_tools.hpp" // plugin tools

void nsrd::RegisterBuilder(builder_t builder, builder_id_t type)
{
    nsrd::framework_factory_t *factory = nsrd::Handleton<nsrd::framework_factory_t>::GetInstance();

    factory->AddCreator(builder, type);
}

void nsrd::RemoveBuilder(builder_id_t type)
{
    nsrd::framework_factory_t *factory = nsrd::Handleton<nsrd::framework_factory_t>::GetInstance();

    factory->RemoveCreator(type);
}