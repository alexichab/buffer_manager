CXX = g++
CXXFLAGS = -std=c++14 -pthread -Wall -Wextra 

SRCS = main.cpp buffer.cpp command.cpp command_factory.cpp threads.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = buffer_manager

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)