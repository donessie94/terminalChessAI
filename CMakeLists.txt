cmake_minimum_required(VERSION 3.15)
project(terminalChessAI)

set(CMAKE_CXX_STANDARD 11)
set(SOURCES
    main.cpp
    ai/chessAI.cpp
    board/board.cpp
    logic/chesslogic.cpp
)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_PATH} ai board logic pieces utils)
add_executable(chess ${SOURCES})
target_link_libraries(chess ${CURSES_LIBRARIES})