#ifndef COMMAND_FACTORY_H
#define COMMAND_FACTORY_H
#include "command.h"
#include <memory>
#include <sstream>
#include <string>

class CommandFactory {
public:
    static std::unique_ptr<ICommand> create(const std::string& cmd, std::istringstream& iss);
};

#endif // COMMAND_FACTORY_H 