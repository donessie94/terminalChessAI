#include"utils/misc.h"

int main()
{
    //
    compute_leapers_attacks_bb();
    compute_bishop_relevant_occupancy_bb();
    compute_rook_relevant_occupancy_bb();

    //
    compute_bishop_attack_table();
    compute_rook_attack_table();

    // void parse_fen_str(const char fen[], bitboard player_occ_bb[3], bitboard piece_occ_bb[2][6], bool turn, int castle_right, int en_passant, int king_pos[]);
    bool turn = false;
    int castle_right = 15; int en_passant = 0;
    int king_pos[2];
    parse_fen_str(tricky_position, player_occ_bb, piece_occ_bb, turn, castle_right, en_passant, king_pos);
    // void print_mini_board(bitboard player_occ_bb[3], bitboard piece_occ_bb[2][6], bool turn, int castle_right, int en_passant)
    print_mini_board(player_occ_bb, piece_occ_bb, turn, castle_right, en_passant);

    return 0;
}