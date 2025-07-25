CXX := g++
CXXFLAGS := -std=c++14 -pthread -Wall -Wextra
TARGET := buffer_manager

all: $(TARGET)

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)