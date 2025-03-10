#include "board.h"
#include "chesslogic.h"
#include "chessAI.h"
#include <ncurses.h>
#include <iostream>

int main() {
    BOARD board;
    CHESSLOGIC game;
    ChessAI ai;  // our Chess AI instance

    // Initial drawing of the board.
    board.draw(game.getState());
    board.drawInfo(game.getMoveHistory());
    board.drawUndoButton();
    refresh();

    // Set getch() to non-blocking mode.
    nodelay(stdscr, TRUE);

    MEVENT event;
    int ch;
    bool awaitingSecondClick = false; // false means waiting for first click
    std::pair<short, short> moveIndex;

    while ((ch = getch()) != 'q') {
        // If it's AI's turn, process it immediately.
        if (game.turnToMove() < 0) {
            std::pair<short, short> bestMove = ai.getBestMove(game);
            game.move(bestMove);
            board.draw(game.getState());
            board.drawInfo(game.getMoveHistory());
            board.drawUndoButton();
            refresh();
            // Skip processing further input during AI turn.
            continue;
        }

        // Process mouse events if available.
        if (ch == KEY_MOUSE) {
            if (getmouse(&event) == OK) {
                if (board.clickInside(event.x, event.y)) {
                    short clickedIndex = board.getClickedPieceIndex(game.getState(), event.x, event.y);

                    if (!awaitingSecondClick) {
                        if (!game.playerMovingEnemyPiece(clickedIndex, game.turnToMove())) {
                            moveIndex.first = clickedIndex;
                            board.highlight(game.getState(), moveIndex.first);
                            refresh();
                            awaitingSecondClick = true;
                        }
                    } else {
                        moveIndex.second = clickedIndex;
                        game.move(moveIndex);
                        awaitingSecondClick = false;
                        board.draw(game.getState());
                        board.drawInfo(game.getMoveHistory());
                        board.drawUndoButton();
                        refresh();
                    }
                } else if (board.clickUndoButton(event.x, event.y)) {
                    clear();
                    game.undoMove();
                    board.draw(game.getState());
                    board.drawInfo(game.getMoveHistory());
                    board.drawUndoButton();
                    refresh();
                }
            }
        }
        // Optional: if no key pressed, you might call napms(50) or similar.
        napms(50);
    }

    return 0;
}



