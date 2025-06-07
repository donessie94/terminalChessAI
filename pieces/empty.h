#pragma once
#include "piece.h"

// “empty” piece subclass to represent no piece:
class EMPTY_PIECE : public PIECE {
public:
    EMPTY_PIECE(int pos) : PIECE(PIECE_TYPE::EMPTY, COLOR::WHITE, pos) {}

    std::vector<MOVE> getUpMoves(const POSITION& from) const override {
        return {};
    }
    std::vector<MOVE> getDownMoves(const POSITION& from) const override {
        return {};
    }
    std::vector<MOVE> getLeftMoves(const POSITION& from) const override {
        return {};
    }
    std::vector<MOVE> getRightMoves(const POSITION& from) const override {
        return {};
    }
    std::vector<MOVE> getUpLeftMoves(const POSITION& from) const override {
        return {};
    }
    std::vector<MOVE> getUpRightMoves(const POSITION& from) const override {
        return {};
    }
    std::vector<MOVE> getDownLeftMoves(const POSITION& from) const override {
        return {};
    }
    std::vector<MOVE> getDownRightMoves(const POSITION& from) const override {
        return {};
    }
};