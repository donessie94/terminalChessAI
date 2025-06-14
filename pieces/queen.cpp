#include "queen.h"
#include "../logic/chess.h"

QUEEN::QUEEN(POSITION pos, COLOR col) : PIECE(PIECE_TYPE::QUEEN, col, pos) {}

void QUEEN::computeValidMoves(const POSITION &from, const CHESS &state)
{
}

void QUEEN::computeValidMovesInCheck(const POSITION& from, const CHESS& state, const std::vector<POSITION>& attackers)
{

}