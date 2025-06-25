#pragma once
#include"tables.h"

namespace RedStone{

// Move is a 32-bit unsigned integer:
using Move = uint32_t;

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

enum CastleSide { Kingside = 0, Queenside = 1 };

// Piece_Type enum:
enum Piece_Type : int {
    Pawn = 0,
    Knight = 1,
    Bishop = 2,
    Rook = 3,
    Queen = 4,
    King = 5,
    Empty = 6
};

// Castle rights dec 15 => bin 1111 => all castle rights
//               dec  8 => bin 1000 => qc only
//               dec  1 => bin 0001 => KC only etc...
enum Castle_Right { KC = 1, QC = 2, kc = 4, qc = 8 };

//
enum Color { white, black, all_color };

// Piece enum
// NOTE wwe moved the empty at the end so the pieces allign nicely
enum Piece { P, N, B, R, Q, K, p, n, b, r, q, k, e };

namespace Encoder{

extern const char *square_to_coord[128];    // coordinate strings
extern const char ascii_pieces[];           // ASCII piece symbols
extern const char *unicode_pieces[];        // Unicode piece symbols
extern const int char_to_piece[];
extern const int char_to_piece_type[];

// CASTLE_EMPTY_MASK[side][kq]
//   side: 0 = white, 1 = black
//   kq:   0 = kingside, 1 = queenside
// Squares the king passes through (and lands on) must not be attacked.
constexpr Bitboard CASTLE_EMPTY_MASK[2][2] = {
    {
        0x6000000000000000ULL,  // white kingside: bits 61,62
        0x0E00000000000000ULL   // white queenside: bits 57,58,59
    },
    {
        0x0000000000000060ULL,  // black kingside: bits 5,6
        0x000000000000000EULL   // black queenside: bits 1,2,3
    }
};
constexpr Bitboard CASTLE_THROUGH_MASK[2][2] = {
    {
        0x6000000000000000ULL,  // white kingside: bits 61,62
        0x0C00000000000000ULL   // white queenside: bits 59,58
    },
    {
        0x0000000000000060ULL,  // black kingside: bits 5,6
        0x000000000000000CULL   // black queenside: bits 3,2
    }
};

// Bit-field layout (bits 0 = least significant):
// bits  0..5   (6 bits): to-square index (0..63)
// bits  6..11  (6 bits): from-square index (0..63)
// bits 12..14  (3 bits): moved piece type (Pawn=0..King=5; 6=Empty unused here)
// bits 15..17  (3 bits): captured piece type (Empty=6 for none, or Pawn..King for actual captures)
// bits 18..20  (3 bits): promotion piece type (Empty=6 for none, or Knight/Bishop/Rook/Queen when pawn promotes)
// bits 21..28  (8 bits): flags byte (individual bits for en-passant, double-push, castling side, check, etc.)
// bits 29..31  (3 bits): reserved (currently unused; may store e.g. move ordering hints)

// Shifts:
constexpr int TO_SHIFT     = 0;
constexpr int FROM_SHIFT   = 6;
constexpr int MOVED_SHIFT  = 12;
constexpr int CAPT_SHIFT   = 15;
constexpr int PROMO_SHIFT  = 18;
constexpr int FLAGS_SHIFT  = 21;

// Masks:
constexpr uint32_t TO_MASK    = 0x3F << TO_SHIFT;    // 6 bits
constexpr uint32_t FROM_MASK  = 0x3F << FROM_SHIFT;  // 6 bits
constexpr uint32_t MOVED_MASK = 0x7  << MOVED_SHIFT; // 3 bits
constexpr uint32_t CAPT_MASK  = 0x7  << CAPT_SHIFT;  // 3 bits
constexpr uint32_t PROMO_MASK = 0x7  << PROMO_SHIFT; // 3 bits
constexpr uint32_t FLAGS_MASK = 0xFF << FLAGS_SHIFT; // 8 bits
// Remaining bits 29..31 unused for now.

// Flag bits within the 8-bit flags field (bit positions 0..7 within flags byte):
constexpr uint32_t FLAG_CAPTURE         = 1u << 0;  // move captures something
constexpr uint32_t FLAG_EN_PASSANT      = 1u << 1;  // this move is en-passant capture
constexpr uint32_t FLAG_DOUBLE_PAWN     = 1u << 2;  // pawn double-step
constexpr uint32_t FLAG_CASTLE_KINGSIDE = 1u << 3;  // kingside castle
constexpr uint32_t FLAG_CASTLE_QUEENSIDE= 1u << 4;  // queenside castle
constexpr uint32_t FLAG_PROMOTION       = 1u << 5;  // move is a promotion
constexpr uint32_t FLAG_CHECK           = 1u << 6;  // optional: move gives check
constexpr uint32_t FLAG_DISCOVERED_CHECK= 1u << 7;  // optional: move uncovers discovered check

// Helper to pack a move:
inline Move construct_move( int from_sq,
                            int to_sq,
                            Piece_Type moved_piece,
                            Piece_Type captured_piece,  // use Empty if no capture
                            Piece_Type promo_piece,     // use Empty if no promotion
                            uint32_t flags_byte         // combine FLAG_ bits here
                            )
{
    // Ensure inputs fit in their bit widths:
    // from_sq, to_sq in 0..63; moved_piece, captured_piece, promo_piece in 0..7; flags_byte in 0..0xFF.
    return  ( (uint32_t)(to_sq)             << TO_SHIFT )
          | ( (uint32_t)(from_sq)           << FROM_SHIFT )
          | ( (uint32_t)(moved_piece)       << MOVED_SHIFT )
          | ( (uint32_t)(captured_piece)    << CAPT_SHIFT )
          | ( (uint32_t)(promo_piece)       << PROMO_SHIFT )
          | ( (flags_byte)                  << FLAGS_SHIFT );
}

// Extractors:
inline int move_get_to(Move m) {
    return int((m & TO_MASK) >> TO_SHIFT);
}
inline int move_get_from(Move m) {
    return int((m & FROM_MASK) >> FROM_SHIFT);
}
inline Piece_Type move_get_moved_piece(Move m) {
    return Piece_Type((m & MOVED_MASK) >> MOVED_SHIFT);
}
inline Piece_Type move_get_captured_piece(Move m) {
    return Piece_Type((m & CAPT_MASK) >> CAPT_SHIFT);
}
inline Piece_Type move_get_promo_piece(Move m) {
    return Piece_Type((m & PROMO_MASK) >> PROMO_SHIFT);
}
inline uint32_t move_get_flags(Move m) {
    return uint32_t((m & FLAGS_MASK) >> FLAGS_SHIFT);
}

// Convenience flag-check functions:
inline bool move_is_capture(Move m) {
    // or test captured_piece != Empty
    return (move_get_flags(m) & FLAG_CAPTURE) != 0;
}
inline bool move_is_en_passant(Move m) {
    return (move_get_flags(m) & FLAG_EN_PASSANT) != 0;
}
inline bool move_is_double_pawn(Move m) {
    return (move_get_flags(m) & FLAG_DOUBLE_PAWN) != 0;
}
inline bool move_is_castle_kingside(Move m) {
    return (move_get_flags(m) & FLAG_CASTLE_KINGSIDE) != 0;
}
inline bool move_is_castle_queenside(Move m) {
    return (move_get_flags(m) & FLAG_CASTLE_QUEENSIDE) != 0;
}
inline bool move_is_promotion(Move m) {
    return (move_get_flags(m) & FLAG_PROMOTION) != 0;
}
inline bool move_gives_check(Move m) {
    return (move_get_flags(m) & FLAG_CHECK) != 0;
}
inline bool move_discovers_check(Move m) {
    return (move_get_flags(m) & FLAG_DISCOVERED_CHECK) != 0;
}

}   // end Encoder namespace
}   // end of Redstone namespace