#include "king.h"
#include "../logic/chess.h"

KING::KING(POSITION pos, COLOR col) : PIECE(PIECE_TYPE::KING, col, pos) {}

void KING::computeValidMoves(const POSITION &from, const CHESS &state)
{
}

void KING::computeValidMovesInCheck(const POSITION& from, const CHESS& state, const std::vector<POSITION>& attackers)
{

}