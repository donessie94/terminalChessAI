#include"encoder.h"

namespace RedStone{

namespace Encoder{

// Define ASCII/Unicode:
const char ascii_pieces[] = {
    'P', 'N', 'B', 'R', 'Q', 'K',
    'p', 'n', 'b', 'r', 'q', 'k', '.'
};
const char *unicode_pieces[] = {
    "♟", "♞", "♝", "♜", "♛", "♚",
    "♙", "♘", "♗", "♖", "♕", "♔", ".",
};

// Define coordinate lookup:
const char *square_to_coord[65] = {
    "a8","b8","c8","d8","e8","f8","g8","h8",
    "a7","b7","c7","d7","e7","f7","g7","h7",
    "a6","b6","c6","d6","e6","f6","g6","h6",
    "a5","b5","c5","d5","e5","f5","g5","h5",
    "a4","b4","c4","d4","e4","f4","g4","h4",
    "a3","b3","c3","d3","e3","f3","g3","h3",
    "a2","b2","c2","d2","e2","f2","g2","h2",
    "a1","b1","c1","d1","e1","f1","g1","h1","no_sqr"
};


// Define char_to_piece:
const int char_to_piece[] = {
    ['P'] = Piece::P, ['p'] = Piece::p,
    ['N'] = Piece::N, ['n'] = Piece::n,
    ['B'] = Piece::B, ['b'] = Piece::b,
    ['R'] = Piece::R, ['r'] = Piece::r,
    ['Q'] = Piece::Q, ['q'] = Piece::q,
    ['K'] = Piece::K, ['k'] = Piece::k,
    ['e'] = Piece::e
};

// Define char_to_type:
const int char_to_piece_type[] = {
    ['P'] = Piece_Type::Pawn,   ['p'] = Piece_Type::Pawn,
    ['N'] = Piece_Type::Knight, ['n'] = Piece_Type::Knight,
    ['B'] = Piece_Type::Bishop, ['b'] = Piece_Type::Bishop,
    ['R'] = Piece_Type::Rook,   ['r'] = Piece_Type::Rook,
    ['Q'] = Piece_Type::Queen,  ['q'] = Piece_Type::Queen,
    ['K'] = Piece_Type::King,   ['k'] = Piece_Type::King,
    ['e'] = Piece_Type::Empty
};

Move max_killer[2][64];
Move min_killer[2][64];
uint8_t max_history_move_score[64][64];
uint8_t min_history_move_score[64][64];
Move principal_variation_move[65][65];
Move principal_variation_length[65];

} // end Encoder namespace
} // end RedStone namespace