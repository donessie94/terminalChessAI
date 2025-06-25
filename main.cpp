#include"utils/misc.h"

using namespace RedStone;

// Define the FEN start string:
const char start_position[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
const char tricky_position[] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
const char test_position[] = "1r2k2r/p1ppqp2/n4nB1/2BPpP1b/1p2PN1b/6p1/PPP3PP/3K1R2 w KQkq - 0 1";

int main()
{
    Tables::initialize_precomputed_tables();

    // void parse_fen_str(const char fen[], bitboard player_occ_bb[3], bitboard piece_occ_bb[2][6], bool turn, int castle_right, int en_passant, int king_pos[])
    Utils::parse_fen_str(start_position, Move_Gen::player_occ_bb, Move_Gen::piece_occ_bb, Move_Gen::turn, Move_Gen::castle_right, Move_Gen::en_passant, Move_Gen::king_position);

    // void print_mini_board(bitboard player_occ_bb[3], bitboard piece_occ_bb[2][6], bool turn, int castle_right, int en_passant)
    Utils::print_mini_board(Move_Gen::player_occ_bb, Move_Gen::piece_occ_bb, Move_Gen::turn, Move_Gen::castle_right, Move_Gen::en_passant);

    Move_Gen::generate_moves();

    // void print_attack_map(bitboard player_occ_bb[3], bitboard piece_occ_bb[2][6], const bool turn)
    //print_attack_map(player_occ_bb, piece_occ_bb, turn);

    printf("Generated Moves: %d", Move_Gen::move_count);

    return 0;
}