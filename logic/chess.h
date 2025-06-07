#pragma once
#include <vector>
#include "../utils/miscellanous.h"
#include "../pieces/piece.h"

class CHESS {
public:
    std::vector<PIECE> board; // 64 squares, each with a piece or empty
    std::vector<MOVE> validMoves; // Valid moves for the current player
    COLOR currentPlayer; // Current player (WHITE or BLACK)
    int turnCount; // Number of turns played

    CHESS() {
        currentPlayer = COLOR::WHITE; // Start with white player
        turnCount = 0; // Initialize turn count
    }

    void changeTurn() {
        currentPlayer = (currentPlayer == COLOR::WHITE) ? COLOR::BLACK : COLOR::WHITE;
        turnCount++;
        computeNewValidMoves(); // Recompute valid moves for the new player
    }

    void movePiece(MOVE move) {

    }

    void computeNewValidMoves() {
        validMoves.clear(); // Clear valid moves for the new turn

    }
};