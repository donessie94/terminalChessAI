#include "graphic.h"

void GRAPHICS::getHighlightIndex(int index) { highLightIndex = index; }

GRAPHICS::~GRAPHICS() {
    if(renderer) {SDL_DestroyRenderer(renderer);}
    if(window) {SDL_DestroyWindow(window);}
    if(squaresTexture) {SDL_DestroyTexture(squaresTexture);}
    if(whitePawnTexture) {SDL_DestroyTexture(whitePawnTexture);}
    if(blackPawnTexture) {SDL_DestroyTexture(blackPawnTexture);}
    if(whiteRookTexture) {SDL_DestroyTexture(whiteRookTexture);}
    if(blackRookTexture) {SDL_DestroyTexture(blackRookTexture);}
    if(whiteKnightTexture) {SDL_DestroyTexture(whiteKnightTexture);}
    if(blackKnightTexture) {SDL_DestroyTexture(blackKnightTexture);}
    if(whiteBishopTexture) {SDL_DestroyTexture(whiteBishopTexture);}
    if(blackBishopTexture) {SDL_DestroyTexture(blackBishopTexture);}
    if(whiteQueenTexture) {SDL_DestroyTexture(whiteQueenTexture);}
    if(blackQueenTexture) {SDL_DestroyTexture(blackQueenTexture);}
    if(whiteKingTexture) {SDL_DestroyTexture(whiteKingTexture);}
    if(blackKingTexture) {SDL_DestroyTexture(blackKingTexture);}
    IMG_Quit();
    SDL_Quit();
}

bool GRAPHICS::init(const char* windowTitle, int w, int h) {
    highLightIndex = -1;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL_Init Error: %s", SDL_GetError());
        return false;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        SDL_Log("IMG_Init Error: %s", IMG_GetError());
        SDL_Quit();
        return false;
    }

    window = SDL_CreateWindow(
        windowTitle,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        w, h,
        0
    );
    if (!window) {
        SDL_Log("SDL_CreateWindow Error: %s", SDL_GetError());
        IMG_Quit();
        SDL_Quit();
        return false;
    }

    renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!renderer) {
        SDL_Log("SDL_CreateRenderer Error: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return false;
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    //
    SDL_Surface* surf = IMG_Load("assets/greens.png");
    if (!surf) {
        SDL_Log("IMG_Load Error: %s", IMG_GetError());
        return false;
    }
    squaresTexture = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    if (!squaresTexture) {
        SDL_Log("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
        return false;
    }
    SDL_SetTextureBlendMode(squaresTexture, SDL_BLENDMODE_BLEND);

    //
    surf = IMG_Load("assets/board.png");
    if (!surf) {
        SDL_Log("IMG_Load Error: %s", IMG_GetError());
        return false;
    }
    boardTexture = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    if (!boardTexture) {
        SDL_Log("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
        return false;
    }
    SDL_SetTextureBlendMode(boardTexture, SDL_BLENDMODE_BLEND);

    //
    surf = IMG_Load("assets/redStoneAI.png");
    if (!surf) {
        SDL_Log("IMG_Load Error: %s", IMG_GetError());
        return false;
    }
    faceAI = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    if (!faceAI) {
        SDL_Log("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
        return false;
    }
    SDL_SetTextureBlendMode(faceAI, SDL_BLENDMODE_BLEND);

    //
    surf = IMG_Load("assets/face1.png");
    if (!surf) {
        SDL_Log("IMG_Load Error: %s", IMG_GetError());
        return false;
    }
    faceHumanW = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    if (!faceHumanW) {
        SDL_Log("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
        return false;
    }
    SDL_SetTextureBlendMode(faceHumanW, SDL_BLENDMODE_BLEND);

    //
    surf = IMG_Load("assets/redStoneThinking.png");
    if (!surf) {
        SDL_Log("IMG_Load Error: %s", IMG_GetError());
        return false;
    }
    thinkingStrip = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    if (!thinkingStrip) {
        SDL_Log("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
        return false;
    }
    SDL_SetTextureBlendMode(thinkingStrip, SDL_BLENDMODE_BLEND);

    // animation
    // 310 x 390 71 - 30
    // 431
    // 828
    //     418
    constexpr int ANIM_W = 310;
    constexpr int ANIM_H = 390;
    thinkAnimationAI[0] = {71, 30, ANIM_W, ANIM_H};
    thinkAnimationAI[1] = {431, 30, ANIM_W, ANIM_H};
    thinkAnimationAI[2] = {829, 30, ANIM_W, ANIM_H};
    thinkAnimationAI[3] = {71, 418, ANIM_W, ANIM_H};
    thinkAnimationAI[4] = {431, 418, ANIM_W, ANIM_H};
    thinkAnimationAI[5] = {829, 418, ANIM_W, ANIM_H};

    // surf = IMG_Load("assets/wPawn.png");
    // if (!surf) {
    //     SDL_Log("IMG_Load Error: %s", IMG_GetError());
    //     return false;
    // }
    // whitePawnTexture = SDL_CreateTextureFromSurface(renderer, surf);
    // SDL_FreeSurface(surf);
    // if (!whitePawnTexture) {
    //     SDL_Log("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
    //     return false;
    // }
    // SDL_SetTextureBlendMode(whitePawnTexture, SDL_BLENDMODE_BLEND);

    // surf = IMG_Load("assets/bPawn.png");
    // if (!surf) {
    //     SDL_Log("IMG_Load Error: %s", IMG_GetError());
    //     return false;
    // }
    // blackPawnTexture = SDL_CreateTextureFromSurface(renderer, surf);
    // SDL_FreeSurface(surf);
    // if (!blackPawnTexture) {
    //     SDL_Log("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
    //     return false;
    // }
    // SDL_SetTextureBlendMode(blackPawnTexture, SDL_BLENDMODE_BLEND);

    // surf = IMG_Load("assets/wRook.png");
    // if (!surf) {
    //     SDL_Log("IMG_Load Error: %s", IMG_GetError());
    //     return false;
    // }
    // whiteRookTexture = SDL_CreateTextureFromSurface(renderer, surf);
    // SDL_FreeSurface(surf);
    // if (!whiteRookTexture) {
    //     SDL_Log("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
    //     return false;
    // }
    // SDL_SetTextureBlendMode(whiteRookTexture, SDL_BLENDMODE_BLEND);

    // surf = IMG_Load("assets/bRook.png");
    // if (!surf) {
    //     SDL_Log("IMG_Load Error: %s", IMG_GetError());
    //     return false;
    // }
    // blackRookTexture = SDL_CreateTextureFromSurface(renderer, surf);
    // SDL_FreeSurface(surf);
    // if (!blackRookTexture) {
    //     SDL_Log("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
    //     return false;
    // }
    // SDL_SetTextureBlendMode(blackRookTexture, SDL_BLENDMODE_BLEND);

    // surf = IMG_Load("assets/wKnight.png");
    // if (!surf) {
    //     SDL_Log("IMG_Load Error: %s", IMG_GetError());
    //     return false;
    // }
    // whiteKnightTexture = SDL_CreateTextureFromSurface(renderer, surf);
    // SDL_FreeSurface(surf);
    // if (!whiteKnightTexture) {
    //     SDL_Log("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
    //     return false;
    // }
    // SDL_SetTextureBlendMode(whiteKnightTexture, SDL_BLENDMODE_BLEND);

    // surf = IMG_Load("assets/bKnight.png");
    // if (!surf) {
    //     SDL_Log("IMG_Load Error: %s", IMG_GetError());
    //     return false;
    // }
    // blackKnightTexture = SDL_CreateTextureFromSurface(renderer, surf);
    // SDL_FreeSurface(surf);
    // if (!blackKnightTexture) {
    //     SDL_Log("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
    //     return false;
    // }
    // SDL_SetTextureBlendMode(blackKnightTexture, SDL_BLENDMODE_BLEND);

    // surf = IMG_Load("assets/wBishop.png");
    // if (!surf) {
    //     SDL_Log("IMG_Load Error: %s", IMG_GetError());
    //     return false;
    // }
    // whiteBishopTexture = SDL_CreateTextureFromSurface(renderer, surf);
    // SDL_FreeSurface(surf);
    // if (!whiteBishopTexture) {
    //     SDL_Log("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
    //     return false;
    // }
    // SDL_SetTextureBlendMode(whiteBishopTexture, SDL_BLENDMODE_BLEND);

    // surf = IMG_Load("assets/bBishop.png");
    // if (!surf) {
    //     SDL_Log("IMG_Load Error: %s", IMG_GetError());
    //     return false;
    // }
    // blackBishopTexture = SDL_CreateTextureFromSurface(renderer, surf);
    // SDL_FreeSurface(surf);
    // if (!blackBishopTexture) {
    //     SDL_Log("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
    //     return false;
    // }
    // SDL_SetTextureBlendMode(blackBishopTexture, SDL_BLENDMODE_BLEND);

    // surf = IMG_Load("assets/wQueen.png");
    // if (!surf) {
    //     SDL_Log("IMG_Load Error: %s", IMG_GetError());
    //     return false;
    // }
    // whiteQueenTexture = SDL_CreateTextureFromSurface(renderer, surf);
    // SDL_FreeSurface(surf);
    // if (!whiteQueenTexture) {
    //     SDL_Log("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
    //     return false;
    // }
    // SDL_SetTextureBlendMode(whiteQueenTexture, SDL_BLENDMODE_BLEND);

    // surf = IMG_Load("assets/bQueen.png");
    // if (!surf) {
    //     SDL_Log("IMG_Load Error: %s", IMG_GetError());
    //     return false;
    // }
    // blackQueenTexture = SDL_CreateTextureFromSurface(renderer, surf);
    // SDL_FreeSurface(surf);
    // if (!blackQueenTexture) {
    //     SDL_Log("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
    //     return false;
    // }
    // SDL_SetTextureBlendMode(blackQueenTexture, SDL_BLENDMODE_BLEND);

    // surf = IMG_Load("assets/wKing.png");
    // if (!surf) {
    //     SDL_Log("IMG_Load Error: %s", IMG_GetError());
    //     return false;
    // }
    // whiteKingTexture = SDL_CreateTextureFromSurface(renderer, surf);
    // SDL_FreeSurface(surf);
    // if (!whiteKingTexture) {
    //     SDL_Log("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
    //     return false;
    // }
    // SDL_SetTextureBlendMode(whiteKingTexture, SDL_BLENDMODE_BLEND);

    // surf = IMG_Load("assets/bKing.png");
    // if (!surf) {
    //     SDL_Log("IMG_Load Error: %s", IMG_GetError());
    //     return false;
    // }
    // blackKingTexture = SDL_CreateTextureFromSurface(renderer, surf);
    // SDL_FreeSurface(surf);
    // if (!blackKingTexture) {
    //     SDL_Log("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
    //     return false;
    // }
    // SDL_SetTextureBlendMode(blackKingTexture, SDL_BLENDMODE_BLEND);
    int W=300;
    int H=400;
    surf = IMG_Load("assets/set.bmp");
    if (!surf) {
        SDL_Log("IMG_Load Error: %s", IMG_GetError());
        return false;
    }

    //
    SDL_Surface* pieceSurf = SDL_CreateRGBSurfaceWithFormat(0, W, H, 32, surf->format->format);
    if(!pieceSurf) {
        SDL_Log("SDL_CreateRGBSurface Error: %s", SDL_GetError());
        SDL_FreeSurface(surf);
        return false;
    }
    SDL_Rect src = { 0*W, 0*H, W, H };
    SDL_BlitSurface(surf, &src, pieceSurf, nullptr);  // copy region
    blackRookTexture = SDL_CreateTextureFromSurface(renderer, pieceSurf);
    if (!blackRookTexture) {
        SDL_Log("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
        SDL_FreeSurface(surf);
        SDL_FreeSurface(pieceSurf);
        return false;
    }
    SDL_FreeSurface(pieceSurf);
    SDL_SetTextureBlendMode(blackRookTexture, SDL_BLENDMODE_BLEND);

    //
    pieceSurf = SDL_CreateRGBSurfaceWithFormat(0, W, H, 32, surf->format->format);
    if(!pieceSurf) {
        SDL_Log("SDL_CreateRGBSurface Error: %s", SDL_GetError());
        SDL_FreeSurface(surf);
        return false;
    }
    src = { 0*W, 1*H, W, H };
    SDL_BlitSurface(surf, &src, pieceSurf, nullptr);  // copy region
    whiteRookTexture = SDL_CreateTextureFromSurface(renderer, pieceSurf);
    if (!whiteRookTexture) {
        SDL_Log("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
        SDL_FreeSurface(surf);
        SDL_FreeSurface(pieceSurf);
        return false;
    }
    SDL_FreeSurface(pieceSurf);
    SDL_SetTextureBlendMode(whiteRookTexture, SDL_BLENDMODE_BLEND);

    //
    pieceSurf = SDL_CreateRGBSurfaceWithFormat(0, W, H, 32, surf->format->format);
    if(!pieceSurf) {
        SDL_Log("SDL_CreateRGBSurface Error: %s", SDL_GetError());
        SDL_FreeSurface(surf);
        return false;
    }
    src = { 1*W, 0*H, W, H };
    SDL_BlitSurface(surf, &src, pieceSurf, nullptr);  // copy region
    blackBishopTexture = SDL_CreateTextureFromSurface(renderer, pieceSurf);
    if (!blackBishopTexture) {
        SDL_Log("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
        SDL_FreeSurface(surf);
        SDL_FreeSurface(pieceSurf);
        return false;
    }
    SDL_FreeSurface(pieceSurf);
    SDL_SetTextureBlendMode(blackBishopTexture, SDL_BLENDMODE_BLEND);

    //
    pieceSurf = SDL_CreateRGBSurfaceWithFormat(0, W, H, 32, surf->format->format);
    if(!pieceSurf) {
        SDL_Log("SDL_CreateRGBSurface Error: %s", SDL_GetError());
        SDL_FreeSurface(surf);
        return false;
    }
    src = { 1*W, 1*H, W, H };
    SDL_BlitSurface(surf, &src, pieceSurf, nullptr);  // copy region
    whiteBishopTexture = SDL_CreateTextureFromSurface(renderer, pieceSurf);
    if (!whiteBishopTexture) {
        SDL_Log("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
        SDL_FreeSurface(surf);
        SDL_FreeSurface(pieceSurf);
        return false;
    }
    SDL_FreeSurface(pieceSurf);
    SDL_SetTextureBlendMode(whiteBishopTexture, SDL_BLENDMODE_BLEND);

    //
    pieceSurf = SDL_CreateRGBSurfaceWithFormat(0, W, H, 32, surf->format->format);
    if(!pieceSurf) {
        SDL_Log("SDL_CreateRGBSurface Error: %s", SDL_GetError());
        SDL_FreeSurface(surf);
        return false;
    }
    src = { 2*W, 0*H, W, H };
    SDL_BlitSurface(surf, &src, pieceSurf, nullptr);  // copy region
    blackQueenTexture = SDL_CreateTextureFromSurface(renderer, pieceSurf);
    if (!blackQueenTexture) {
        SDL_Log("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
        SDL_FreeSurface(surf);
        SDL_FreeSurface(pieceSurf);
        return false;
    }
    SDL_FreeSurface(pieceSurf);
    SDL_SetTextureBlendMode(blackQueenTexture, SDL_BLENDMODE_BLEND);

    //
    pieceSurf = SDL_CreateRGBSurfaceWithFormat(0, W, H, 32, surf->format->format);
    if(!pieceSurf) {
        SDL_Log("SDL_CreateRGBSurface Error: %s", SDL_GetError());
        SDL_FreeSurface(surf);
        return false;
    }
    src = { 2*W, 1*H, W, H };
    SDL_BlitSurface(surf, &src, pieceSurf, nullptr);  // copy region
    whiteQueenTexture = SDL_CreateTextureFromSurface(renderer, pieceSurf);
    if (!whiteQueenTexture) {
        SDL_Log("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
        SDL_FreeSurface(surf);
        SDL_FreeSurface(pieceSurf);
        return false;
    }
    SDL_FreeSurface(pieceSurf);
    SDL_SetTextureBlendMode(whiteQueenTexture, SDL_BLENDMODE_BLEND);

    //
    pieceSurf = SDL_CreateRGBSurfaceWithFormat(0, W, H, 32, surf->format->format);
    if(!pieceSurf) {
        SDL_Log("SDL_CreateRGBSurface Error: %s", SDL_GetError());
        SDL_FreeSurface(surf);
        return false;
    }
    src = { 3*W, 0*H, W, H };
    SDL_BlitSurface(surf, &src, pieceSurf, nullptr);  // copy region
    blackKingTexture = SDL_CreateTextureFromSurface(renderer, pieceSurf);
    if (!blackKingTexture) {
        SDL_Log("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
        SDL_FreeSurface(surf);
        SDL_FreeSurface(pieceSurf);
        return false;
    }
    SDL_FreeSurface(pieceSurf);
    SDL_SetTextureBlendMode(blackKingTexture, SDL_BLENDMODE_BLEND);

    //
    pieceSurf = SDL_CreateRGBSurfaceWithFormat(0, W, H, 32, surf->format->format);
    if(!pieceSurf) {
        SDL_Log("SDL_CreateRGBSurface Error: %s", SDL_GetError());
        SDL_FreeSurface(surf);
        return false;
    }
    src = { 3*W, 1*H, W, H };
    SDL_BlitSurface(surf, &src, pieceSurf, nullptr);  // copy region
    whiteKingTexture = SDL_CreateTextureFromSurface(renderer, pieceSurf);
    if (!whiteKingTexture) {
        SDL_Log("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
        SDL_FreeSurface(surf);
        SDL_FreeSurface(pieceSurf);
        return false;
    }
    SDL_FreeSurface(pieceSurf);
    SDL_SetTextureBlendMode(whiteKingTexture, SDL_BLENDMODE_BLEND);

    //
    pieceSurf = SDL_CreateRGBSurfaceWithFormat(0, W, H, 32, surf->format->format);
    if(!pieceSurf) {
        SDL_Log("SDL_CreateRGBSurface Error: %s", SDL_GetError());
        SDL_FreeSurface(surf);
        return false;
    }
    src = { 4*W, 0*H, W, H };
    SDL_BlitSurface(surf, &src, pieceSurf, nullptr);  // copy region
    blackKnightTexture = SDL_CreateTextureFromSurface(renderer, pieceSurf);
    if (!blackKnightTexture) {
        SDL_Log("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
        SDL_FreeSurface(surf);
        SDL_FreeSurface(pieceSurf);
        return false;
    }
    SDL_FreeSurface(pieceSurf);
    SDL_SetTextureBlendMode(blackKnightTexture, SDL_BLENDMODE_BLEND);

    //
    pieceSurf = SDL_CreateRGBSurfaceWithFormat(0, W, H, 32, surf->format->format);
    if(!pieceSurf) {
        SDL_Log("SDL_CreateRGBSurface Error: %s", SDL_GetError());
        SDL_FreeSurface(surf);
        return false;
    }
    src = { 4*W, 1*H, W, H };
    SDL_BlitSurface(surf, &src, pieceSurf, nullptr);  // copy region
    whiteKnightTexture = SDL_CreateTextureFromSurface(renderer, pieceSurf);
    if (!whiteKnightTexture) {
        SDL_Log("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
        SDL_FreeSurface(surf);
        SDL_FreeSurface(pieceSurf);
        return false;
    }
    SDL_FreeSurface(pieceSurf);
    SDL_SetTextureBlendMode(whiteKnightTexture, SDL_BLENDMODE_BLEND);

    //
    pieceSurf = SDL_CreateRGBSurfaceWithFormat(0, W, H, 32, surf->format->format);
    if(!pieceSurf) {
        SDL_Log("SDL_CreateRGBSurface Error: %s", SDL_GetError());
        SDL_FreeSurface(surf);
        return false;
    }
    src = { 5*W, 0*H, W, H };
    SDL_BlitSurface(surf, &src, pieceSurf, nullptr);  // copy region
    blackPawnTexture = SDL_CreateTextureFromSurface(renderer, pieceSurf);
    if (!blackPawnTexture) {
        SDL_Log("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
        SDL_FreeSurface(surf);
        SDL_FreeSurface(pieceSurf);
        return false;
    }
    SDL_FreeSurface(pieceSurf);
    SDL_SetTextureBlendMode(blackPawnTexture, SDL_BLENDMODE_BLEND);

    //
    pieceSurf = SDL_CreateRGBSurfaceWithFormat(0, W, H, 32, surf->format->format);
    if(!pieceSurf) {
        SDL_Log("SDL_CreateRGBSurface Error: %s", SDL_GetError());
        SDL_FreeSurface(surf);
        return false;
    }
    src = { 5*W, 1*H, W, H };
    SDL_BlitSurface(surf, &src, pieceSurf, nullptr);  // copy region
    whitePawnTexture = SDL_CreateTextureFromSurface(renderer, pieceSurf);
    if (!whitePawnTexture) {
        SDL_Log("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
        SDL_FreeSurface(surf);
        SDL_FreeSurface(pieceSurf);
        return false;
    }
    SDL_FreeSurface(pieceSurf);
    SDL_SetTextureBlendMode(whitePawnTexture, SDL_BLENDMODE_BLEND);


    // x-coordinate within the texture (pixels from the left)
    // y-coordinate within the texture (pixels from the top)
    // width  of that sub-rectangle (in pixels)
    // height of that sub-rectangle (in pixels)
    // darkSquareRect = {0, 0, 512, 512};
    // lightSquareRect = {512, 0, 512, 512};

    // darkSquareRect = {0, 512, 512, 512};
    // lightSquareRect = {512, 512, 512, 512};

    // lightSquareRect = {512, 512, 512, 512};
    // darkSquareRect = {512, 0, 512, 512};

    lightSquareRect = {0, 0, 200, 200};
    darkSquareRect = {200, 0, 200, 200};

    return true;
}

void GRAPHICS::drawBoard()
{
    if (SDL_RenderCopy(renderer, boardTexture, nullptr, nullptr) != 0) {
        SDL_Log("RenderCopy boardTexture failed: %s", SDL_GetError());
    }
}

void GRAPHICS::drawFaces()
{
    // ---- AI “face” (animated) ----
    // 1) Advance frame if enough time has passed
    Uint32 now = SDL_GetTicks();
    if (now - lastThinkUpdate >= THINK_FRAME_DURATION) {
        // move to next frame, wrap at 6
        thinkFrameIndex = (thinkFrameIndex + 1) % 6;
        lastThinkUpdate = now;
    }

    // 2) pick source rect for current frame
    const SDL_Rect& srcAI = thinkAnimationAI[thinkFrameIndex];

    // 3) destination rect on screen (same as your old faceB)
    SDL_Rect dstAI = { 69, 0, 79, 79 };

    // 4) render that sub-rect of the thinkingStrip
    if (SDL_RenderCopy(renderer, thinkingStrip, &srcAI, &dstAI) != 0) {
        SDL_Log("RenderCopy thinkingStrip failed: %s", SDL_GetError());
    }

    // ---- Human face (static) ----
    SDL_Rect dstW = { 69, 948, 79, 77 };
    if (SDL_RenderCopy(renderer, faceHumanW, nullptr, &dstW) != 0) {
        SDL_Log("RenderCopy faceHumanW failed: %s", SDL_GetError());
    }
}

void GRAPHICS::drawPieces(const CHESS& state) {
    // Define the scale factor for piece size
    const float scale = 1.1f;
    // Base square dimensions
    const int baseW = SQUARE_WIDTH;
    const int baseH = SQUARE_HEIGHT;
    // Compute scaled dimensions
    const int scaledW = int(baseW * scale);
    const int scaledH = int(baseH * scale);
    // Offsets to center the scaled piece in the square
    const int offsetX = (scaledW - baseW) / 2;
    const int offsetY = (scaledH - baseH) / 2;

    // 2) Draw each piece scaled by 1.1x and centered
    for (auto& piece : state.board) {
        if (!piece) continue; // I skip empty squares

        // I convert file/rank to 0..7 indices
        int fileIndex = piece->position.file - 'a';
        int rankIndex0 = piece->position.rank - 1;
        // I invert rank so rank=1 is on the bottom row
        int invertedRank = 7 - rankIndex0;
        // Compute base top-left of the square
        int baseX = BOARD_START_W + fileIndex * SQUARE_WIDTH + fileIndex * LINE_SIZE;
        int baseY = BOARD_START_H + invertedRank * SQUARE_HEIGHT + invertedRank * LINE_SIZE;

        // I compute the destination rectangle for the scaled piece, centered over its square
        SDL_Rect dstRect = {
            baseX - offsetX,
            baseY - offsetY,
            scaledW,
            scaledH
        };

        // I pick the correct texture based on piece type and color
        SDL_Texture* texture = nullptr;
        switch (piece->type) {
            case PIECE_TYPE::PAWN:
                texture = (piece->color == COLOR::WHITE) ? whitePawnTexture : blackPawnTexture;
                break;
            case PIECE_TYPE::ROOK:
                texture = (piece->color == COLOR::WHITE) ? whiteRookTexture : blackRookTexture;
                break;
            case PIECE_TYPE::KNIGHT:
                texture = (piece->color == COLOR::WHITE) ? whiteKnightTexture : blackKnightTexture;
                break;
            case PIECE_TYPE::BISHOP:
                texture = (piece->color == COLOR::WHITE) ? whiteBishopTexture : blackBishopTexture;
                break;
            case PIECE_TYPE::QUEEN:
                texture = (piece->color == COLOR::WHITE) ? whiteQueenTexture : blackQueenTexture;
                break;
            case PIECE_TYPE::KING:
                texture = (piece->color == COLOR::WHITE) ? whiteKingTexture : blackKingTexture;
                break;
        }
        if (!texture) continue; // I guard against missing textures

        // I ensure normal blending and full opacity
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureColorMod(texture, 255, 255, 255);
        SDL_SetTextureAlphaMod(texture, 255);

        // I draw the scaled piece, centered over its square
        if (SDL_RenderCopy(renderer, texture, nullptr, &dstRect) != 0) {
            SDL_Log("RenderCopy piece failed: %s", SDL_GetError());
        }
    }
}

void GRAPHICS::drawPieceHighLight() {
    // If no square is selected, do nothing
    if (highLightIndex < 0 || highLightIndex >= 64) {
        return;
    }
    // Compute fileIndex (0..7) and rankIndex0 (0..7) from selectedSquareIndex
    int fileIndex = highLightIndex % 8;    // 0 = 'a', 7 = 'h'
    int rankIndex0 = highLightIndex / 8;    // 0 = rank 1, 7 = rank 8
    // Invert rank so rank 1 is bottom row
    int invertedRank = 7 - rankIndex0;

    // Compute the top-left pixel of the square in window coordinates
    int squareX = BOARD_START_W + fileIndex * SQUARE_WIDTH + fileIndex * LINE_SIZE;
    int squareY = BOARD_START_H + invertedRank * SQUARE_HEIGHT + invertedRank * LINE_SIZE;

    SDL_Rect highlightRect = {
        squareX,
        squareY,
        SQUARE_WIDTH,
        SQUARE_HEIGHT
    };

    // Draw a green 1px border around highlightRect
    // I set draw color to green
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    // SDL_RenderDrawRect draws a 1px border. If I want thicker, I could draw nested rects.
    if (SDL_RenderDrawRect(renderer, &highlightRect) != 0) {
        SDL_Log("RenderDrawRect highlight failed: %s", SDL_GetError());
    }

    // thicker border
    // 2px-thick border by drawing an inner rect inset by 1
    SDL_Rect innerRect = {
        highlightRect.x + 1,
        highlightRect.y + 1,
        highlightRect.w - 2,
        highlightRect.h - 2
    };
    if (innerRect.w > 0 && innerRect.h > 0) {
        if (SDL_RenderDrawRect(renderer, &innerRect) != 0) {
            SDL_Log("RenderDrawRect inner highlight failed: %s", SDL_GetError());
        }
    }
}

// void GRAPHICS::drawMoveHint(const CHESS& state)
// {
//     // If no square is selected, do nothing
//     if (highLightIndex < 0 || highLightIndex >= 64) {
//         return;
//     }
//     for(auto dir : DIRECTION direction){

//     }
// }

void GRAPHICS::clear(const CHESS &state)
{
    // I enable blending so PNGs with transparency render correctly
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // 1) Clear screen and draw the full board background
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    drawBoard();
    drawFaces();
    drawPieces(state);
    drawPieceHighLight();
    //drawMoveHint(state);

    SDL_RenderPresent(renderer);
}




