#ifndef COMMAND_H
#define COMMAND_H
#include "buffer.h"
#include <ostream>

class ICommand {
public:
    virtual void execute(Buffer& buffer, std::ostream& out) = 0;
    virtual ~ICommand() = default;
};

class InsertCommand : public ICommand {
    int pos_, value_;
public:
    InsertCommand(int pos, int value);
    void execute(Buffer& buffer, std::ostream& out) override;
};
class RemoveCommand : public ICommand {
    int pos_;
public:
    RemoveCommand(int pos);
    void execute(Buffer& buffer, std::ostream& out) override;
};
class SortAscCommand : public ICommand {
public:
    void execute(Buffer& buffer, std::ostream& out) override;
};
class SortDescCommand : public ICommand {
public:
    void execute(Buffer& buffer, std::ostream& out) override;
};
class ReverseCommand : public ICommand {
public:
    void execute(Buffer& buffer, std::ostream& out) override;
};
class ReadCommand : public ICommand {
public:
    void execute(Buffer& buffer, std::ostream& out) override;
};
class CountEvenOddCommand : public ICommand {
public:
    void execute(Buffer& buffer, std::ostream& out) override;
};

#endif // COMMAND_H 