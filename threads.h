#ifndef THREADS_H
#define THREADS_H
#include "buffer.h"
#include "command.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <ostream>
#include <atomic>
#include <memory>

class WriterThread {
public:
    WriterThread(std::queue<std::unique_ptr<ICommand>>& tasks,
                 std::mutex& queue_mutex,
                 std::condition_variable& queue_cv,
                 Buffer& buffer,
                 std::ostream& out,
                 std::atomic<bool>& stop_flag);
    void operator()();
private:
    std::queue<std::unique_ptr<ICommand>>& tasks_;
    std::mutex& queue_mutex_;
    std::condition_variable& queue_cv_;
    Buffer& buffer_;
    std::ostream& out_;
    std::atomic<bool>& stop_flag_;
};

class ReaderThread {
public:
    ReaderThread(std::queue<std::unique_ptr<ICommand>>& tasks,
                 std::mutex& queue_mutex,
                 std::condition_variable& queue_cv,
                 Buffer& buffer,
                 std::ostream& out,
                 std::atomic<bool>& stop_flag);
    void operator()();
private:
    std::queue<std::unique_ptr<ICommand>>& tasks_;
    std::mutex& queue_mutex_;
    std::condition_variable& queue_cv_;
    Buffer& buffer_;
    std::ostream& out_;
    std::atomic<bool>& stop_flag_;
};

#endif // THREADS_H 