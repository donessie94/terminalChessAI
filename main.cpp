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
    //     print_bb( queen_relevant_sqrs_bb[i] );
    // }

    bitboard brd = 0ULL;
    set_bit(brd, Square::e4);
    set_bit(brd, Square::c3);
    set_bit(brd, Square::f4);
    set_bit(brd, Square::g7);

    print_bb(brd);
    print_bb(compute_bishop_attack_bb(brd, Square::d2));



    return 0;
}