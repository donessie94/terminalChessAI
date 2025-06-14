#include "rook.h"
#include "../logic/chess.h"

ROOK::ROOK(POSITION pos, COLOR col) : PIECE(PIECE_TYPE::ROOK, col, pos) {}

void ROOK::computeValidMoves(const POSITION &from, const CHESS &state)
{
}

void ROOK::computeValidMovesInCheck(const POSITION& from, const CHESS& state, const std::vector<POSITION>& attackers)
{

}