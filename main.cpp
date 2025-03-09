#include "board.h"
#include "chesslogic.h"
#include <ncurses.h>

int main() {
    BOARD board;
    CHESSLOGIC game;

    // Initial drawing of the board.
    board.draw(game.getState());
    board.drawInfo(game.getMoveHistory());
    refresh();

    MEVENT event;
    int ch;
    bool awaitingSecondClick = false; // false means we're waiting for the first click
    short firstClickIndex = -1;
    //pair containing the first index (move from) to last index (move to)
    std::pair<short, short> moveIndex;

    while ((ch = getch()) != 'q') {
        if (ch == KEY_MOUSE) {
            if (getmouse(&event) == OK) {
                // Check if click is inside the board.
                if (board.clickInside(event.x, event.y)) {
                    // Get the board index from the click coordinates.
                    short clickedIndex = board.getClickedPieceIndex(game.getState(), event.x, event.y);

                    if (!awaitingSecondClick) {
                        // First click: Check if this click is valid.
                        if (!game.playerMovingEnemyPiece(clickedIndex, game.turnToMove())) {
                            moveIndex.first = clickedIndex;
                            board.highlight(game.getState(), moveIndex.first);
                            refresh();
                            awaitingSecondClick = true; // Now wait for second click.
                        }
                    } else {
                        // Second click: Use it as the final index.
                        moveIndex.second = clickedIndex;
                        // Process the move (for example, moving a knight):
                        game.move(moveIndex);
                        awaitingSecondClick = false;  // Reset for next move.
                        // Redraw board to show the new state.
                        board.draw(game.getState());
                        board.drawInfo(game.getMoveHistory());
                        refresh();
                    }
                }
            }
        }
    }

    return 0;
}



