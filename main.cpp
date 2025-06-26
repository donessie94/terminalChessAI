#include"utils/misc.h"

using namespace RedStone;

// Define the FEN start string:
const char start_position[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
const char tricky_position[] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
const char test_position[] = "r2k3r/p1ppqp2/n4nB1/2BPpPbb/1p2PN2/1p1R4/PPP3PP/3K1R2 w KQkq - 0 1";

int main()
{
    Tables::initialize_precomputed_tables();

    // void parse_fen_str(const char fen[], bitboard player_occ_bb[3], bitboard piece_occ_bb[2][6], bool turn, int castle_right, int en_passant, int king_pos[])
    Utils::parse_fen_str(test_position, Move_Gen::player_occ_bb, Move_Gen::piece_occ_bb, Move_Gen::turn, Move_Gen::castle_right, Move_Gen::en_passant, Move_Gen::king_position);

    // void print_mini_board(bitboard player_occ_bb[3], bitboard piece_occ_bb[2][6], bool turn, int castle_right, int en_passant)
    Utils::print_mini_board(Move_Gen::player_occ_bb, Move_Gen::piece_occ_bb, Move_Gen::turn, Move_Gen::castle_right, Move_Gen::en_passant);

    Move_Gen::generate_moves();

    // void print_attack_map(bitboard player_occ_bb[3], bitboard piece_occ_bb[2][6], const bool turn)
    //Utils::print_attack_map(Move_Gen::player_occ_bb, Move_Gen::piece_occ_bb, Move_Gen::turn);


    // for(int i=0; i<Move_Gen::move_count; i++)
    //     Utils::print_move_info(Move_Gen::valid_moves[i]);

    printf("Generated Moves: %d", Move_Gen::move_count);



    return 0;
}