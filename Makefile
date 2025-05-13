# Compiler to use.
CXX = g++

# Compiler flags.
CXXFLAGS = -std=c++11 -Wall

# Linker flags.
LDFLAGS = -lncurses

# List of source files (with folder prefixes).
SRCS = \
    main.cpp \
    ai/chessAI.cpp \
    board/board.cpp \
    logic/chesslogic.cpp \

# Object files: replace .cpp with .o in each path.
OBJS = $(SRCS:.cpp=.o)

# Final executable name.
TARGET = chess

# Default target: build the executable.
all: $(TARGET)

# Link step.
$(TARGET): $(OBJS)
    $(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS)

# Pattern rule: compile any .cpp → corresponding .o in same directory.
# (so ai/chessAI.cpp → ai/chessAI.o, etc.)
%.o: %.cpp
    $(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up.
clean:
    rm -f $(OBJS) $(TARGET)