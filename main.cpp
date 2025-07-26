#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <sstream>
#include <atomic>
#include <memory>

#include "buffer.h"
#include "command.h"
#include "command_factory.h"
#include "threads.h"

int main() {
    Buffer buffer;

    std::queue<std::unique_ptr<ICommand>> writer_tasks;
    std::mutex writer_queue_mutex;
    std::condition_variable writer_queue_cv;

    std::queue<std::unique_ptr<ICommand>> reader_tasks;
    std::mutex reader_queue_mutex;
    std::condition_variable reader_queue_cv;

    std::atomic<bool> writer_stop{false};
    std::atomic<bool> reader_stop{false};
    std::atomic<bool> running{true};

    WriterThread writer_thread_obj(writer_tasks, writer_queue_mutex, writer_queue_cv, buffer, std::cout, writer_stop);
    ReaderThread reader_thread_obj(reader_tasks, reader_queue_mutex, reader_queue_cv, buffer, std::cout, reader_stop);
    std::thread writer_thread(std::ref(writer_thread_obj));
    std::thread reader_thread(std::ref(reader_thread_obj));

    auto print_help = []() {
        std::cout << "Available commands:" << std::endl;
        std::cout << "  insert <pos> <val> - Insert value at position" << std::endl;
        std::cout << "  remove <pos>       - Remove element" << std::endl;
        std::cout << "  sort asc|desc      - Sort ascending/descending" << std::endl;
        std::cout << "  reverse            - Reverse buffer" << std::endl;
        std::cout << "  read               - Print buffer" << std::endl;
        std::cout << "  count              - Count even/odd positions" << std::endl;
        std::cout << "  exit|q             - Exit program" << std::endl;
    };

    while (running) {
        std::cout << "> ";
        std::string input;
        std::getline(std::cin, input);
        if (input.empty()) continue;
        std::istringstream iss(input);
        std::string cmd;
        iss >> cmd;
        if (cmd == "exit" || cmd == "q") {
            writer_stop = true;
            writer_queue_cv.notify_one();
            reader_stop = true;
            reader_queue_cv.notify_one();
            running = false;
            break;
        } else if (cmd == "help") {
            print_help();
        } else if (cmd == "insert" || cmd == "remove" || cmd == "sort" || cmd == "reverse") {
            auto command = CommandFactory::create(cmd, iss);
            if (command) {
                std::lock_guard<std::mutex> lock(writer_queue_mutex);
                writer_tasks.push(std::move(command));
                writer_queue_cv.notify_one();
            } else {
                std::cout << "Invalid arguments for command '" << cmd << "'. Type 'help' for usage." << std::endl;
            }
        } else if (cmd == "read" || cmd == "count") {
            auto command = CommandFactory::create(cmd, iss);
            if (command) {
                std::lock_guard<std::mutex> lock(reader_queue_mutex);
                reader_tasks.push(std::move(command));
                reader_queue_cv.notify_one();
            } else {
                std::cout << "Invalid arguments for command '" << cmd << "'. Type 'help' for usage." << std::endl;
            }
        } else {
            std::cout << "Unknown command: " << cmd << std::endl;
            std::cout << "Type 'help' for available commands" << std::endl;
        }
    }

    writer_thread.join();
    reader_thread.join();
    return 0;
}