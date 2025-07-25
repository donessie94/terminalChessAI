#include"utils/misc.h"
#include"search/search.h"
#include"uci_protocol/uci.h"

using namespace RedStone;

int main()
{
    // Tables::initialize_precomputed_tables();

    // //void parse_fen_str(const char fen[], bitboard player_occ_bb[3], bitboard piece_occ_bb[2][6], bool turn, int castle_right, int en_passant, int king_pos[])
    // Utils::parse_fen_str("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
    //                     Move_Gen::player_occ_bb, Move_Gen::piece_occ_bb, Move_Gen::turn, Move_Gen::castle_right, Move_Gen::en_passant, Move_Gen::king_position);

    // UCI::on_go("");

    // using namespace std::chrono;
    // auto t0 = steady_clock::now();
    // Search::perft_test(0);
    // auto t1 = steady_clock::now();
    // auto dt = duration_cast<milliseconds>(t1 - t0).count();
    // std::cout << "nodes " << Search::node_count << "\n";
    // std::cout << "prunes " << Search::prune_count << "\n";
    // printf("Search time: %lld ms\n", (long long)dt);
    // printf("Nodes per milisecond: %llu", (Search::node_count / dt));

    //===

    UCI::uci_loop();

    return 0;
}