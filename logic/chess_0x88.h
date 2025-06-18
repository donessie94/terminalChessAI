#pragma once
#include "../utils/miscellanous.h"

class Chess_0x88
{
public:
    Chess_0x88();
    Chess_0x88(const char fen[]);
    ~Chess_0x88() = default;
    void generate_moves();
    void pawn_generation(Piece type, int idx, int rank);
    void castle_generation(Piece type, int idx);
    bool turn;
    int en_passant;
    int castle_right;
    int board[128];
};