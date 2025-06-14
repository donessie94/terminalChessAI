#pragma once
#include "piece.h"

// “empty” piece subclass to represent no piece:
class EMPTY_PIECE : public PIECE {
public:
    EMPTY_PIECE(POSITION pos) : PIECE(PIECE_TYPE::EMPTY, COLOR::WHITE, pos) {}
    virtual void computeValidMoves(const POSITION& from, const CHESS& state) override { }
    virtual void computeValidMovesInCheck(const POSITION& from, const CHESS& state, const std::vector<POSITION>& attackers) override { }
};