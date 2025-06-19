#pragma once

// Max Square index is 119 =>   binary       decimal    thus we need 7 bits for source square
//                              0111 0111    119        representation and destination square
//
// Max Piece (enum) is 11 (there is a k (black king) but u cant promote to it)
// binary => 1011   decimal => 11   so we need 4 bits to represent it
/*
 * Move encoding (32-bit integer). Bit positions are numbered from 0 = least-significant bit.
 *
 * Layout:
 *   Bits   0–6    : source square index       (7 bits)
 *   Bits   7–13   : target square index       (7 bits)
 *   Bits  14–17   : promotion piece code      (4 bits)
 *   Bit      18   : capture flag              (1 bit)
 *   Bit      19   : double-pawn-push flag     (1 bit)
 *   Bit      20   : en-passant flag           (1 bit)
 *   Bit      21   : castling flag             (1 bit)
 *   Bits  22–31   : unused / reserved          (10 bits)
 *
 * Masks in full 32-bit binary (grouped in nibbles for clarity):
 *
 *   Source index mask (bits 0–6):
 *     0x0000007F = 0000 0000 0000 0000 0000 0000 0111 1111
 *
 *   Target index mask (bits 7–13):
 *     0x00003F80 = 0000 0000 0000 0000 0011 1111 1000 0000
 *
 *   Promotion code mask (bits 14–17):
 *     0x0003C000 = 0000 0000 0000 0011 1100 0000 0000 0000
 *
 *   Capture flag mask (bit 18):
 *     0x00040000 = 0000 0000 0000 0100 0000 0000 0000 0000
 *
 *   Double-pawn-push flag mask (bit 19):
 *     0x00080000 = 0000 0000 0000 1000 0000 0000 0000 0000
 *
 *   En-passant flag mask (bit 20):
 *     0x00100000 = 0000 0000 0001 0000 0000 0000 0000 0000
 *
 *   Castling flag mask (bit 21):
 *     0x00200000 = 0000 0000 0010 0000 0000 0000 0000 0000
 *
 * Example: to build a move integer 'mv':
 *   int mv = 0;
 *   mv |= (source_index & 0x7F);            // bits 0–6
 *   mv |= (target_index & 0x7F) << 7;       // bits 7–13
 *   if (is_promotion)      mv |= (prom_code & 0xF) << 14; // bits 14–17
 *   if (is_capture)        mv |= 1 << 18;   // bit 18
 *   if (is_double_pawn)    mv |= 1 << 19;   // bit 19
 *   if (is_en_passant)     mv |= 1 << 20;   // bit 20
 *   if (is_castling)       mv |= 1 << 21;   // bit 21
 *
 * To decode from 'mv':
 *   int src       =  mv         & 0x7F;           // bits 0–6
 *   int dst       = (mv >> 7)   & 0x7F;           // bits 7–13
 *   int prom_code = (mv >> 14)  & 0xF;            // bits 14–17
 *   bool cap      = (mv & (1 << 18)) != 0;        // bit 18
 *   bool dbl      = (mv & (1 << 19)) != 0;        // bit 19
 *   bool ep       = (mv & (1 << 20)) != 0;        // bit 20
 *   bool castl    = (mv & (1 << 21)) != 0;        // bit 21
 *
 * Binary illustrations (32 bits):
 *   0x0000007F (source mask) =
 *     0000 0000 0000 0000 0000 0000 0111 1111
 *
 *   0x00003F80 (target mask) =
 *     0000 0000 0000 0000 0011 1111 1000 0000
 *
 *   0x0003C000 (promo mask) =
 *     0000 0000 0000 0011 1100 0000 0000 0000
 *
 *   0x00040000 (capture) =
 *     0000 0000 0000 0100 0000 0000 0000 0000
 *
 *   0x00080000 (double-pawn) =
 *     0000 0000 0000 1000 0000 0000 0000 0000
 *
 *   0x00100000 (en-passant) =
 *     0000 0000 0001 0000 0000 0000 0000 0000
 *
 *   0x00200000 (castling) =
 *     0000 0000 0010 0000 0000 0000 0000 0000
 *
 * (Higher bits 22–31 remain zero unless we add further flags.)
 */

// Encode an entire move into a 32-bit integer:
constexpr unsigned int encode_move(unsigned source,
                               unsigned target,
                               unsigned promotedPiece,
                               bool isCapture,
                               bool isDoublePawn,
                               bool isEnpassant,
                               bool isCastling)
{
    // the '&' part guards against passing a number that contains more than 7 bits of information and corrupts everything
    unsigned int mv = 0;
    // source: 7 bits  (0..127)
    mv |= (source & 0x7F);
    // target: 7 bits
    mv |= (target & 0x7F) << 7;
    // promotion: 4 bits
    mv |= (promotedPiece & 0xF) << 14;
    // flags: here we need to convert to an unsigned integer to have enough space to the left to shift to create the mask needed
    mv |= (isCapture    ? 1u : 0u) << 18;       // 1u makes an unsigned int valued 1, 0u makes an unsigned int valued 0
    mv |= (isDoublePawn ? 1u : 0u) << 19;       // once we create that unsigned integer we now have the space to the left
    mv |= (isEnpassant  ? 1u : 0u) << 20;       // to sifht so we do so to apply the mask to the 'mv' variable to set the flag
    mv |= (isCastling   ? 1u : 0u) << 21;
    return mv;
}

// Decoding functions
constexpr unsigned int get_move_source(const unsigned int mv) { return ( mv & 0x7F ); }
constexpr unsigned int get_move_destination(const unsigned int mv) { return (( mv & 0x3F80 ) >> 7); }
constexpr unsigned int get_promotion_flag(const unsigned int mv) { return (( mv & 0x3C000 ) >> 14); }
constexpr bool get_capture_flag(const unsigned int mv) { return (( mv & 0x40000 ) >> 18); }
constexpr bool get_double_pawn_flag(const unsigned int mv) { return ( ( mv & 0x80000 ) >> 19); }
constexpr bool get_en_passant_flag(const unsigned int mv) { return ( ( mv & 0x100000 ) >> 20); }
constexpr bool get_castle_flag(const unsigned int mv) { return ( ( mv & 0x200000 ) >> 21); }

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
    a8, b8,  c8,  d8,  e8,  f8,  g8,  h8,
    a7, b7,  c7,  d7,  e7,  f7,  g7,  h7,
    a6, b6,  c6,  d6,  e6,  f6,  g6,  h6,
    a5, b5,  c5,  d5,  e5,  f5,  g5,  h5,
    a4, b4,  c4,  d4,  e4,  f4,  g4,  h4,
    a3, b3,  c3,  d3,  e3,  f3,  g3,  h3,
    a2, b2,  c2,  d2,  e2,  f2,  g2,  h2,
    a1, b1,  c1,  d1,  e1,  f1,  g1,  h1, no_sqr
};

// Declare externs for globals; definitions go in exactly one .cpp
extern const char start_position[];         // FEN string
extern const char tricky_position[];
// extern int board[128];                   // 0x88 board
extern const char *square_to_coord[128];    // coordinate strings
extern const char ascii_pieces[];           // ASCII piece symbols
extern const char *unicode_pieces[];        // Unicode piece symbols
extern const int char_to_piece[];           // char→Piece mapping