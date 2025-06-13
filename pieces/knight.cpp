#include "knight.h"
#include "../logic/chess.h"

KNIGHT::KNIGHT(POSITION pos, COLOR color) : PIECE(PIECE_TYPE::KNIGHT, color, pos) {}

std::vector<MOVE> KNIGHT::getValidMoves(const POSITION& from, const CHESS& state) const {

    return {};
}