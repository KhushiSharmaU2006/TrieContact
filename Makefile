# TrieContact Makefile
# Usage:
#   make        → build the project
#   make run    → build and run
#   make clean  → remove binary

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET = triecontact
SRC = main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)
	@echo "Build successful. Run with: ./$(TARGET)"

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)
