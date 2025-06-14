#pragma once
#include "piece.h"
class KING : public PIECE {
public:
    KING(POSITION pos, COLOR col);
    virtual void computeValidMoves(const POSITION& from, const CHESS& state) override;
    virtual void computeValidMovesInCheck(const POSITION& from, const CHESS& state, const std::vector<POSITION>& attackers) override;
};