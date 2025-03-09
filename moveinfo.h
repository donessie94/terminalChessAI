#ifndef MOVEINFO_H
#define MOVEINFO_H

#include <vector>
#include <utility>

// This structure holds all the information about a move for undo/history purposes.
struct MoveInfo {
    std::vector<short> priorGameState;
    std::pair<short, short> lastMove;
    short lastKingWhitePos;
    short lastKingBlackPos;
    short movedPiece;      // The piece code that moved.
    short capturedPiece;   // The piece code that was captured (or 0 if none).
};

#endif // MOVEINFO_H