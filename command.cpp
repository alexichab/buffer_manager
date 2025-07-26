#include "command.h"
#include <iostream>

InsertCommand::InsertCommand(int pos, int value) : pos_(pos), value_(value) {}
void InsertCommand::execute(Buffer& buffer, std::ostream& out) {
    buffer.insert(pos_, value_);
    out << "[LOG] Inserted " << value_ << " at position " << pos_ << std::endl;
}
RemoveCommand::RemoveCommand(int pos) : pos_(pos) {}
void RemoveCommand::execute(Buffer& buffer, std::ostream& out) {
    buffer.remove(pos_);
    out << "[LOG] Removed element at position " << pos_ << std::endl;
}
void SortAscCommand::execute(Buffer& buffer, std::ostream& out) {
    buffer.sort_asc();
    out << "[LOG] Sorted buffer in ascending order" << std::endl;
}
void SortDescCommand::execute(Buffer& buffer, std::ostream& out) {
    buffer.sort_desc();
    out << "[LOG] Sorted buffer in descending order" << std::endl;
}
void ReverseCommand::execute(Buffer& buffer, std::ostream& out) {
    buffer.reverse();
    out << "[LOG] Reversed buffer" << std::endl;
}
void ReadCommand::execute(Buffer& buffer, std::ostream& out) {
    auto copy = buffer.get_copy();
    out << "Buffer: [";
    for (size_t i = 0; i < copy.size(); ++i) {
        if (i != 0) out << ", ";
        out << copy[i];
    }
    out << "]" << std::endl;
}
void CountEvenOddCommand::execute(Buffer& buffer, std::ostream& out) {
    size_t size = buffer.size();
    int even_count = (size + 1) / 2;
    int odd_count = size / 2;
    out << "Even positions: " << even_count << std::endl;
    out << "Odd positions: " << odd_count << std::endl;
} 