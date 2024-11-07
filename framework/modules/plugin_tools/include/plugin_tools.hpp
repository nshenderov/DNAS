#ifndef NSRD_PLUGIN_TOOLS_HPP
#define NSRD_PLUGIN_TOOLS_HPP

#include "ICommand.hpp" // nsrd::ICommand

namespace nsrd
{
void RegisterBuilder(builder_t builder, builder_id_t type);
void RemoveBuilder(builder_id_t type);
} // namespace nsrd

#endif // NSRD_PLUGIN_TOOLS_HPP
