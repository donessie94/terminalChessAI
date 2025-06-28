#include"utils/misc.h"
#include"search/search.h"
#include <chrono>
#include"uci_protocol/uci.h"

using namespace RedStone;

int main()
{
    //using namespace std::chrono;

    //Tables::initialize_precomputed_tables();

    // // void parse_fen_str(const char fen[], bitboard player_occ_bb[3], bitboard piece_occ_bb[2][6], bool turn, int castle_right, int en_passant, int king_pos[])
    //Utils::parse_fen_str(UCI::start_position, Move_Gen::player_occ_bb, Move_Gen::piece_occ_bb, Move_Gen::turn, Move_Gen::castle_right, Move_Gen::en_passant, Move_Gen::king_position);

    // // void print_mini_board(bitboard player_occ_bb[3], bitboard piece_occ_bb[2][6], bool turn, int castle_right, int en_passant)
    // Utils::print_mini_board(Move_Gen::player_occ_bb, Move_Gen::piece_occ_bb, Move_Gen::turn, Move_Gen::castle_right, Move_Gen::en_passant);

    // int depth = 0;

    // Move_Gen::generate_moves(depth);

    //Utils::print_bb(Move_Gen::pin_mask);

    // void print_attack_map(bitboard player_occ_bb[3], bitboard piece_occ_bb[2][6], const bool turn)
    //Utils::print_attack_map(Move_Gen::player_occ_bb, Move_Gen::piece_occ_bb, Move_Gen::turn);

    // for(int i=0; i<Move_Gen::move_count[depth]; i++)
    // {
    //     //Utils::print_move_info(Move_Gen::valid_moves[i]);
    //     //std::getchar();

    //     UndoPacked undo_info = Move_Gen::do_move(Move_Gen::valid_moves[depth][i]);
    //     Utils::print_mini_board(Move_Gen::player_occ_bb, Move_Gen::piece_occ_bb, Move_Gen::turn, Move_Gen::castle_right, Move_Gen::en_passant);
    //     std::getchar();

    //     Move_Gen::generate_moves(depth+1);
    //     for(int j=0; j<Move_Gen::move_count[depth+1]; j++)
    //     {
    //         UndoPacked undo_info = Move_Gen::do_move(Move_Gen::valid_moves[depth+1][j]);
    //         Utils::print_mini_board(Move_Gen::player_occ_bb, Move_Gen::piece_occ_bb, Move_Gen::turn, Move_Gen::castle_right, Move_Gen::en_passant);
    //         std::getchar();

    //         //Utils::print_move_info(Move_Gen::valid_moves[depth+1][j]);
    //         //std::getchar();

    //         Utils::print_bb(Move_Gen::check_mask);

    //         Move_Gen::undo_move(Move_Gen::valid_moves[depth+1][j], undo_info);
    //         Utils::print_mini_board(Move_Gen::player_occ_bb, Move_Gen::piece_occ_bb, Move_Gen::turn, Move_Gen::castle_right, Move_Gen::en_passant);
    //         std::getchar();
    //     }

    //     Move_Gen::undo_move(Move_Gen::valid_moves[depth][i], undo_info);
    //     Utils::print_mini_board(Move_Gen::player_occ_bb, Move_Gen::piece_occ_bb, Move_Gen::turn, Move_Gen::castle_right, Move_Gen::en_passant);
    //     std::getchar();
    // }
    //Utils::print_move_info(Move_Gen::valid_moves[i]);

    //printf("Generated Moves: %d", Move_Gen::move_count);




    //printf("Size: %lu", sizeof(Move_Gen::player_occ_bb));


    // int depth = 0;

    // auto t0 = steady_clock::now();
    // Search::perft_test(depth);
    // auto t1 = steady_clock::now();
    // auto dt = duration_cast<milliseconds>(t1 - t0).count();

    // printf("Search time: %lld ms\n", (long long)dt);
    // printf("Nodes Explored: %llu\n", Search::node_count);
    // printf("Nodes per second: %llu", (Search::node_count / dt));

    UCI::uci_loop();
    // for(int m=0; m<Move_Gen::move_count[0]; m++)
    // {
    //     Utils::print_move_info(Move_Gen::valid_moves[0][m]);
    // }




    return 0;
}