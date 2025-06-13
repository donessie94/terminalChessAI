#pragma once
#include "piece.h"
class KING : public PIECE {
public:
    KING(POSITION pos, COLOR col) : PIECE(PIECE_TYPE::KING, col, pos) {}
    virtual std::vector<MOVE> getValidMoves(const POSITION& from, const CHESS& state) const override;
};