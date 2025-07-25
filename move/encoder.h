#pragma once
#include <cstdint>
#include <string>

namespace RedStone{

using Bitboard = uint64_t;

namespace Move_Gen
{
    struct TTEntry;                // forward only
    extern TTEntry   TT[];         // extern
    extern uint64_t  position_hash;
}

// forward‐declare the Tables namespace and MVV_LVA array:
namespace Tables
{
    extern const uint16_t MVV_LVA[5][5];   // just “declare” the capture‐order table
}


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


// ┌──────────────────┬───────────────┐
// │ Bits 0–6         │ Bits 7–10     │  Bits 11–15 unused
// │ en_passant (0–64)│ castle_right  │
// └──────────────────┴───────────────┘
// plus a full 64-bit hash
struct UndoPacked
{
    uint16_t   flags;  //  EP (7 bits) + Castle (4 bits)
    uint64_t   hash;   //  full Zobrist hash before the move
};

namespace Encoder{

// Undo ============================================================================================================

// Masks & shifts
static constexpr uint16_t EP_MASK    = 0x007F;     // 0b0000 0000 0111 1111
static constexpr uint16_t CR_MASK    = 0x0780;     // 0b0000 0111 1000 0000
static constexpr int      CR_SHIFT   = 7;

// pack EP + castle into 16 bits
static inline __attribute__((always_inline))
uint16_t pack_flags(int en_passant, int castle_right) {
    return uint16_t(
         (en_passant     & EP_MASK)
       | ((castle_right & 0xF) << CR_SHIFT)
    );
}

// unpackors
static inline __attribute__((always_inline))
int unpack_ep(uint16_t f) {
    return f & EP_MASK;
}
static inline __attribute__((always_inline))
int unpack_cr(uint16_t f) {
    return (f & CR_MASK) >> CR_SHIFT;
}

// Undo ============================================================================================================

extern const char *square_to_coord[65];    // coordinate strings
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

// MOVE LAYOUT =================================================================================================================

/*
    Bit-field layout (bits 0 = LSB):
      0..5    ( 6 bits) to-square
      6..11   ( 6 bits) from-square
     12..14   ( 3 bits) moved piece (0=Pawn…5=King)
     15..17   ( 3 bits) captured piece (6=Empty)
     18..20   ( 3 bits) promotion piece (6=Empty)
     21       ( 1 bit ) FLAG_DOUBLE_PAWN
     22       ( 1 bit ) FLAG_CASTLE_KINGSIDE
     23       ( 1 bit ) FLAG_CASTLE_QUEENSIDE
     24       ( 1 bit ) FLAG_EN_PASSANT
     25       ( 1 bit ) FLAG_CHECK
     26..31   ( 6 bits) unused/reserved
*/

constexpr int TO_SHIFT      =  0;
constexpr int FROM_SHIFT    =  6;
constexpr int MOVED_SHIFT   = 12;
constexpr int CAPT_SHIFT    = 15;
constexpr int PROMO_SHIFT   = 18;
constexpr int FLAGS_SHIFT   = 21;  // covers double pawn, castling, en passant
constexpr int CHECK_SHIFT   = 25;  // check flag at bit 25

constexpr uint32_t TO_MASK     = 0x3Fu << TO_SHIFT;    // bits  0..5
constexpr uint32_t FROM_MASK   = 0x3Fu << FROM_SHIFT;  // bits  6..11
constexpr uint32_t MOVED_MASK  = 0x7u  << MOVED_SHIFT; // bits 12..14
constexpr uint32_t CAPT_MASK   = 0x7u  << CAPT_SHIFT;  // bits 15..17
constexpr uint32_t PROMO_MASK  = 0x7u  << PROMO_SHIFT; // bits 18..20
constexpr uint32_t FLAGS_MASK  = 0xFu  << FLAGS_SHIFT; // bits 21..24
constexpr uint32_t CHECK_MASK  = 1u   << CHECK_SHIFT; // bit 25

// Flags:
constexpr uint32_t FLAG_DOUBLE_PAWN      = 1u << 21;
constexpr uint32_t FLAG_CASTLE_KINGSIDE  = 1u << 22;
constexpr uint32_t FLAG_CASTLE_QUEENSIDE = 1u << 23;
constexpr uint32_t FLAG_EN_PASSANT       = 1u << 24;
constexpr uint32_t FLAG_CHECK            = 1u << CHECK_SHIFT;

// packer
static inline __attribute__((always_inline)) Move construct_move(
    int       from_sq,
    int       to_sq,
    Piece_Type moved_piece,
    Piece_Type captured_piece,  // use Empty=6 if no capture
    Piece_Type promo_piece,     // use Empty=6 if no promo
    uint32_t  flags              // OR any of FLAG_* bits (including FLAG_CHECK)
) {
    return   (uint32_t(to_sq)           << TO_SHIFT)
           | (uint32_t(from_sq)         << FROM_SHIFT)
           | (uint32_t(moved_piece)     << MOVED_SHIFT)
           | (uint32_t(captured_piece)  << CAPT_SHIFT)
           | (uint32_t(promo_piece)     << PROMO_SHIFT)
           | (flags                     & (FLAGS_MASK | CHECK_MASK));
}

// extractors
static inline __attribute__((always_inline)) int move_get_to(Move m) {
    return int((m & TO_MASK) >> TO_SHIFT);
}
static inline __attribute__((always_inline)) int move_get_from(Move m) {
    return int((m & FROM_MASK) >> FROM_SHIFT);
}
static inline __attribute__((always_inline)) Piece_Type move_get_moved_piece(Move m) {
    return Piece_Type((m & MOVED_MASK) >> MOVED_SHIFT);
}
static inline __attribute__((always_inline)) Piece_Type move_get_captured_piece(Move m) {
    return Piece_Type((m & CAPT_MASK) >> CAPT_SHIFT);
}
static inline __attribute__((always_inline)) Piece_Type move_get_promo_piece(Move m) {
    return Piece_Type((m & PROMO_MASK) >> PROMO_SHIFT);
}
static inline __attribute__((always_inline)) uint32_t move_get_flags(Move m) {
    return (m & FLAGS_MASK);
}
static inline __attribute__((always_inline)) bool move_get_check(Move m) {
    return (m & CHECK_MASK) != 0;
}

// convenience flag-checks
static inline __attribute__((always_inline)) bool move_is_double_pawn(Move m) {
    return (move_get_flags(m) & FLAG_DOUBLE_PAWN) != 0;
}
static inline __attribute__((always_inline)) bool move_is_castle_kingside(Move m) {
    return (move_get_flags(m) & FLAG_CASTLE_KINGSIDE) != 0;
}
static inline __attribute__((always_inline)) bool move_is_castle_queenside(Move m) {
    return (move_get_flags(m) & FLAG_CASTLE_QUEENSIDE) != 0;
}
static inline __attribute__((always_inline)) bool move_is_en_passant(Move m) {
    return (move_get_flags(m) & FLAG_EN_PASSANT) != 0;
}
static inline __attribute__((always_inline)) bool move_is_check(Move m) {
    return move_get_check(m);
}

static constexpr uint8_t PROMOTION_BONUS = 100;
static constexpr uint16_t KILLER1_BONUS = 60;
static constexpr uint16_t KILLER2_BONUS = 50;
static constexpr uint8_t HISTORY_BONUS = 2;
static constexpr uint16_t PV_BONUS = 2000;
static constexpr uint16_t TT_BONUS = 1000;
static constexpr uint16_t CHECK_BONUS = 40;

// for storing historic moves (64x64 cuz from_squ -> to_sq)
// we want to index a move but obviously having something like
// historic[move] = something wont cut it because the HUGE
// ammount of different moves possible, ex:
// e2e1.e2e3,e2e4,e2e5, ... and so on HUGE
//
// with this approach down here we
// unquely identify the move [from][to] non-captures, non-promos
// and it only cost us at most 4 096 possible quiet moves WAY LOWER
extern uint8_t max_history_move_score[64][64];
extern uint8_t min_history_move_score[64][64];

// for storing our killer moves
extern Move max_killer[2][64];
extern Move min_killer[2][64];

// ==================================================
// Principal Variation Array                        |
// ==================================================
// idx   0      1      2       3 . . .  Max_Depth+1 |
//                                                  |
// 0    M1     M2     M3      M4                    |
//                                                  |
// 1     0     M2     M3      M4                    |
//                                                  |
// 2     0      0     M3      M4                    |
//                                                  |
// 3     0      0      0      M4                    |
// .                                                |
// .                                                |
// .                                                |
// Max_Depth+1                                      |
// ==================================================
extern Move principal_variation_move[65][65];
extern Move principal_variation_length[65];

// ============================================================================================================================

/// Convert a coordinate string ("a8".."h1") to a square index (0..63), or no_sqr if invalid.
static inline int coord_to_square(const std::string &coord)
{
    if (coord.size() < 2)
        return no_sqr;
    char file = coord[0];
    char rank = coord[1];
    // file in 'a'..'h', rank in '1'..'8'
    if (file < 'a' || file > 'h' || rank < '1' || rank > '8')
        return no_sqr;
    int f = file - 'a';         // 0..7
    int r = '8' - rank;         // '8'->0, '1'->7
    return r * 8 + f;           // row-major 0=a8,1=b8,..7=h8, 8=a7,..
}

// Overload for C-string (e.g. square_to_coord entries)
static inline int coord_to_square(const char *coord)
{
    // delegate to std::string version
    return coord_to_square(std::string(coord));
}

}   // end Encoder namespace
}   // end of Redstone namespace