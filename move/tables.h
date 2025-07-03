#pragma once
#include <cstdint>
#include <cstdio>
#include <vector>
#include <random>
#include <algorithm> // for std::fill

namespace RedStone{

using Bitboard = uint64_t;
// FOR SAFETY USE ALL MACROS WITH SINGLE VARIABLES AND NOT COMPLEX EXPRESSIONS
// i do macros to minimize overhead from function calls and prioritize speed

// NOTE: a Bitboard is a number (an 64-bits (ULL) number )
// set, get or clear the specific bit (sqr) in a number
#define SET_BIT(bb, sqr) ( (bb) |=  (1ULL << (sqr)) )
#define GET_BIT(bb, sqr) ( (bb) &   (1ULL << (sqr)) )
#define POP_BIT(bb, sqr) ( (bb) &= ~(1ULL << (sqr)) )

// gets the number of set bits (count) in an number
#define COUNT_BITS(bb) ( __builtin_popcountll((bb)) )

// gets the number of trailing zeros of the least significant bit in a number
// in my representation this will give us exactly the idnex we need on the board
#define LS1B_IDX(bb) ( __builtin_ctzll((bb)) )

// GET POSSIBLE SLIDER ATTACK FROM ANY BOARD CONFIGURATION =================================================================================================

// get the piece current square, and look the "sliderPiece_relevant_sqr_bb" that corresponds to it
// now we must AND the "all_piece_position Bitboard" with this "sliderPiece_relevant_sqr_bb" to accomplish:
// 1: well you zero out the rest of the board to focus on those relevant squares
// They are relevant because their finite combinations determine every single possible this sliderPiece attacks moves (or moves)
// note that end squares do not change anything (hence why we dont include them) since pieces position here dont
// matter for our purpose (even if it was a piece here we still attack it, see it?)
// 2: within these possible squares the AND operation basically give us the right combination of blockers (the occupancy subset Bitboard)
// since we already have a perfect hash ---> ( thisOccupancy_subset_bitboard * SliderPieceMagic[current_square] ) >> ( 64 - SliderPieceShift[current_square] )
// that maps each of these unique configurations to an unique (0 to 2^N) index, where N is the sliderPieceShift index we simply use it
// to look in our "sliderPiece_attack_bb" array, we index by first: [current index] and second: [our hash function result]
// the Bitboard we get from this lookup is all possible attack (and moves too) from that square for that board configuration for that piece
// NOTE: we still need to check if the piece is allied or enemy before moving there because we dont know this information from this alghortihm
// and of course we need to make sure we dont leave our king in check

// we use macros to avoid function call overheads to maximize speed (inline functions may work too (and safer))
#define GET_BISHOP_ATTACK(occ, sq) \
    (Tables::bishop_attack_bb[(sq)][ (((occ) & Tables::bishop_magic[(sq)].relevant_sqrs_bb) \
                             * Tables::bishop_magic[(sq)].magic_bb) \
                             >> Tables::bishop_magic[(sq)].shift ])

//
#define GET_ROOK_ATTACK(occ, sq) \
    (Tables::rook_attack_bb[(sq)][ ((((occ) & Tables::rook_magic[(sq)].relevant_sqrs_bb) \
                              * Tables::rook_magic[(sq)].magic_bb) \
                              >> Tables::rook_magic[(sq)].shift) ])

// NOTE queen is just attack or rook attacks
#define GET_QUEEN_ATTACK(occ, sq) \
    ( GET_BISHOP_ATTACK((occ),(sq)) | GET_ROOK_ATTACK((occ),(sq)) )


// Pawn attacks: turn is bool (false=WHITE, true=BLACK), sq is int 0..63
#define GET_PAWN_ATTACK(turn, sq) \
    ( Tables::pawn_attack_bb[(turn) ? 1 : 0][(sq)] )

#define GET_KNIGHT_ATTACK(sq) \
    ( Tables::knight_attack_bb[(sq)] )

#define GET_KING_ATTACK(sq) \
    ( Tables::king_attack_bb[(sq)] )

namespace Tables{


// Castling rights bits:
//  KC = 1 (White kingside), QC = 2 (White queenside),
//  kc = 4 (Black kingside), qc = 8 (Black queenside)
/*
  How rights are masked on each square move:
      initial rights = 1111 (15)

  White:
    King moved:            1111 & 1100 = 1100 (12)
    White king’s rook:     1111 & 1110 = 1110 (14)
    White queen’s rook:    1111 & 1101 = 1101 (13)

  Black:
    King moved:            1111 & 0011 = 0011 (3)
    Black king’s rook:     1111 & 1011 = 1011 (11)
    Black queen’s rook:    1111 & 0111 = 0111 (7)
*/
// For each square a8…h1, mask out any castling rights invalidated by moving from that square.
static constexpr int castling_table[64] = {
    //  a8   b8   c8   d8   e8   f8   g8   h8
       7,   15,  15,  15,   3,  15,  15,  11,
    //  a7   b7   c7   d7   e7   f7   g7   h7
      15,   15,  15,  15,  15,  15,  15,  15,
    //  a6   b6   c6   d6   e6   f6   g6   h6
      15,   15,  15,  15,  15,  15,  15,  15,
    //  a5   b5   c5   d5   e5   f5   g5   h5
      15,   15,  15,  15,  15,  15,  15,  15,
    //  a4   b4   c4   d4   e4   f4   g4   h4
      15,   15,  15,  15,  15,  15,  15,  15,
    //  a3   b3   c3   d3   e3   f3   g3   h3
      15,   15,  15,  15,  15,  15,  15,  15,
    //  a2   b2   c2   d2   e2   f2   g2   h2
      15,   15,  15,  15,  15,  15,  15,  15,
    //  a1   b1   c1   d1   e1   f1   g1   h1
      13,   15,  15,  15,  12,  15,  15,  14
};

constexpr Bitboard Rank8 = 0x00000000000000FFULL;
constexpr Bitboard Rank7 = 0x000000000000FF00ULL;
constexpr Bitboard Rank6 = 0x0000000000FF0000ULL;
constexpr Bitboard Rank3 = 0x0000FF0000000000ULL;
constexpr Bitboard Rank2 = 0x00FF000000000000ULL;
constexpr Bitboard Rank1 = 0xFF00000000000000ULL;

// “not A-file” = all bits except those on file A:
constexpr Bitboard not_a_file = 18374403900871474942ULL;
//   hex:    0xFEFEFEFEFEFEFEFEULL
//   decimal: 18374403900871474942ULL

// “not H-file” = all bits except those on file H:
constexpr Bitboard not_h_file = 9187201950435737471ULL;
//   hex:    0x7F7F7F7F7F7F7F7FULL
//   decimal:  9187201950435737471ULL

// “not HG-file” = all bits except those on files G or H:
//   file G | file H = 0x4040404040404040ULL | 0x8080808080808080ULL = 0xC0C0C0C0C0C0C0C0ULL
constexpr Bitboard not_hg_file = 4557430888798830399ULL;
//   hex:    0x3F3F3F3F3F3F3F3FULL
//   decimal: 4557430888798830399ULL

// “not AB-file” = all bits except those on files A or B:
//   file A | file B = 0x0101010101010101ULL | 0x0202020202020202ULL = 0x0303030303030303ULL
constexpr Bitboard not_ab_file = 18229723555195321596ULL;
//   hex:    0xFCFCFCFCFCFCFCFCULL
//   decimal: 18229723555195321596ULL

static const int PIECE_VALUE[6] =
{
  100, 300, 350, 500, 900, 0
};

// POSITION TABLES ==================================================================================================================

static inline __attribute__((always_inline)) int mirror_square(int sq) {
    // flip a8<->a1, b8<->b1, … h8<->h1
    return sq ^ 56;
    // or: return 63 - sq; (same thing)
}
// Piece‐Square Table [piece][square], square = 0..63 (a8..h1), piece = 0:Pawn,1:Knight,2:Bishop,3:Rook,4:Queen,5:King
static const int PST[6][64] = {
    // Pawn
    {
       0,   0,   0,   0,   0,   0,   0,   0,
      40,  80,  80,  70,  70,  80,  80,  40,
       5,   5,  10,  25,  25,  10,   5,   5,
       5,   5,  10,  25,  25,  10,   5,   5,
       5,   5,  10,  25,  25,  10,   5,   5,
      10,  10,  20,  10,  10,  20,  10,  10,
      40,  40,  40, -25, -25,  40,  40,  40,
       0,   0,   0,   0,   0,   0,   0,   0
    },
    // Knight
    {
     -50, -40, -30, -30, -30, -30, -40, -50,
     -40, -20,   0,   5,   5,   0, -20, -40,
     -30,   5,  10,  15,  15,  10,   5, -30,
     -30,   0,  15,  20,  20,  15,   0, -30,
     -30,   5,  15,  20,  20,  15,   5, -30,
     -30,   0,  10,  15,  15,  10,   0, -30,
     -40, -20,   0,   0,   0,   0, -20, -40,
     -50, -40, -30, -30, -30, -30, -40, -50
    },
    // Bishop
    {
     -20, -10, -10, -10, -10, -10, -10, -20,
     -10,   5,   0,   0,   0,   0,   5, -10,
     -10,   5,   5,   5,   5,   5,   5, -10,
     -10,   0,  10,  10,  10,  10,   0, -10,
     -10,   5,   5,  10,  10,   5,   5, -10,
     -10,   0,   5,  10,  10,   5,   0, -10,
     -10,   0,   0,   0,   0,   0,   0, -10,
     -20, -10, -10, -10, -10, -10, -10, -20
    },
    // Rook
    {
       0,   0,   0,   0,   0,   0,   0,   0,
      -5,   0,   0,   0,   0,   0,   0,  -5,
      -5,   0,   0,   0,   0,   0,   0,  -5,
      -5,   0,   0,   0,   0,   0,   0,  -5,
      -5,   0,   0,   0,   0,   0,   0,  -5,
      -5,   0,   0,   0,   0,   0,   0,  -5,
       5,   0,   0,   0,   0,   0,   0,   5,
       0,   0,   4,   5,   5,   4,   0,   0
    },
    // Queen
    {
     -20, -10, -10,  -5,  -5, -10, -10, -20,
     -10,   0,   5,   0,   0,   0,   0, -10,
     -10,   5,   5,   5,   5,   5,   0, -10,
      -5,   0,   5,   5,   5,   5,   0,  -5,
       0,   0,   5,   5,   5,   5,   0,  -5,
     -10,   0,   5,   5,   5,   5,   0, -10,
     -10,   0,   0,   0,   0,   0,   0, -10,
     -20, -10, -10,  -5,  -5, -10, -10, -20
    },
    // King
    {
     -30, -40, -40, -50, -50, -40, -40, -30,
     -30, -40, -40, -50, -50, -40, -40, -30,
     -30, -40, -40, -50, -50, -40, -40, -30,
     -30, -40, -40, -50, -50, -40, -40, -30,
     -20, -30, -30, -40, -40, -30, -30, -20,
     -10, -20, -20, -20, -20, -20, -20, -10,
      20,  20,   0,   0,   0,   0,  20,  20,
      20,  30,  20,   0,   0,   0,  30,  20
    }
};

// ============================================================================================================================

//----------------------------------------------------------------
// MVV/LVA table, attacker × victim in [Pawn..King]
//----------------------------------------------------------------
static constexpr int MVV_LVA[5][5] = {
  /*           victim:    P    N    B    R    Q    K  */
  /* attacker P */ {      105, 205, 305, 405, 505 },
  /*         N */  {      104, 204, 304, 404, 504 },
  /*         B */  {      103, 203, 303, 403, 503 },
  /*         R */  {      102, 202, 302, 402, 502 },
  /*         Q */  {      101, 201, 301, 401, 501 },
  // kings never capture in MVV/LVA
};

// global array, indexed by square 0…63
extern Bitboard KING_ZONE[64];

//
static inline __attribute__((always_inline))
void build_king_zones() {
  for (int sq = 0; sq < 64; ++sq) {
    Bitboard m = 0ULL;
    int rk = sq >> 3, fl = sq & 7;
    // only one square away in any direction, plus the center (dr=0,df=0)
    for (int dr = -1; dr <= 1; ++dr) {
      for (int df = -1; df <= 1; ++df) {
        int r2 = rk + dr, f2 = fl + df;
        if (r2 >= 0 && r2 < 8 && f2 >= 0 && f2 < 8) {
          m |= (Bitboard(1) << (r2*8 + f2));
        }
      }
    }
    KING_ZONE[sq] = m;
  }
}

// helper function
inline int sign(int x) {
    return (x > 0) ? 1 : (x < 0) ? -1 : 0;
}

extern Bitboard pawn_attack_bb[2][64];
extern Bitboard knight_attack_bb[64];
extern Bitboard king_attack_bb[64];
//
extern Bitboard bishop_attack_bb[64][(1u<<9)];
extern Bitboard rook_attack_bb[64][(1u<<12)];

// I condensed All this into this struct for clarity
// extern Bitboard bishop_relevant_sqrs_bb[64];
// extern Bitboard rook_relevant_sqrs_bb[64];
// extern int bishop_magic_shift[64];
// extern int rook_magic_shift[64];
// extern Bitboard bishop_magic[64];
// extern Bitboard rook_magic[64];
struct Slider_Magic
{
    Bitboard relevant_sqrs_bb;
    uint8_t  shift;
    Bitboard magic_bb;
};
// Declare globals:
extern Slider_Magic rook_magic[64];
extern Slider_Magic bishop_magic[64];

// Pin checking helpers precomputed tables
extern Bitboard diagonal_ray_mask[64];
extern Bitboard orthogonal_ray_mask[64];
//
extern Bitboard between_squares_mask[64][64];

// initializes our precomputed tables needed
void initialize_precomputed_tables();

//
void compute_leapers_attacks_bb();
//
void compute_bishop_relevant_occupancy_bb();
void compute_rook_relevant_occupancy_bb();
void compute_diagonal_ray_mask();
void compute_orthogonal_ray_mask();

// between_squares_mask[a][b] will have bits set for all squares strictly between a and b,
// if a and b lie on the same rank, file, or diagonal. Otherwise it will be zero.
// square index 0=a8, 1=b8, ..., 7=h8,
void compute_between_squares_mask();

//
void compute_bishop_attack_table();
void compute_rook_attack_table();

// Returns a Bitboard with bits set on all squares a rook on `sq` can attack/capture
// given blockers in `relevant_occupancy_bb`.
Bitboard compute_bishop_attack_bb(Bitboard relevant_occupancy_bb, int sq);
Bitboard compute_rook_attack_bb(Bitboard relevant_occupancy_bb, int sq);

} // end of Tables namespace
} // end of RedStone namespace