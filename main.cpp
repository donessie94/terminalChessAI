#include "board.h"
#include "chesslogic.h"
#include "chessAI.h"
#include <ncurses.h>
#include <iostream>
#include <sstream>
#include <utility>

int main() {
    // Initialize ncurses.
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    start_color();

    // Define window dimensions.
    int boardHeight = 150, boardWidth = 150;
    int debugHeight = 100, debugWidth = 100;

    // Create separate windows for the board and debug info.
    WINDOW* boardWin = newwin(boardHeight, boardWidth, 0, 0);
    WINDOW* debugWin = newwin(debugHeight, debugWidth, 0, boardWidth);

    // Create game objects.
    BOARD board;
    CHESSLOGIC game;
    ChessAI ai;

    // Set non-blocking input.
    nodelay(stdscr, TRUE);

    MEVENT event;
    int ch;
    bool awaitingSecondClick = false;
    std::pair<short, short> moveIndex;
    short highlightedSquare = -1;

    while ((ch = getch()) != 'q') {
        // Always redraw the board.
        board.draw(game.getState(), boardWin);
        board.drawInfo(game.getMoveHistory(), boardWin);
        board.drawUndoButton(boardWin);
        // Reapply the highlight if a square is selected.
        if (highlightedSquare != -1) {
            board.highlight(game.getState(), highlightedSquare, boardWin);
        }
        wrefresh(boardWin);

        // If it's AI's turn (Black is AI-controlled).
        if (game.turnToMove() < 0) {
            // Let the AI compute its best move.
            std::pair<short, short> bestMove = ai.getBestMove(game);

            // Update the debug window.
            wclear(debugWin);
            std::ostringstream debugStream;
            debugStream << "AI Move: " << bestMove.first << " -> " << bestMove.second << "\n";
            debugStream << "Root Evaluation: " << ai.getRootEvaluation() << "\n";
            debugStream << "Valid Moves at Root: " << game.allValidMoves.size() << "\n";

            // Loop through each valid move and convert it to standard notation.
            for (const auto &move : game.allValidMoves) {
                // Assuming you have a function to convert an index to chess notation, for example:
                // std::string indexToNotation(short index);
                std::string sourceNotation = board.indexToNotation(move.first);
                std::string destNotation = board.indexToNotation(move.second);
                debugStream << sourceNotation << "-" << destNotation << "\n";
            }

            mvwprintw(debugWin, 1, 1, "%s", debugStream.str().c_str());
            wrefresh(debugWin);
            napms(1000); // Pause briefly.

            game.move(bestMove);
            highlightedSquare = -1; // Clear any highlight since board has changed.
            board.draw(game.getState(), boardWin);
            board.drawInfo(game.getMoveHistory(), boardWin);
            board.drawUndoButton(boardWin);
            wrefresh(boardWin);

            napms(1000); // Pause briefly.
            continue;   // Skip further input processing during AI turn.
        }

        // Process human input.
        if (ch == KEY_MOUSE) {
            if (getmouse(&event) == OK) {
                if (board.clickInside(event.x, event.y)) {
                    short clickedIndex = board.getClickedPieceIndex(game.getState(), event.x, event.y);
                    if (!awaitingSecondClick) {
                        if (!game.playerMovingEnemyPiece(clickedIndex, game.turnToMove())) {
                            moveIndex.first = clickedIndex;
                            highlightedSquare = clickedIndex; // Save highlighted square.
                            awaitingSecondClick = true;
                        }
                    } else {
                        moveIndex.second = clickedIndex;
                        game.move(moveIndex);
                        awaitingSecondClick = false;
                        highlightedSquare = -1; // Clear highlight after move.
                        board.draw(game.getState(), boardWin);
                        board.drawInfo(game.getMoveHistory(), boardWin);
                        board.drawUndoButton(boardWin);
                        wrefresh(boardWin);
                    }
                } else if (board.clickUndoButton(event.x, event.y)) {
                    game.undoMove();
                    highlightedSquare = -1; // Clear highlight.
                    board.draw(game.getState(), boardWin);
                    board.drawInfo(game.getMoveHistory(), boardWin);
                    board.drawUndoButton(boardWin);
                    wrefresh(boardWin);
                }
            }
        }
        napms(50); // Delay to reduce CPU usage.
    }

    // Clean up.
    delwin(boardWin);
    delwin(debugWin);
    endwin();
    return 0;
}



