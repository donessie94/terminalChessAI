#pragma once
#include <cstdio>

// Castle rights dec 15 => bin 1111 => all castle rights
//               dec  8 => bin 1000 => qc only
//               dec  1 => bin 0001 => KC only etc...
enum Castle_Right { KC = 1, QC = 2, kc = 4, qc = 8 };

//
enum Turn { white, black };

// Piece enum
enum Piece { e, P, N, B, R, Q, K, p, n, b, r, q, k, o };

// Move offsets
extern const int pawn_step[2];
extern const int pawn_double_step[2];
extern const int pawn_capture_white[2];
extern const int pawn_capture_black[2];
extern const int knight_step[8];
extern const int bishop_step[4];
extern const int rook_step[4];
extern const int queen_step[8];
extern const int king_step[8];

// Enum mapping for 0x88 indices of squares a8..h1
enum Square {
    a8 = 0,  b8,  c8,  d8,  e8,  f8,  g8,  h8,
    a7 = 16, b7,  c7,  d7,  e7,  f7,  g7,  h7,
    a6 = 32, b6,  c6,  d6,  e6,  f6,  g6,  h6,
    a5 = 48, b5,  c5,  d5,  e5,  f5,  g5,  h5,
    a4 = 64, b4,  c4,  d4,  e4,  f4,  g4,  h4,
    a3 = 80, b3,  c3,  d3,  e3,  f3,  g3,  h3,
    a2 = 96, b2,  c2,  d2,  e2,  f2,  g2,  h2,
    a1 = 112,b1,  c1,  d1,  e1,  f1,  g1,  h1, no_sqr = 120
};

// Declare externs for globals; definitions go in exactly one .cpp
extern const char start_position[];         // FEN string
extern const char tricky_position[];
// extern int board[128];                   // 0x88 board
extern const char *square_to_coord[128];    // coordinate strings
extern const char ascii_pieces[];           // ASCII piece symbols
extern const char *unicode_pieces[];        // Unicode piece symbols
extern const int char_to_piece[];           // char→Piece mapping

// Function declarations
void print_mini_board(const int board[], bool turn, int castle_right, int en_passant);
void zero_board(int board[]);
void parse_fen_str(const char fen[], int board[], bool &turn, int &castle_right, int &en_passant);
void print_attack_map(const int board[], const bool turn);
bool is_square_attacked(const int *board, const bool turn, const int idx);