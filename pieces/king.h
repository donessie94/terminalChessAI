#pragma once
#include "piece.h"
class KING : public PIECE {
public:
    KING(POSITION pos, COLOR col) : PIECE(PIECE_TYPE::KING, col, pos) {}

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