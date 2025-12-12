CXX = g++
CXXFLAGS = -Iinclude -Wall -Wextra -std=c++11
SRC = main.cpp scanner.cpp parser.cpp staticSemantics.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = statSem

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
