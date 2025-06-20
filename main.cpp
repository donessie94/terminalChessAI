#include"utils/utils.h"

int main()
{
    //
    compute_leapers_attacks_bb();
    compute_bishop_relevant_occupancy_bb();
    compute_rook_relevant_occupancy_bb();

    //
    compute_bishop_attack_table();
    compute_rook_attack_table();

    // for(int i = 0; i<64; i++)
    // {
    //     //print_bb( pawn_attack_mask[Color::white][i] );
    //     print_bb( bishop_magic[Square::d4].relevant_sqrs_bb );
    // }

    bitboard brd = 0ULL;
    set_bit(brd, Square::f3);
    set_bit(brd, Square::c2);

    print_bb(brd);

    print_bb(get_bishop_attack(brd, Square::e4));

    return 0;
}