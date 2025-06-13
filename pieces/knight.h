#pragma once
#include "piece.h"

class KNIGHT : public PIECE {
public:
    // Constructor for KNIGHT piece
    KNIGHT(POSITION pos, COLOR color);
    virtual std::vector<MOVE> getValidMoves(const POSITION& from, const CHESS& state) const override;
};