#pragma once
#include "utils.h"

// Castle rights dec 15 => bin 1111 => all castle rights
//               dec  8 => bin 1000 => qc only
//               dec  1 => bin 0001 => KC only etc...
enum Castle_Right { KC = 1, QC = 2, kc = 4, qc = 8 };

//
enum Color { white, black, all_color };

// Piece enum
// NOTE wwe moved the empty at the end so the pieces allign nicely
enum Piece { P, N, B, R, Q, K, p, n, b, r, q, k, e };

// fck it ill just do this for the arrays indexing of the pieces
enum Piece_Type { Pawn, Knight, Bishop, Rook, Queen, King, Empty };

enum Square {
    a8,  b8,  c8,  d8,  e8,  f8,  g8,  h8,
    a7,  b7,  c7,  d7,  e7,  f7,  g7,  h7,
    a6,  b6,  c6,  d6,  e6,  f6,  g6,  h6,
    a5,  b5,  c5,  d5,  e5,  f5,  g5,  h5,
    a4,  b4,  c4,  d4,  e4,  f4,  g4,  h4,
    a3,  b3,  c3,  d3,  e3,  f3,  g3,  h3,
    a2,  b2,  c2,  d2,  e2,  f2,  g2,  h2,
    a1,  b1,  c1,  d1,  e1,  f1,  g1,  h1, no_sqr
};

extern const char start_position[];         // FEN strings
extern const char tricky_position[];

//
extern const char *square_to_coord[128];    // coordinate strings
extern const char ascii_pieces[];           // ASCII piece symbols
extern const char *unicode_pieces[];        // Unicode piece symbols
extern const int char_to_piece[];

void print_bb(bitboard bb);

void print_mini_board(bitboard player_occ_bb[3], bitboard piece_occ_bb[2][6], bool turn, int castle_right, int en_passant);
void parse_fen_str(const char fen[], bitboard player_occ_bb[3], bitboard piece_occ_bb[2][6], bool &turn, int &castle_right, int &en_passant, int king_pos[]);