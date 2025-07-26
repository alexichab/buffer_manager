#ifndef BUFFER_H
#define BUFFER_H
#include <vector>
#include <mutex>

class Buffer {
public:
    void insert(int pos, int value);
    void remove(int pos);
    void sort_asc();
    void sort_desc();
    void reverse();
    std::vector<int> get_copy() const;
    size_t size() const;
private:
    std::vector<int> data_;
    mutable std::mutex mtx_;
};

#endif // BUFFER_H 