#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>

enum class WriterOperation {
    INSERT,
    REMOVE,
    SORT_ASC,
    SORT_DESC,
    REVERSE
};

struct WriterTask {
    WriterOperation op;
    int position;
    int value;

    WriterTask(WriterOperation o, int p = -1, int v = 0) 
        : op(o), position(p), value(v) {}
};

enum class ReaderOperation {
    READ,
    COUNT_EVEN_ODD
};

struct ReaderTask {
    ReaderOperation op;
    explicit ReaderTask(ReaderOperation o) : op(o) {}
};

int main() {
    std::vector<int> buffer;
    std::mutex buffer_mutex;

    std::queue<WriterTask> writer_tasks;
    std::mutex writer_queue_mutex;
    std::condition_variable writer_queue_cv;

    std::queue<ReaderTask> reader_tasks;
    std::mutex reader_queue_mutex;
    std::condition_variable reader_queue_cv;

    std::mutex cout_mutex;

    bool writer_stop = false;
    bool reader_stop = false;

    auto writer_func = [&]() {
        while (true) {
            std::unique_lock<std::mutex> lock(writer_queue_mutex);
            writer_queue_cv.wait(lock, [&](){ 
                return !writer_tasks.empty() || writer_stop; 
            });
            
            if (writer_stop) break;
            
            WriterTask task = writer_tasks.front();
            writer_tasks.pop();
            lock.unlock();

            std::lock_guard<std::mutex> buffer_lock(buffer_mutex);
            
            switch (task.op) {
                case WriterOperation::INSERT:
                    if (task.position >= 0 && task.position <= static_cast<int>(buffer.size())) {
                        buffer.insert(buffer.begin() + task.position, task.value);
                    } else {
                        std::lock_guard<std::mutex> cout_lock(cout_mutex);
                        std::cout << "Error: Invalid insertion position " << task.position << std::endl;
                    }
                    break;
                    
                case WriterOperation::REMOVE:
                    if (task.position >= 0 && task.position < static_cast<int>(buffer.size())) {
                        buffer.erase(buffer.begin() + task.position);
                    } else {
                        std::lock_guard<std::mutex> cout_lock(cout_mutex);
                        std::cout << "Error: Invalid removal position " << task.position << std::endl;
                    }
                    break;
                    
                case WriterOperation::SORT_ASC:
                    std::sort(buffer.begin(), buffer.end());
                    break;
                    
                case WriterOperation::SORT_DESC:
                    std::sort(buffer.begin(), buffer.end(), std::greater<int>());
                    break;
                    
                case WriterOperation::REVERSE:
                    std::reverse(buffer.begin(), buffer.end());
                    break;
            }
        }
    };

    auto reader_func = [&]() {
        while (true) {
            std::unique_lock<std::mutex> lock(reader_queue_mutex);
            reader_queue_cv.wait(lock, [&](){ 
                return !reader_tasks.empty() || reader_stop; 
            });
            
            if (reader_stop) break;
            
            ReaderTask task = reader_tasks.front();
            reader_tasks.pop();
            lock.unlock();


            std::lock_guard<std::mutex> buffer_lock(buffer_mutex);
            
            switch (task.op) {
                case ReaderOperation::READ: {
                    std::lock_guard<std::mutex> cout_lock(cout_mutex);
                    std::cout << "Buffer: [";
                    for (size_t i = 0; i < buffer.size(); ++i) {
                        std::cout << buffer[i];
                        if (i < buffer.size() - 1) std::cout << ", ";
                    }
                    std::cout << "]" << std::endl;
                    break;
                }
                    
                case ReaderOperation::COUNT_EVEN_ODD: {
                    int even_count = 0;
                    int odd_count = 0;
                    for (size_t i = 0; i < buffer.size(); ++i) {
                        (i % 2 == 0) ? even_count++ : odd_count++;
                    }
                    std::lock_guard<std::mutex> cout_lock(cout_mutex);
                    std::cout << "Even positions: " << even_count << std::endl;
                    std::cout << "Odd positions: " << odd_count << std::endl;
                    break;
                }
            }
        }
    };

    std::thread writer_thread(writer_func);
    std::thread reader_thread(reader_func);

    while (true) {
        std::string input;
        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "> ";
        }
        std::getline(std::cin, input);
        if (input.empty()) continue;

        std::istringstream iss(input);
        std::string cmd;
        iss >> cmd;

        if (cmd == "exit") {
            {
                std::lock_guard<std::mutex> lock(writer_queue_mutex);
                writer_stop = true;
            }
            writer_queue_cv.notify_one();
            
            {
                std::lock_guard<std::mutex> lock(reader_queue_mutex);
                reader_stop = true;
            }
            reader_queue_cv.notify_one();
            break;
        }
        else if (cmd == "read") {
            std::lock_guard<std::mutex> lock(reader_queue_mutex);
            reader_tasks.push(ReaderTask(ReaderOperation::READ));
            reader_queue_cv.notify_one();
        }
        else if (cmd == "count") {
            std::lock_guard<std::mutex> lock(reader_queue_mutex);
            reader_tasks.push(ReaderTask(ReaderOperation::COUNT_EVEN_ODD));
            reader_queue_cv.notify_one();
        }
        else if (cmd == "insert") {
            int pos, value;
            if (iss >> pos >> value) {
                std::lock_guard<std::mutex> lock(writer_queue_mutex);
                writer_tasks.push(WriterTask(WriterOperation::INSERT, pos, value));
                writer_queue_cv.notify_one();
            } else {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cout << "Usage: insert <position> <value>" << std::endl;
            }
        }
        else if (cmd == "remove") {
            int pos;
            if (iss >> pos) {
                std::lock_guard<std::mutex> lock(writer_queue_mutex);
                writer_tasks.push(WriterTask(WriterOperation::REMOVE, pos));
                writer_queue_cv.notify_one();
            } else {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cout << "Usage: remove <position>" << std::endl;
            }
        }
        else if (cmd == "sort") {
            std::string dir;
            if (iss >> dir) {
                WriterOperation op = (dir == "asc") ? WriterOperation::SORT_ASC :
                                    (dir == "desc") ? WriterOperation::SORT_DESC :
                                    WriterOperation::SORT_ASC; // default
                if (dir != "asc" && dir != "desc") {
                    std::lock_guard<std::mutex> lock(cout_mutex);
                    std::cout << "Invalid direction. Using 'asc' by default." << std::endl;
                }
                std::lock_guard<std::mutex> lock(writer_queue_mutex);
                writer_tasks.push(WriterTask(op));

                writer_queue_cv.notify_one();
            } else {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cout << "Usage: sort <asc|desc>" << std::endl;
            }
        }
        else if (cmd == "reverse") {
            std::lock_guard<std::mutex> lock(writer_queue_mutex);
            writer_tasks.push(WriterTask(WriterOperation::REVERSE));
            writer_queue_cv.notify_one();
        }
        else {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "Unknown command: " << cmd << std::endl;
        }
    }

    writer_thread.join();
    reader_thread.join();
    return 0;
}

