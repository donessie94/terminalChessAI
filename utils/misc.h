#pragma once
#include "../move/encoder.h"
#include "../move/move_gen.h"

//

namespace RedStone{

namespace Utils{

void print_bb(Bitboard bb);

void print_mini_board(Bitboard player_occ_bb[3], Bitboard piece_occ_bb[2][6], bool turn, int castle_right, int en_passant);
void print_attack_map(Bitboard player_occ_bb[3], Bitboard piece_occ_bb[2][6], const bool turn);
void parse_fen_str(const char fen[], Bitboard player_occ_bb[3], Bitboard piece_occ_bb[2][6], bool &turn, int &castle_right, int &en_passant, int king_pos[]);

// human‐readable names for Piece_Type 0..6
static const char* piece_names[] = {
    "Pawn", "Knight", "Bishop", "Rook", "Queen", "King", "Empty"
};

static inline void print_move_info(Move m) {
    int from       = Encoder::move_get_from(m);
    int to         = Encoder::move_get_to(m);
    Piece_Type mv  = Encoder::move_get_moved_piece(m);
    Piece_Type cp  = Encoder::move_get_captured_piece(m);
    Piece_Type pp  = Encoder::move_get_promo_piece(m);
    uint32_t flags = Encoder::move_get_flags(m);

    // decode file/rank for a8..h1 mapping:
    char from_file = 'a' + (from & 7);
    char from_rank = '8' - (from >> 3);
    char to_file   = 'a' + (to   & 7);
    char to_rank   = '8' - (to   >> 3);

    printf("Moved:     %s from %c%c to %c%c\n",
           piece_names[(int)mv],
           from_file, from_rank,
           to_file,   to_rank);

    printf("Captured:  %s\n", piece_names[(int)cp]);
    printf("Promoted:  %s\n", piece_names[(int)pp]);

    printf("Flags:     ");
    int any = 0;

    #define FLAG_PRINT(f, name) \
      do { if (flags & (f)) { \
            if (any) printf(" | "); \
            printf("%s", name); \
            any = 1; \
          } } while (0)

    FLAG_PRINT(Encoder::FLAG_CAPTURE,          "Capture");
    FLAG_PRINT(Encoder::FLAG_EN_PASSANT,       "EnPassant");
    FLAG_PRINT(Encoder::FLAG_DOUBLE_PAWN,      "DoublePawn");
    FLAG_PRINT(Encoder::FLAG_CASTLE_KINGSIDE,  "CastleK");
    FLAG_PRINT(Encoder::FLAG_CASTLE_QUEENSIDE, "CastleQ");
    FLAG_PRINT(Encoder::FLAG_PROMOTION,        "Promotion");
    FLAG_PRINT(Encoder::FLAG_CHECK,            "Check");    
    FLAG_PRINT(Encoder::FLAG_DISCOVERED_CHECK, "DiscCheck");

    if (!any) {
        printf("None");
    }
    printf("\n\n");

    #undef FLAG_PRINT
}

} // end Utils namespace
} // end RedStone namespace