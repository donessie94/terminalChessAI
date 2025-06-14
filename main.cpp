#include "graphics/graphic.h"
#include "inputHandler/inputHandler.h"
#include "sounds/effects.h"

constexpr int SCREEN_WIDTH  = 1024; // Width of the window in pixels
constexpr int SCREEN_HEIGHT = 1024; // Height of the window in pixels

int main(int argc, char* argv[])
{
    //audio initialization
    if (SDL_Init(SDL_INIT_AUDIO) != 0) {
        SDL_Log("SDL_Init error: %s", SDL_GetError());
        return 1;
    }
    if (!EFFECTS::initAudio()) {
        SDL_Log("Audio init failed, continuing without sound.");
    }

    // Load your sound files (paths relative to executable working dir or absolute):
    EFFECTS::loadSounds("assets/sounds/move.wav",
                        "assets/sounds/capture.wav",
                        "assets/sounds/errorMove.wav",
                        "assets/sounds/check_alert.wav",
                        "assets/sounds/background.wav");

    EFFECTS::playBackground(-1); // loop forever
    EFFECTS::setBackgroundVolumePercent(15);

    // Build raw move table once at startup
    PIECE::buildRawMoveTable();

    GRAPHICS gfx;
    CHESS chess;
    InputHandler inputHandler;

    if (!gfx.init("RedStone Chess", SCREEN_WIDTH, SCREEN_HEIGHT)) {
        return 1;
    }

    bool running = true;
    int selectedSquare = -1;  // -1 means no piece selected

    while (running) {
        // Poll input events once per frame
        inputHandler.pollInputs(chess, running);

        // Get one-time click index (or -1 if none this frame)
        int clicked = inputHandler.getClickedSquareIndex();
        if (clicked >= 0 && clicked < 64) {
            //SDL_Log("Main: Mouse click on square %d", clicked);

            // CASE A: Click on a friendly piece -> select or deselect immediately
            if (chess.board[clicked] && chess.board[clicked]->color == chess.currentPlayer) {
                if (selectedSquare == clicked) {
                    // Deselect if clicking same square again
                    //SDL_Log("Main: Deselected square %d", clicked);
                    selectedSquare = -1;
                } else {
                    // Select new piece immediately
                    selectedSquare = clicked;
                    //SDL_Log("Main: Selected square %d (piece=%s)",
                    //        selectedSquare,
                    //        chess.board[selectedSquare]->getTypeAsString().c_str());
                    // Optionally: we could log its available moves now, or simply rely on drawMoveHint to show them.
                }
            }
            // CASE B: Click on a different square while a piece is already selected -> attempt move
            else if (selectedSquare >= 0) {
                // Build MOVE from selectedSquare -> clicked
                POSITION fromP(selectedSquare);
                POSITION toP(clicked);
                PIECE_TYPE pt = chess.board[selectedSquare]->type;
                MOVE move(fromP, toP, pt);

                SDL_Log("Main: Attempting move %d -> %d for player %s",
                        move.from.index, move.to.index,
                        (chess.currentPlayer==COLOR::WHITE?"WHITE":"BLACK"));

                bool capture = (chess.board[toP.index] ? true : false);

                gfx.animateMove(chess, fromP.index, toP.index, 200);
                bool ok = chess.movePiece(move);
                if (ok) {
                    SDL_Log("Main: movePiece succeeded");

                    //capture or move sound
                    (capture ? EFFECTS::playCapture() : EFFECTS::playMove());

                    // After movePiece, turn has been advanced and valid moves recomputed internally
                    selectedSquare = -1; // clear selection
                } else {
                    SDL_Log("Main: movePiece rejected the move");
                    EFFECTS::playInvalid();
                    // Keep selection so user can try another destination if desired:
                    selectedSquare = -1; //actually lets clear the selection (it feels better)
                }
            }
            // CASE C: Click on empty square or opponent piece while no piece selected -> do nothing
            else {
                SDL_Log("Main: Click on square %d is not a selectable piece and no piece is selected",
                        clicked);
                selectedSquare = -1; // ensure no selection
            }
        }
        // else: clicked < 0 -> no new click this frame; nothing to do for selection.

        // Tell graphics which square to highlight (if any)
        gfx.getHighlightIndex(selectedSquare);

        // Draw the board & highlights
        gfx.clear(chess);

        // (Optional) cap framerate, e.g. SDL_Delay(16);
    }

    EFFECTS::cleanup();
    return 0;
}