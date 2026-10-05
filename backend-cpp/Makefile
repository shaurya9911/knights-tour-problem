CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra
SRCS := server.cpp algorithms.cpp json.cpp response_builder.cpp
TARGET := knights_tour_server

all: $(TARGET)

$(TARGET): $(SRCS) algorithms.h json.h response_builder.h
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) -lws2_32

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
