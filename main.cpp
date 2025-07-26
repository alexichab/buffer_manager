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
#include <functional>
#include <map>
#include <atomic>

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

    std::atomic<bool> writer_stop{false};
    std::atomic<bool> reader_stop{false};
    std::atomic<bool> running{true};

    auto safe_print = [&](const std::string& msg) {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << msg << std::endl;
    };

    auto log = [&](const std::string& action) {
        safe_print("[LOG] " + action);
    };

    // Поток для записи
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
                        log("Inserted " + std::to_string(task.value) + 
                            " at position " + std::to_string(task.position));
                    } else {
                        safe_print("Error: Position must be between 0 and " + 
                                 std::to_string(buffer.size()));
                    }
                    break;
                    
                case WriterOperation::REMOVE:
                    if (task.position >= 0 && task.position < static_cast<int>(buffer.size())) {
                        buffer.erase(buffer.begin() + task.position);
                        log("Removed element at position " + std::to_string(task.position));
                    } else {
                        safe_print("Error: Position must be between 0 and " + 
                                 std::to_string(buffer.size() - 1));
                    }
                    break;
                    
                case WriterOperation::SORT_ASC:
                    std::sort(buffer.begin(), buffer.end());
                    log("Sorted buffer in ascending order");
                    break;
                    
                case WriterOperation::SORT_DESC:
                    std::sort(buffer.begin(), buffer.end(), std::greater<int>());
                    log("Sorted buffer in descending order");
                    break;
                    
                case WriterOperation::REVERSE:
                    std::reverse(buffer.begin(), buffer.end());
                    log("Reversed buffer");
                    break;
            }модифицировать
        }
    };

    // Поток для чтения
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

            switch (task.op) {
                case ReaderOperation::READ: {
                    std::vector<int> buffer_copy;
                    {
                        std::lock_guard<std::mutex> buffer_lock(buffer_mutex);
                        buffer_copy = buffer;
                    }
                    
                    std::stringstream ss;
                    ss << "Buffer: [";
                    for (size_t i = 0; i < buffer_copy.size(); ++i) {
                        if (i != 0) ss << ", ";
                        ss << buffer_copy[i];
                    }
                    ss << "]";
                    safe_print(ss.str());
                    break;
                }
                    
                case ReaderOperation::COUNT_EVEN_ODD: {
                    size_t size;
                    {
                        std::lock_guard<std::mutex> buffer_lock(buffer_mutex);
                        size = buffer.size();
                    }
                    int even_count = (size + 1) / 2;
                    int odd_count = size / 2;
                    safe_print("Even positions: " + std::to_string(even_count));
                    safe_print("Odd positions: " + std::to_string(odd_count));
                    break;
                }
            }
        }
    };

    std::thread writer_thread(writer_func);
    std::thread reader_thread(reader_func);

    // Карта для обработки команд
    using CommandHandler = std::function<void(std::istringstream&)>;
    std::map<std::string, CommandHandler> handlers;

    // Регистрация обработчиков команд
    handlers["exit"] = handlers["q"] = [&](std::istringstream&) {
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
        
        running = false;
    };

    handlers["read"] = [&](std::istringstream&) {
        std::lock_guard<std::mutex> lock(reader_queue_mutex);
        reader_tasks.push(ReaderTask(ReaderOperation::READ));
        reader_queue_cv.notify_one();
    };

    handlers["count"] = [&](std::istringstream&) {
        std::lock_guard<std::mutex> lock(reader_queue_mutex);
        reader_tasks.push(ReaderTask(ReaderOperation::COUNT_EVEN_ODD));
        reader_queue_cv.notify_one();
    };

    handlers["insert"] = [&](std::istringstream& iss) {
        int pos, value;
        if (iss >> pos >> value) {
            std::lock_guard<std::mutex> lock(writer_queue_mutex);
            writer_tasks.push(WriterTask(WriterOperation::INSERT, pos, value));
            writer_queue_cv.notify_one();
        } else {
            safe_print("Usage: insert <position> <value>");
        }
    };

    handlers["remove"] = [&](std::istringstream& iss) {
        int pos;
        if (iss >> pos) {
            std::lock_guard<std::mutex> lock(writer_queue_mutex);
            writer_tasks.push(WriterTask(WriterOperation::REMOVE, pos));
            writer_queue_cv.notify_one();
        } else {
            safe_print("Usage: remove <position>");
        }
    };

    handlers["sort"] = [&](std::istringstream& iss) {
        std::string dir;
        if (iss >> dir) {
            WriterOperation op = (dir == "asc") ? WriterOperation::SORT_ASC :
                                (dir == "desc") ? WriterOperation::SORT_DESC :
                                WriterOperation::SORT_ASC;
            
            if (dir != "asc" && dir != "desc") {
                safe_print("Invalid direction. Using 'asc' by default.");
            }
            
            std::lock_guard<std::mutex> lock(writer_queue_mutex);
            writer_tasks.push(WriterTask(op));
            writer_queue_cv.notify_one();
        } else {
            safe_print("Usage: sort <asc|desc>");
        }
    };

    handlers["reverse"] = [&](std::istringstream&) {
        std::lock_guard<std::mutex> lock(writer_queue_mutex);
        writer_tasks.push(WriterTask(WriterOperation::REVERSE));
        writer_queue_cv.notify_one();
    };

    handlers["help"] = [&](std::istringstream&) {
        safe_print("Available commands:");
        safe_print("  insert <pos> <val> - Insert value at position");
        safe_print("  remove <pos>       - Remove element");
        safe_print("  sort asc|desc      - Sort ascending/descending");
        safe_print("  reverse            - Reverse buffer");
        safe_print("  read               - Print buffer");
        safe_print("  count              - Count even/odd positions");
        safe_print("  exit|q             - Exit program");
    };

    while (running) {
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

        auto handler = handlers.find(cmd);
        if (handler != handlers.end()) {
            handler->second(iss);
        } else {
            safe_print("Unknown command: " + cmd);
            safe_print("Type 'help' for available commands");
        }
    }

    writer_thread.join();
    reader_thread.join();
    return 0;
}