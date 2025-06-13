#pragma once
#include "piece.h"
class BISHOP : public PIECE {
public:
    BISHOP(POSITION pos, COLOR col) : PIECE(PIECE_TYPE::BISHOP, col, pos) {}

    virtual std::vector<MOVE> getValidMoves(const POSITION& from, const CHESS& state) const override;
};