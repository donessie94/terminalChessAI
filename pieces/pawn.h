#pragma once
#include "piece.h"

class PAWN : public PIECE {
public:
    PAWN(POSITION pos, COLOR col) : PIECE(PIECE_TYPE::PAWN, col, pos) {}
    virtual std::vector<MOVE> getValidMoves(const POSITION& from, const CHESS& state) const override;
};