#pragma once
#include "piece.h"

class ROOK : public PIECE {
public:
    ROOK(POSITION pos, COLOR col) : PIECE(PIECE_TYPE::ROOK, col, pos) {}
    virtual std::vector<MOVE> getValidMoves(const POSITION& from, const CHESS& state) const override;
};