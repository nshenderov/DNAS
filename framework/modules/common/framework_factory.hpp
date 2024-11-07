#ifndef NSRD_FRAMEWORK_FACTORY_HPP
#define NSRD_FRAMEWORK_FACTORY_HPP

#include <functional> // std::function

#include "factory.hpp" // nsrd::Factory
#include "ICommand.hpp" // nsrd::ICommand

namespace nsrd
{
typedef Factory<ICommand, builder_id_t, ICommandParams *, std::function<ICommand*(ICommandParams *)> > framework_factory_t;
} // namespace nsrd

#endif // NSRD_FRAMEWORK_FACTORY_HPP