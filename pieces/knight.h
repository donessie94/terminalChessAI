#pragma once
#include "piece.h"

class KNIGHT : public PIECE {
public:
    // Constructor for KNIGHT piece
    KNIGHT(COLOR color, POSITION pos)
        : PIECE(PIECE_TYPE::KNIGHT, color, pos) {
    }

    // Override methods to compute valid moves for the knight
    std::vector<MOVE> getUpMoves(const POSITION& from) const override {
        return {}; // Knights do not move straight up
    }

    std::vector<MOVE> getDownMoves(const POSITION& from) const override {
        return {}; // Knights do not move straight down
    }

    std::vector<MOVE> getLeftMoves(const POSITION& from) const override {
        return {}; // Knights do not move straight left
    }

    std::vector<MOVE> getRightMoves(const POSITION& from) const override {
        return {}; // Knights do not move straight right
    }
    
    std::vector<MOVE> getUpLeftMoves(const POSITION& from) const override {
        std::vector<MOVE> validMoves;

        return validMoves;
    }

    std::vector<MOVE> getUpRightMoves(const POSITION& from) const override {
        std::vector<MOVE> validMoves;

        return validMoves;
    }

    std::vector<MOVE> getDownLeftMoves(const POSITION& from) const override {
        std::vector<MOVE> validMoves;

        return validMoves;
    }

    std::vector<MOVE> getDownRightMoves(const POSITION& from) const override {
        std::vector<MOVE> validMoves;

        return validMoves;
    }
};