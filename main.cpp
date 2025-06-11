#include "graphics/graphic.h"
#include "inputHandler/inputHandler.h"

constexpr int SCREEN_WIDTH  = 1024; // Width of the window in pixels
constexpr int SCREEN_HEIGHT = 1024; // Height of the window in pixels

int main(int argc, char* argv[])
{
    GRAPHICS gfx;
    CHESS chess;
    InputHandler inputHandler;

    // 1) initialize before using
    if (!gfx.init("RedStone Chess", SCREEN_WIDTH, SCREEN_HEIGHT)) {
        return 1;
    }

    bool running = true;
    while (running) {
        inputHandler.pollInputs(chess, running);
        gfx.clear(chess);
    }

    return 0;
}