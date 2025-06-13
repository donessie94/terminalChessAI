#pragma once
#include "piece.h"

class QUEEN : public PIECE {
public:
    QUEEN(POSITION pos, COLOR col) : PIECE(PIECE_TYPE::QUEEN, col, pos) {}
    virtual std::vector<MOVE> getValidMoves(const POSITION& from, const CHESS& state) const override;
};