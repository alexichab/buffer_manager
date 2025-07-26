#include "buffer.h"
#include <algorithm>

void Buffer::insert(int pos, int value) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (pos >= 0 && pos <= static_cast<int>(data_.size())) {
        data_.insert(data_.begin() + pos, value);
    }
}
void Buffer::remove(int pos) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (pos >= 0 && pos < static_cast<int>(data_.size())) {
        data_.erase(data_.begin() + pos);
    }
}
void Buffer::sort_asc() {
    std::lock_guard<std::mutex> lock(mtx_);
    std::sort(data_.begin(), data_.end());
}
void Buffer::sort_desc() {
    std::lock_guard<std::mutex> lock(mtx_);
    std::sort(data_.begin(), data_.end(), std::greater<int>());
}
void Buffer::reverse() {
    std::lock_guard<std::mutex> lock(mtx_);
    std::reverse(data_.begin(), data_.end());
}
std::vector<int> Buffer::get_copy() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return data_;
}
size_t Buffer::size() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return data_.size();
} 