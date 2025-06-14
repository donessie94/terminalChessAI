#pragma once
#include "piece.h"

class KNIGHT : public PIECE {
public:
    // Constructor for KNIGHT piece
    KNIGHT(POSITION pos, COLOR color);
    virtual void computeValidMoves(const POSITION& from, const CHESS& state) override;
    virtual void computeValidMovesInCheck(const POSITION& from, const CHESS& state, const std::vector<POSITION>& attackers) override;
};