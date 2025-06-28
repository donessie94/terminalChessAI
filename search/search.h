#pragma once
#include"../move/move_gen.h"


namespace RedStone{

namespace Search{

// fucntion forward declaration so Max can see it
static inline __attribute__((always_inline)) int alpha_beta_min(int alpha, int beta, int depth);

// used for custom max depth search
unsigned search_depth;

// 0 for white (+ infinity) and 1 for black (- infinity)
constexpr const int infinity[2] = { std::numeric_limits<int>::max(), std::numeric_limits<int>::lowest() };

unsigned long long node_count;
unsigned long long prune_count;
int pos_eval;

static inline __attribute__((always_inline)) int quiescence(int alpha, int beta)
{
    return 0;
}

static inline __attribute__((always_inline)) int static_evaluation(bool ab)
{
    using namespace Move_Gen;
    int test_depth = MAX_DEPTH - 1;
    int evaluation = 0;
    bool opp = !turn;

    // CHECKMATE AND STALEMATE ==================================================================
    // generate moves in the evry last table position (reserved for testing currently 35-1)
    // to test for checkmate and stalemate
    generate_moves(test_depth);

    // check mate
    if(num_attackers > 0)
    {
        if(move_list[test_depth].count == 0)
            return infinity[ab];
    }
    // stealmate
    else
    {
        if(move_list[test_depth].count == 0)
            return 0;
    }
    // ==========================================================================================

    for(int i=0; i<6; i++)
    {
        // This player
        Bitboard pieces_occ = piece_occ_bb[turn][i];
        while (pieces_occ)
        {
            int this_piece_sq = LS1B_IDX(pieces_occ);
            pieces_occ &= pieces_occ - 1;     // clear LS1B

            evaluation += Tables::PIECE_VALUE[i];
            evaluation += Tables::PST[i][this_piece_sq];
        }

        // Oponnent
        Bitboard opp_pieces_occ = piece_occ_bb[opp][i];
        while (opp_pieces_occ)
        {
            int this_piece_sq = LS1B_IDX(opp_pieces_occ);
            opp_pieces_occ &= opp_pieces_occ - 1;

            evaluation -= Tables::PIECE_VALUE[i];
            evaluation -= Tables::PST[i][this_piece_sq];
        }
    }

    //
    return evaluation;
}

// counts all final leafs at a given depth
static inline __attribute__((always_inline)) void perft_test(int depth)
{
    // break rule when we reach max depth
    if(depth == MAX_DEPTH)
    {
        node_count++;
        return;
    }

    Move_Gen::generate_moves(depth);

    for(int mv=0; mv<Move_Gen::move_list[depth].count; mv++)
    {
        UndoPacked undo_info = Move_Gen::do_move(Move_Gen::move_list[depth].moves[mv]);

        perft_test(depth+1);

        Move_Gen::undo_move(Move_Gen::move_list[depth].moves[mv], undo_info);
    }
}

static inline __attribute__((always_inline)) int alpha_beta_max(int alpha, int beta, int depth)
{
    // break rule when we reach max depth
    if(depth == search_depth)
    {
        // for insights on the prunning efficiency etc
        node_count++;
        // we must ensure first this is a quiescence position to have a meaningful evaluation
        // we must return evaluation here
        return static_evaluation(1);
    }


    // CHECKMATE AND STALEMATE ==================================================================
    // generate moves in the evry last table position (reserved for testing currently 35-1)
    // to test for checkmate and stalemate
    Move_Gen::generate_moves(depth);

    // check mate
    if(Move_Gen::num_attackers > 0)
    {
        if(Move_Gen::move_list[depth].count == 0)
            return std::numeric_limits<int>::lowest();
    }
    // stealmate
    else
    {
        if(Move_Gen::move_list[depth].count == 0)
            return 0;
    }
    // ==========================================================================================

    //  - infinity basically
    //int current_node_best = infinity[!Move_Gen::turn];
    int current_node_best = alpha; // start at the lower bound instead (same thing)

    // iterate all moves
    for(int mv=0; mv<Move_Gen::move_list[depth].count; mv++)
    {
        Move move = Move_Gen::move_list[depth].moves[mv];
        UndoPacked undo_info = Move_Gen::do_move(move);

        int next_node_evaluation = alpha_beta_min(alpha, beta, depth+1);

        Move_Gen::undo_move(move, undo_info);

        // MAX does cutoff for MIN and viceversa
        // we do a soft cuttof (equal positions are still traversed)
        if(next_node_evaluation >= beta)
        {
            prune_count++;
            return next_node_evaluation;
        }

        // when we get to the bottom (well 1 up from the bottom since bottom only evaluates)
        // we assign the next_node_evaluation (at the bottom is the leaf node eval basically) to current node best
        if( next_node_evaluation > current_node_best)
        {
            // if the above is true then we found a better move for MAX player so we assign it
            // as current depth node best move
            current_node_best = next_node_evaluation;

            // if this evaluation is also best that our current cut-off limit then we assign it as cut-off limit
            if(next_node_evaluation > alpha)
                alpha = next_node_evaluation;
        }
    }

    return current_node_best;
}

static inline __attribute__((always_inline)) int alpha_beta_min(int alpha, int beta, int depth)
{
    if(depth == search_depth)
    {
        node_count++;
        return static_evaluation(0);
    }
    Move_Gen::generate_moves(depth);

    if(Move_Gen::num_attackers > 0)
    {
        if(Move_Gen::move_list[depth].count == 0)
            return std::numeric_limits<int>::max();
    }
    else
    {
        if(Move_Gen::move_list[depth].count == 0)
            return 0;
    }

    //int current_node_best = infinity[!Move_Gen::turn];
    int current_node_best = beta;
    for(int mv=0; mv<Move_Gen::move_list[depth].count; mv++)
    {
        Move move = Move_Gen::move_list[depth].moves[mv];
        UndoPacked undo_info = Move_Gen::do_move(move);
        int next_node_evaluation = alpha_beta_max(alpha, beta, depth+1);
        Move_Gen::undo_move(move, undo_info);
        if(next_node_evaluation <= alpha)
        {
            prune_count++;
            return next_node_evaluation;
        }

        if(next_node_evaluation < current_node_best)
        {
            current_node_best = next_node_evaluation;
            if(next_node_evaluation < beta)
                beta = next_node_evaluation;
        }
    }
    return current_node_best;
}

static inline __attribute__((always_inline))
Move find_best_move(int max_depth)
{
    int alpha = std::numeric_limits<int>::lowest();
    int beta  = std::numeric_limits<int>::max();
    int current_node_best = alpha;                              // track the best score
    search_depth = max_depth;
    Move_Gen::generate_moves(0);
    Move best_move = Move_Gen::move_list[0].moves[0];

    //printf("Number of moves available: %d\n", Move_Gen::move_count[0]);

    // NOTE: MAX can prune when it knows a beta and Min can prune when it knows an alpha
    //
    // so by doing this we know the move and expand the next nodes and ensure they can prune right away
    // by updating alpha here manually on top node
    for (int i = 0; i < Move_Gen::move_list[0].count; ++i)
    {
        Move m = Move_Gen::move_list[0].moves[i];
        auto undo_info = Move_Gen::do_move(m);

        // call MIN on each root‐move
        int next_node_evaluation = alpha_beta_min(alpha, beta, 1);

        Move_Gen::undo_move(m, undo_info);

        // Debug: print every root‐move and its score
        // printf("Root move %2d: %c%c → %c%c  score=%d\n",
        //        i,
        //        'a' + (Encoder::move_get_from(m) & 7),
        //        '1' + (Encoder::move_get_from(m)>>3),
        //        'a' + (Encoder::move_get_to(m)&7),
        //        '1' + (Encoder::move_get_to(m)>>3),
        //        next_node_evaluation);

        // immediate mate detection
        // if (next_node_evaluation == infinity[ Move_Gen::turn ]) {
        //     pos_eval  = next_node_evaluation;
        //     return m;
        // }

        // update best score & move
        if (next_node_evaluation > current_node_best)
        {
            current_node_best   = next_node_evaluation;
            best_move           = m;
            alpha               = next_node_evaluation;  // tighten alpha for subsequent siblings
        }
        // (no beta‐cut here because beta is -infinity at the root)
    }
    pos_eval = current_node_best;
    return best_move;
}

}   // end Search namespace
}   // end RedStone namespace