#include"encoder.h"

// Define the FEN start string:
const char start_position[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
const char tricky_position[] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
//const char tricky_position[] = "8/3ppP2/8/4P3/4P3/8/3PP3/8 w KQkq - 0 1";

// Board array:
// int board[128] = {
//     // rank 8
//     r, n, b, q, k, b, n, r,   o, o, o, o, o, o, o, o,
//     // rank 7
//     p, p, p, p, p, p, p, p,   o, o, o, o, o, o, o, o,
//     // rank 6
//     e, e, e, e, e, e, e, e,   o, o, o, o, o, o, o, o,
//     // rank 5
//     e, e, e, e, e, e, e, e,   o, o, o, o, o, o, o, o,
//     // rank 4
//     e, e, e, e, e, e, e, e,   o, o, o, o, o, o, o, o,
//     // rank 3
//     e, e, e, e, e, e, e, e,   o, o, o, o, o, o, o, o,
//     // rank 2
//     P, P, P, P, P, P, P, P,   o, o, o, o, o, o, o, o,
//     // rank 1
//     R, N, B, Q, K, B, N, R,   o, o, o, o, o, o, o, o
// };

// Define coordinate lookup:
const char *square_to_coord[128] = {
    "a8","b8","c8","d8","e8","f8","g8","h8","i8","j8","k8","l8","m8","n8","o8","p8",
    "a7","b7","c7","d7","e7","f7","g7","h7","i7","j7","k7","l7","m7","n7","o7","p7",
    "a6","b6","c6","d6","e6","f6","g6","h6","i6","j6","k6","l6","m6","n6","o6","p6",
    "a5","b5","c5","d5","e5","f5","g5","h5","i5","j5","k5","l5","m5","n5","o5","p5",
    "a4","b4","c4","d4","e4","f4","g4","h4","i4","j4","k4","l4","m4","n4","o4","p4",
    "a3","b3","c3","d3","e3","f3","g3","h3","i3","j3","k3","l3","m3","n3","o3","p3",
    "a2","b2","c2","d2","e2","f2","g2","h2","i2","j2","k2","l2","m2","n2","o2","p2",
    "a1","b1","c1","d1","e1","f1","g1","h1","i1","j1","k1","l1","m1","n1","o1","p1"
};

// Define ASCII/Unicode:
const char ascii_pieces[] = {
    '.', 'P', 'N', 'B', 'R', 'Q', 'K',
    'p', 'n', 'b', 'r', 'q', 'k', 'o'
};
const char *unicode_pieces[] = {
    ".", "♟", "♞", "♝", "♜", "♛", "♚",
    "♙", "♘", "♗", "♖", "♕", "♔", "o"
};

// Define char_to_piece:
const int char_to_piece[] = {
    ['P'] = Piece::P, ['p'] = Piece::p,
    ['N'] = Piece::N, ['n'] = Piece::n,
    ['B'] = Piece::B, ['b'] = Piece::b,
    ['R'] = Piece::R, ['r'] = Piece::r,
    ['Q'] = Piece::Q, ['q'] = Piece::q,
    ['K'] = Piece::K, ['k'] = Piece::k,
    ['o'] = Piece::o, ['e'] = Piece::e
};

// Move offsets arrays
const int knight_step[8] = { -31, -14, +18, +33, +31, +14, -18, -33 };
const int bishop_step[4] = { -15, +17, +15, -17 };
const int rook_step[4]   = { -16, +1, +16, -1 };
const int queen_step[8]  = { -16, -15, +1, +17, +16, +15, -1, -17 };
const int king_step[8]   = { -16, -15, +1, +17, +16, +15, -1, -17 };
const int pawn_step[2]          = { -16, +16 };
const int pawn_double_step[2]   = { -32, +32 };
const int pawn_capture_white[2] = { -15, -17 };
const int pawn_capture_black[2] = { +17, +15 };