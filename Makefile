CXX      = g++
CXXFLAGS = -std=c++11 -Wall
LDFLAGS  = -lncurses

SRCS   = main.cpp \
         ai/chessAI.cpp \
         board/board.cpp \
         logic/chesslogic.cpp
OBJS   = $(SRCS:.cpp=.o)
TARGET = chess

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)