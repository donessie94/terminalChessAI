// #pragma once
// #include <SDL2/SDL.h>
// #include <SDL2/SDL_image.h>
// #include <stdexcept>
// #include "../logic/chess.h"
// #include <cmath> // for std::sqrt

// class GRAPHICS {
// public:
//     SDL_Window*   window   = nullptr;
//     SDL_Renderer* renderer = nullptr;
//     SDL_Texture*  squaresTexture = nullptr;
//     SDL_Texture* boardTexture = nullptr;
//     SDL_Texture* whitePawnTexture   = nullptr;
//     SDL_Texture* blackPawnTexture   = nullptr;
//     SDL_Texture* whiteRookTexture   = nullptr;
//     SDL_Texture* blackRookTexture   = nullptr;
//     SDL_Texture* whiteKnightTexture = nullptr;
//     SDL_Texture* blackKnightTexture = nullptr;
//     SDL_Texture* whiteBishopTexture = nullptr;
//     SDL_Texture* blackBishopTexture = nullptr;
//     SDL_Texture* whiteQueenTexture  = nullptr;
//     SDL_Texture* blackQueenTexture  = nullptr;
//     SDL_Texture* whiteKingTexture   = nullptr;
//     SDL_Texture* blackKingTexture   = nullptr;
//     SDL_Texture* faceAI = nullptr;
//     SDL_Texture* faceHumanW = nullptr;
//     SDL_Texture* faceHumanB = nullptr;
//     SDL_Texture* thinkingStrip = nullptr;


//     SDL_Rect lightSquareRect;
//     SDL_Rect darkSquareRect;
//     SDL_Rect thinkAnimationAI[6];

//     // animation state
//     int     thinkFrameIndex   = 0;
//     Uint32  lastThinkUpdate   = 0;
//     static constexpr Uint32 THINK_FRAME_DURATION = 1800; // ms per frame

//     int highLightIndex;
//     bool moveFlag;

//     GRAPHICS() = default;
//     ~GRAPHICS();

//     void getHighlightIndex(int index);
//     void drawBoard();
//     void drawFaces();
//     void drawPieces(const CHESS& state);
//     void drawPieceHighLight();
//     void drawMoveHint(const CHESS& state);
//     void drawFilledCircle(int cx, int cy, int radius);
//     void drawCircleOutline(int cx, int cy, int radius, int thickness);

//     // Animate a move from 'fromIdx' to 'toIdx' for the given state:
//     //   - state: current board state before move
//     //   - fromIdx, toIdx: 0..63 indices of source/destination squares
//     //   - durationMs: total duration in milliseconds for the animation
//     void animateMove(const CHESS& state, int fromIdx, int toIdx, int durationMs = 300);

//     // Return true on success, false on any init error:
//     bool init(const char* windowTitle, int w, int h);
//     void clear(const CHESS& state);
// };