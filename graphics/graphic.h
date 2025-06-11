#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdexcept>
#include "../logic/chess.h"

class GRAPHICS {
public:
    SDL_Window*   window   = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture*  squaresTexture = nullptr;
    SDL_Texture* boardTexture = nullptr;
    SDL_Texture* whitePawnTexture   = nullptr;
    SDL_Texture* blackPawnTexture   = nullptr;
    SDL_Texture* whiteRookTexture   = nullptr;
    SDL_Texture* blackRookTexture   = nullptr;
    SDL_Texture* whiteKnightTexture = nullptr;
    SDL_Texture* blackKnightTexture = nullptr;
    SDL_Texture* whiteBishopTexture = nullptr;
    SDL_Texture* blackBishopTexture = nullptr;
    SDL_Texture* whiteQueenTexture  = nullptr;
    SDL_Texture* blackQueenTexture  = nullptr;
    SDL_Texture* whiteKingTexture   = nullptr;
    SDL_Texture* blackKingTexture   = nullptr;
    SDL_Texture* faceAI = nullptr;
    SDL_Texture* faceHumanW = nullptr;
    SDL_Texture* faceHumanB = nullptr;

    SDL_Rect lightSquareRect;
    SDL_Rect darkSquareRect;

    GRAPHICS() = default;
    ~GRAPHICS();

    // Return true on success, false on any init error:
    bool init(const char* windowTitle, int w, int h);

    // Clears & presents the backbuffer
    void clear(const CHESS& game);
};