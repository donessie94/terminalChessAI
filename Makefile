# Compiler to use.
CXX = g++

# Compiler flags:
#   -std=c++11 sets the C++ standard.
#   -Wall enables all common warnings.
CXXFLAGS = -std=c++11 -Wall

# Linker flags:
#   -lncurses tells the linker to link against the ncurses library.
LDFLAGS = -lncurses

# List of source files.
SRCS = main.cpp chesslogic.cpp board.cpp

# Automatically generate list of object files by replacing .cpp with .o.
OBJS = $(SRCS:.cpp=.o)

# The name of the final executable.
TARGET = chess

# The default target: build the executable.
all: $(TARGET)

# Link object files to create the executable.
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

# Rule to compile .cpp files to .o files.
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

# 'clean' target to remove all object files and the executable.
clean:
	rm -f $(OBJS) $(TARGET)