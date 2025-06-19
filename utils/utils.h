#pragma once
#include <cstdint>
#include <cstdio>
#include <vector>

using bitboard = uint64_t;

// NOTE: a bitboard is a number (an 64-bits (ULL) number )
// set, get or clear the specific bit (sqr) in a number
#define set_bit(bb, sqr) ( bb |=  (1ULL << sqr) )
#define get_bit(bb, sqr) ( bb &   (1ULL << sqr) )
#define pop_bit(bb, sqr) ( bb &= ~(1ULL << sqr) )

// gets the number of set bits (count) in an number
#define count_bits(bb) ( __builtin_popcountll(bb) )

// gets the number of trailing zeros of the least significant bit in a number
// in my representation this will give us exactly the idnex we need on the board
#define ls1b_index(bb) ( __builtin_ctzll(bb) )

// “not A-file” = all bits except those on file A:
constexpr bitboard not_a_file = 18374403900871474942ULL;
//   hex:    0xFEFEFEFEFEFEFEFEULL
//   decimal: 18374403900871474942ULL

// “not H-file” = all bits except those on file H:
constexpr bitboard not_h_file = 9187201950435737471ULL;
//   hex:    0x7F7F7F7F7F7F7F7FULL
//   decimal:  9187201950435737471ULL

// “not HG-file” = all bits except those on files G or H:
//   file G | file H = 0x4040404040404040ULL | 0x8080808080808080ULL = 0xC0C0C0C0C0C0C0C0ULL
constexpr bitboard not_hg_file = 4557430888798830399ULL;
//   hex:    0x3F3F3F3F3F3F3F3FULL
//   decimal: 4557430888798830399ULL

// “not AB-file” = all bits except those on files A or B:
//   file A | file B = 0x0101010101010101ULL | 0x0202020202020202ULL = 0x0303030303030303ULL
constexpr bitboard not_ab_file = 18229723555195321596ULL;
//   hex:    0xFCFCFCFCFCFCFCFCULL
//   decimal: 18229723555195321596ULL

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

enum Color { white, black };

extern bitboard pawn_attack_bb[2][64];
extern bitboard knight_attack_bb[64];
extern bitboard king_attack_bb[64];

extern bitboard bishop_attack_bb[64];


extern bitboard bishop_relevant_sqrs_bb[64];
extern bitboard rook_relevant_sqrs_bb[64];
extern bitboard queen_relevant_sqrs_bb[64];

void print_bb(bitboard bb);

void compute_leapers_attacks_bb();

//
void compute_bishop_relevant_occupancy_bb();
void compute_rook_relevant_occupancy_bb();
void compute_queen_relevant_occupancy_bb();

// Returns a bitboard with bits set on all squares a rook on `sq` can attack/capture
// given blockers in `relevant_occupancy_bb`.
bitboard compute_bishop_attack_bb(bitboard relevant_occupancy_bb, int sq);
bitboard compute_rook_attack_bb(bitboard relevant_occupancy_bb, int sq);
bitboard compute_queen_attack_bb(bitboard relevant_occupancy_bb, int sq);