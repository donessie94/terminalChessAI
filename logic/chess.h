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

    // all possible moves classified in order of importance
    std::vector<MOVE> movesCheck;
    std::vector<MOVE> movesCapture;
    std::vector<MOVE> movesDevelopment;
    std::vector<MOVE> movesQuiet;

    COLOR currentPlayer; // Current player (WHITE or BLACK)
    int turnCount; // Number of turns played
    POSITION wKingPosition;
    POSITION bKingPosition;

    // note: up to 2 pieces at the same time are able to attack our king
    // (think direct attack rook after unblocking an attacking bishop (discovered attack))
    std::vector<POSITION> attackers; // if king of current player is in check here we store the POSITION of those attacking pieces

    bool check;
    bool checkMate;

    CHESS();

    void changeTurn();

    void updateKingPosition();

    void movePiece(MOVE move);

    void computeNewValidMoves();
};