#pragma once
#include "piece.h"

// “empty” piece subclass to represent no piece:
class EMPTY_PIECE : public PIECE {
public:
    EMPTY_PIECE(POSITION pos) : PIECE(PIECE_TYPE::EMPTY, COLOR::WHITE, pos) {}
    virtual std::vector<MOVE> getValidMoves(const POSITION& from, const CHESS& state) const override { return {}; }
};