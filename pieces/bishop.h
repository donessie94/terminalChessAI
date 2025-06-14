#pragma once
#include "piece.h"
class BISHOP : public PIECE {
public:
    BISHOP(POSITION pos, COLOR col);

    virtual void computeValidMoves(const POSITION& from, const CHESS& state) override;
    virtual void computeValidMovesInCheck(const POSITION& from, const CHESS& state, const std::vector<POSITION>& attackers) override;
};