#pragma once
#include "../pieces/piece.h"
#include "../pieces/pawn.h"
#include "../pieces/rook.h"
#include "../pieces/knight.h"
#include "../pieces/bishop.h"
#include "../pieces/queen.h"
#include "../pieces/king.h"

class CHESS {
public:
    std::vector<std::unique_ptr<PIECE>> board; // Board containing pieces, using smart pointers for memory management
    std::vector<MOVE> validMoves; // Valid moves for the current player
    COLOR currentPlayer; // Current player (WHITE or BLACK)
    int turnCount; // Number of turns played
    POSITION wKingPosition;
    POSITION bKingPosition;

    CHESS();

    void changeTurn();

    void updateKingPosition();

    void movePiece(MOVE move);

    void computeNewValidMoves();
};