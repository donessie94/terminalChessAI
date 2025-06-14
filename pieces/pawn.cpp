#include "pawn.h"
#include "../logic/chess.h"

PAWN::PAWN(POSITION pos, COLOR col) : PIECE(PIECE_TYPE::PAWN, col, pos) {}

void PAWN::computeValidMoves(const POSITION &from, const CHESS &state)
{
}

void PAWN::computeValidMovesInCheck(const POSITION& from, const CHESS& state, const std::vector<POSITION>& attackers)
{

}