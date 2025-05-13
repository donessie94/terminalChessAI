#ifndef MOVEINFO_H
#define MOVEINFO_H

#include <vector>
#include <utility>

struct MoveInfo {
    std::vector<short> priorGameState;
    std::pair<short, short> lastMove;
    short lastKingWhitePos;
    short lastKingBlackPos;
    // New fields:
    bool whiteCanCastle;
    bool blackCanCastle;
    short movedPiece;
    short capturedPiece;
};

#endif // MOVEINFO_H