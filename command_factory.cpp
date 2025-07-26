#include "command_factory.h"

std::unique_ptr<ICommand> CommandFactory::create(const std::string& cmd, std::istringstream& iss) {
    if (cmd == "insert") {
        int pos, value;
        if (iss >> pos >> value) {
            return std::make_unique<InsertCommand>(pos, value);
        }
    } else if (cmd == "remove") {
        int pos;
        if (iss >> pos) {
            return std::make_unique<RemoveCommand>(pos);
        }
    } else if (cmd == "sort") {
        std::string dir;
        if (iss >> dir) {
            if (dir == "asc") return std::make_unique<SortAscCommand>();
            if (dir == "desc") return std::make_unique<SortDescCommand>();
        }
    } else if (cmd == "reverse") {
        return std::make_unique<ReverseCommand>();
    } else if (cmd == "read") {
        return std::make_unique<ReadCommand>();
    } else if (cmd == "count") {
        return std::make_unique<CountEvenOddCommand>();
    }
    return nullptr;
} 