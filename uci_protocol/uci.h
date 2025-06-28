#pragma once
#include "../move/move_gen.h"
#include <iostream>
#include  <sstream>

namespace RedStone{

namespace UCI{

constexpr const char* ENGINE_NAME = "RedStone";
constexpr const char* ENGINE_AUTHOR = "donessie";
// Define the FEN start string:
const char start_position[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
const char tricky_position[] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
const char test_position[] = "2rr2k1/p4ppp/8/5P2/Qn4n1/2P5/PP3qPP/R1BK1B1R w - - 0 1";
const char test_position2[] = "2kr3r/1pp2Qp1/p4b1p/3B1p2/2Pn4/2qP4/P4PPP/4R1K1 b - - 0 1";

// when recieves "uci" token
inline void init_engine()
{
    // identify
    std::cout << "id name "   << ENGINE_NAME   << "\n";
    std::cout << "id author " << ENGINE_AUTHOR << "\n";
    // signal ready to switch to normal mode
    std::cout << "uciok\n";
}

//
inline void on_isready()
{
    //
    Tables::initialize_precomputed_tables();
    std::cout << "readyok\n";
}

inline void on_newgame()
{
    // reset your hash tables, history, etc.
    Utils::parse_fen_str(start_position, Move_Gen::player_occ_bb, Move_Gen::piece_occ_bb, Move_Gen::turn, Move_Gen::castle_right, Move_Gen::en_passant, Move_Gen::king_position);
    Move_Gen::generate_moves(0);
}

inline void on_position(const std::string& args)
{
    std::istringstream in(args);
    std::string token;
    // passes the first space/tab separated word and keeps in the stream buffer ("in") the rest of the string
    in >> token;

    if (token == "startpos")
        Utils::parse_fen_str(start_position, Move_Gen::player_occ_bb, Move_Gen::piece_occ_bb, Move_Gen::turn, Move_Gen::castle_right, Move_Gen::en_passant, Move_Gen::king_position);
    else if (token == "fen")
    {
        // FEN is 6 whitespace‐separated fields:
        // so we concatenate the string in "fen"
        std::string part, fen;
        for (int i = 0; i < 6; i++)
        {
            if (!(in >> part)) return;         // malformed
            fen += part + (i<5 ? " " : "");
        }
        // and build or position from the given "fen" string
        // fen.c_str() so it goes to a char* instead of string cuz thats hwo i declared my parse fucntion
        Utils::parse_fen_str(fen.c_str(), Move_Gen::player_occ_bb, Move_Gen::piece_occ_bb, Move_Gen::turn, Move_Gen::castle_right, Move_Gen::en_passant, Move_Gen::king_position);
    }
    else
    {
        // unknown form
        return;
    }

    // =============================================
    // DEBUG: ("isready" init the tables)
    // isready
    // position startpos moves e2e4 d7d5 b1c3


    // now see if there is a "moves" token
    if (in >> token && token == "moves")
    {
        std::string mv;
        while (in >> mv)
        {
            // generate moves for the 0 move_count etc (the one that keeps our position (the rest are for actual searching))
            Move_Gen::generate_moves(0);

            // we must parse the move here and then make it using the do_move fucntion
            int from_sq = Encoder::coord_to_square(mv.substr(0, 2));
            int to_sq = Encoder::coord_to_square(mv.substr(2, 2));
            Piece_Type promo = Empty;
            if (mv.size() == 5)
            {
                switch (mv[4])
                {
                    case 'n': promo = Knight; break;
                    case 'b': promo = Bishop; break;
                    case 'r': promo = Rook;   break;
                    case 'q': promo = Queen;  break;
                }
            }

            // lets traverse the move to look for the move is required from us to make
            for(auto &m: Move_Gen::move_list[0].moves)
            {
                // parse move
                int valid_source = Encoder::move_get_from(m);
                int valid_destination = Encoder::move_get_to(m);

                // if this move matches one of the valid moves
                if(valid_source == from_sq && valid_destination == to_sq)
                {
                    // if promotion not empty we must make sure the move we make is the exact promo move
                    if(promo != Empty)
                    {
                        Piece_Type valid_promotion = Encoder::move_get_promo_piece(m);
                        if(valid_promotion == promo)
                        {
                            Move_Gen::do_move(m);
                            // once we are done with the move no need to keep seacrhing the array of moves
                            break;
                        }
                        // if this is not the promotio we are looking for we keep seacrhing the valid moves array
                        else
                            continue;
                    }
                    // else promo is empty and move matches what we have so we make it and break from this loop
                    Move_Gen::do_move(m);
                    // once we are done with the move no need to keep seacrhing the array of moves
                    break;
                }
            }

        }
    }
}

inline void on_go(const std::string& args)
{
    // parse depth/movetime/etc., call our search, then:
    // std::string best = think(depth, movetime);
    // std::cout << "bestmove " << best << "\n";
    Move best = Search::find_best_move(6);

    // unpack
    int from = Encoder::move_get_from(best);
    int to   = Encoder::move_get_to(best);
    Piece_Type promo = Encoder::move_get_promo_piece(best);

    // encode the move into the necessary format
    std::string mv;
    mv += Encoder::square_to_coord[from];   // e.g. "e2"
    mv += Encoder::square_to_coord[to];     // e.g. "e4"
    if (promo != Empty) {
        // lowercase UCI promotion letter
        static const char p2c[6] = { ' ', 'n','b','r','q','k' };
        mv += p2c[(int)promo];
    }

    // output the move so the uci program gets it
    std::cout << "bestmove " << mv << "\n";
    std::cout << "evaluation " << Search::pos_eval << "\n";
    std::cout << "nodes " << Search::node_count << "\n";
    std::cout << "prunes " << Search::prune_count << "\n";
    Search::node_count=0;
    Search::prune_count=0;
    Search::pos_eval=0;
}

inline void on_stop()
{
    // if we support infinite/pondering, stop and print bestmove
}

inline void uci_loop()
{
    // UNCOMENT THIS IN RELEASE FOR FASTER I/O
    // speeds up std::cin and cout BUT we cant mix now printf()
    // takes away a bunch of overhead such as flushing of cout
    // automatically for interactive prompts (since we dont need that)
    // std::ios::sync_with_stdio(false);
    // std::cin.tie(nullptr);

    std::string line;
    // while there is something to get from the standard input
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;   // we get nothing we wait for something else

        // shortcut that creates an input stream called "in" that reads from the string "line"
        // allow us to use the stream operations (>>, getline, etc) on that string ("line") as if it ws coming from std::cin
        std::istringstream in(line);
        std::string cmd;

        // stream extractor operator (populates string "cmd" with up to the first space/tab)
        in >> cmd;

        if (cmd == "uci") {
        init_engine();
        }
        else if (cmd == "isready") {
        on_isready();
        }
        else if (cmd == "ucinewgame") {
        on_newgame();
        }
        else if (cmd == "position") {
        // passes the substring from ' ' (first space index) + 1 (to ignore the actual space) to the end of the string to "rest"
        std::string rest = line.substr(line.find(' ')+1);
        on_position(rest);
        }
        else if (cmd == "go") {
        std::string rest = line.substr(line.find(' ')+1);
        on_go(rest);
        }
        else if (cmd == "stop") {
        on_stop();
        }
        else if (cmd == "quit") {
        break;
        }
        // ignore unknown commands
        Utils::print_mini_board(Move_Gen::player_occ_bb, Move_Gen::piece_occ_bb, Move_Gen::turn, Move_Gen::castle_right, Move_Gen::en_passant);
    }
}

}   // end UCI namespace
}   // end RedStone namespace