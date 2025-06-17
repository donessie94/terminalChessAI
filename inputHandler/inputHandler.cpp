// #include "inputHandler.h"

// InputHandler::InputHandler() { squareIndexClicked = -1; }

// bool InputHandler::handleEvent(const SDL_Event &ev, CHESS &state)
// {
//     if (ev.type == SDL_QUIT) {
//         return false;
//     }
//     if (ev.type == SDL_MOUSEBUTTONDOWN) {
//         int x = ev.button.x;
//         int y = ev.button.y;
//         // map (x,y) to squareIndex as before
//         int squareIndex = mapClickToSquare(x, y);
//         if (squareIndex >= 0)
//             squareIndexClicked = squareIndex;
//         else
//             squareIndexClicked = -1;
//     }

//     return true; // continue running
// }

// void InputHandler::pollInputs(CHESS &state, bool &running)
// {
//     SDL_Event e;
//     while (SDL_PollEvent(&e)) {
//         switch (e.type) {
//             case SDL_QUIT:
//                 running = false;
//                 break;

//             case SDL_MOUSEBUTTONUP:
//                 if (e.button.button == SDL_BUTTON_LEFT) {
//                     int mouseX = e.button.x;
//                     int mouseY = e.button.y;
//                     int idx = mapClickToSquare(mouseX, mouseY);
//                     if (idx >= 0 && idx < 64) {
//                         // Only set pendingClick if none is pending
//                         if (pendingClick < 0) {
//                             pendingClick = idx;
//                             SDL_Log("InputHandler: registered click on square %d", idx);
//                         }
//                     }
//                 }
//                 break;

//             // ... handle other events if needed ...
//             default:
//                 break;
//         }
//     }
// }

// int InputHandler::getClickedSquareIndex()
// {
//     int result = pendingClick;
//     pendingClick = -1;  // consume it so next call returns -1 until next real click
//     return result;
// }

// int InputHandler::mapClickToSquare(int mouseX, int mouseY)
// {
//     int relX = mouseX - BOARD_START_W;
//     int relY = mouseY - BOARD_START_H;
//     int cellFullW = SQUARE_WIDTH + LINE_SIZE;
//     int cellFullH = SQUARE_HEIGHT + LINE_SIZE;
//     int boardW = 8 * cellFullW - LINE_SIZE;
//     int boardH = 8 * cellFullH - LINE_SIZE;
//     if (relX < 0 || relX >= boardW || relY < 0 || relY >= boardH)
//         return -1; //-1 means the click was not on the board basically
//     int fileIndex = relX / cellFullW;
//     int rowFromTop = relY / cellFullH;
//     int rankIndex0 = 7 - rowFromTop;
//     return rankIndex0 * 8 + fileIndex;
//}


