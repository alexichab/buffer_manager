#include "threads.h"

WriterThread::WriterThread(std::queue<std::unique_ptr<ICommand>>& tasks,
                 std::mutex& queue_mutex,
                 std::condition_variable& queue_cv,
                 Buffer& buffer,
                 std::ostream& out,
                 std::atomic<bool>& stop_flag)
    : tasks_(tasks), queue_mutex_(queue_mutex), queue_cv_(queue_cv), buffer_(buffer), out_(out), stop_flag_(stop_flag) {}

void WriterThread::operator()() {
    while (true) {
        std::unique_ptr<ICommand> cmd;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(lock, [&]() { return !tasks_.empty() || stop_flag_; });
            if (stop_flag_ && tasks_.empty()) break;
            if (!tasks_.empty()) {
                cmd = std::move(tasks_.front());
                tasks_.pop();
            }
        }
        if (cmd) cmd->execute(buffer_, out_);
    }
}

ReaderThread::ReaderThread(std::queue<std::unique_ptr<ICommand>>& tasks,
                 std::mutex& queue_mutex,
                 std::condition_variable& queue_cv,
                 Buffer& buffer,
                 std::ostream& out,
                 std::atomic<bool>& stop_flag)
    : tasks_(tasks), queue_mutex_(queue_mutex), queue_cv_(queue_cv), buffer_(buffer), out_(out), stop_flag_(stop_flag) {}

void ReaderThread::operator()() {
    while (true) {
        std::unique_ptr<ICommand> cmd;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(lock, [&]() { return !tasks_.empty() || stop_flag_; });
            if (stop_flag_ && tasks_.empty()) break;
            if (!tasks_.empty()) {
                cmd = std::move(tasks_.front());
                tasks_.pop();
            }
        }
        if (cmd) cmd->execute(buffer_, out_);
    }
} 