CXX      = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
TARGET   = email_priority
SRC      = email_priority.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET) input.txt

clean:
	rm -f $(TARGET)

.PHONY: all run clean