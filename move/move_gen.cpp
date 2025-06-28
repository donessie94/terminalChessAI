#include"move_gen.h"

namespace RedStone{
namespace Move_Gen{

// Globals====================================================================================================================================================
Bitboard pin_mask;
Bitboard pin_ray_mask[64];
Bitboard discovered_mask;
Bitboard discovered_ray_mask[64];
Bitboard check_mask;
int num_attackers;
Move_List move_list[MAX_DEPTH];

// not yet reinitialized per move
Piece_Type mailbox[2][64];
int king_position[2];
int castle_right;
int en_passant;
bool turn;
Bitboard piece_occ_bb[2][6];
Bitboard player_occ_bb[3];
// ============================================================================================================================================================




} // end Move_Gen namespace
}// end RedStone namespace

// inline Move make_move(int from_sq,
//                       int to_sq,
//                       Piece_Type moved_piece,
//                       Piece_Type captured_piece,  // use Empty if no capture
//                       Piece_Type promo_piece,     // use Empty if no promotion
//                       uint32_t flags_byte         // combine FLAG_ bits here
//                       )

// Flag bits within the 8-bit flags field (bit positions 0..7 within flags byte):
// constexpr uint32_t FLAG_CAPTURE         = 1u << 0;  // move captures something
// constexpr uint32_t FLAG_EN_PASSANT      = 1u << 1;  // this move is en-passant capture
// constexpr uint32_t FLAG_DOUBLE_PAWN     = 1u << 2;  // pawn double-step
// constexpr uint32_t FLAG_CASTLE_KINGSIDE = 1u << 3;  // kingside castle
// constexpr uint32_t FLAG_CASTLE_QUEENSIDE= 1u << 4;  // queenside castle
// constexpr uint32_t FLAG_PROMOTION       = 1u << 5;  // move is a promotion
// constexpr uint32_t FLAG_CHECK           = 1u << 6;  // optional: move gives check
// constexpr uint32_t FLAG_DISCOVERED_CHECK= 1u << 7;  // optional: move uncovers discovered check