#pragma once
#include <SDL2/SDL.h>
#include "../logic/chess.h"

class InputHandler {
public:
    InputHandler();

    // Returns false if quit was requested
    bool handleEvent(const SDL_Event& ev, CHESS& state);

    void pollInputs(CHESS& state, bool& running);

    // Return the pending click once, then clear it.
    int getClickedSquareIndex();

private:
    int mapClickToSquare(int mouseX, int mouseY);
    int squareIndexClicked;
    int pendingClick = -1;
};