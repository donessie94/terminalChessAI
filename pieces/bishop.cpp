#include "bishop.h"
#include "../logic/chess.h"

BISHOP::BISHOP(POSITION pos, COLOR col) : PIECE(PIECE_TYPE::BISHOP, col, pos) {}

void BISHOP::computeValidMoves(const POSITION &from, const CHESS &state)
{
}

void BISHOP::computeValidMovesInCheck(const POSITION& from, const CHESS& state, const std::vector<POSITION>& attackers)
{

}