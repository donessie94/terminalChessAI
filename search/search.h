#pragma once
#include"../move/move_gen.h"
#include <algorithm>

namespace RedStone{

namespace Search{

bool engine_color;

static inline __attribute__((always_inline))
void sort_moves_by_score(int depth) {
    auto &ML = Move_Gen::move_list[depth];
    std::sort(
      ML.moves,
      ML.moves + ML.count,
      [](Move a, Move b) {
        return Encoder::move_get_score(a) > Encoder::move_get_score(b);
      }
    );
}

// fucntion forward declaration so Max can see it
static inline __attribute__((always_inline)) int alpha_beta_min(int alpha, int beta, int depth);
static inline __attribute__((always_inline)) int quiescence_min(int alpha, int beta, int depth);
static inline __attribute__((always_inline)) int static_evaluation(bool ab, int depth);

// used for custom max depth search
unsigned search_depth;

// 0 for white (+ infinity) and 1 for black (- infinity)
constexpr const int infinity[2] = { std::numeric_limits<int>::max(), std::numeric_limits<int>::lowest() };

unsigned long long node_count;
unsigned long long prune_count;
int pos_eval;

// wait i am missing here covers of the check or king evasions too because else an attack
// could be not explored (since it will stop middle attack when enemy cant capture or check back)
// ANOTHER IDEA: i eventually need to allow lets say 2-3 quiet moves to check for mates? (but this needs another search, so what to do about it? idk)
//
// THIS IMPLEMENTATION:
// The idea here is: enemy plays last (even depth), and then on my turn i am just allowed to
// capture/promo/check/king ecape/check_block then he is just allowed to do
// same etc til stable position is reached
static inline __attribute__((always_inline)) int quiescence_max(int alpha, int beta, int depth)
{
    node_count++;
    // this already generates all moves for us and check for checkmate and stalemate
    int initial_evaluation = static_evaluation(1, depth);

    //makes sure we have an alpha to compare against (if moves from here are bad we dont want to take them, example capturing a pawn with a queen)
    if(initial_evaluation>alpha)
        alpha = initial_evaluation;
    if (initial_evaluation >= beta)     // SOFT/HARD cutoff
    {
        prune_count++;
        return initial_evaluation;
    }


    // Sort the list of moves by MVV/LVA table
    sort_moves_by_score(depth);

    //  - infinity basically
    //int current_node_best = infinity[!Move_Gen::turn];
    int current_node_best = alpha; // start at the lower bound instead (same thing)

    // lets divide this into in check or not in check
    // we allow all valid moves "in check"
    // NOTE we MUST check the whole attack line, we must allow "check covers moves" and "king escapes moves"
    // the down side is that a a "better position" close to mate (lets say in 2 more moves) is not accounted for
    // if it involves quiet moves in between
    if(Move_Gen::num_attackers > 0)
    {
        // iterate all moves (we only generate valid moves so all possible moves are going to protect the check)
        for(int mv=0; mv<Move_Gen::move_list[depth].count; mv++)
        {
            Move move = Move_Gen::move_list[depth].moves[mv];

            UndoPacked undo_info = Move_Gen::do_move(move);

            int next_node_evaluation = quiescence_min(alpha, beta, depth+1);

            Move_Gen::undo_move(move, undo_info);

            // MAX does cutoff for MIN and viceversa
            // we do a soft cuttof (equal positions are still traversed)
            if(next_node_evaluation >= beta)                                                // SOFT/HARD CUT-OFFF
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
    }
    // non check state -> we only generate checks, promos, captures
    else
    {
        // iterate only captures/promotion/check moves to find a stable position where a static evluation is meaningful
        for(int mv=0; mv<Move_Gen::move_list[depth].count; mv++)
        {
            Move move = Move_Gen::move_list[depth].moves[mv];

            UndoPacked undo_info = Move_Gen::do_move(move);

            // ONLY CAPTURES, CHECK, PROMOTIONS CHECK_BLOCKS, KING_EVASIONS ==========================================================================================
            // note this wont mess up anything since we aready used to generate this depth valid moves
            // so this information is not needed on this dpeth anymore (we already have all the moves)
            Move_Gen::generate_check_mask();
            // if the move is not a check, or promotion, or capture we ignore it
            if  ( !( Encoder::move_get_promo_piece(move) != Empty
                    || Encoder::move_get_captured_piece(move) != Empty
                    || Move_Gen::num_attackers > 0  // we ensure here move is check or not
                    )
                )
            {
                Move_Gen::undo_move(move, undo_info);
                continue;
            }
            // else we continue our search

            // =====================================================================================================================================================

            int next_node_evaluation = quiescence_min(alpha, beta, depth+1);

            Move_Gen::undo_move(move, undo_info);

            // MAX does cutoff for MIN and viceversa
            // we do a soft cuttof (equal positions are still traversed)
            if(next_node_evaluation >= beta)                                                // SOFT/HARD CUT-OFFF
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
    }
    //
    return current_node_best;
}

static inline __attribute__((always_inline)) int quiescence_min(int alpha, int beta, int depth)
{   //beta +infinity starts
    node_count++;
    int initial_evaluation = static_evaluation(0, depth);
    if(initial_evaluation<beta)
        beta = initial_evaluation;
    if(initial_evaluation<=alpha)                                                   // SOFT/HARD CUT-OFFF
    {
        prune_count++;
        return initial_evaluation;
    }
    sort_moves_by_score(depth);
    int current_node_best = beta;
    if(Move_Gen::num_attackers > 0)
    {
        for(int mv=0; mv<Move_Gen::move_list[depth].count; mv++)
        {
            Move move = Move_Gen::move_list[depth].moves[mv];
            UndoPacked undo_info = Move_Gen::do_move(move);
            int next_node_evaluation = quiescence_max(alpha, beta, depth+1);
            Move_Gen::undo_move(move, undo_info);
            if(next_node_evaluation <= alpha)                                            // SOFT/HARD CUT-OFFF
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
    }
    else
    {
        for(int mv=0; mv<Move_Gen::move_list[depth].count; mv++)
        {
            Move move = Move_Gen::move_list[depth].moves[mv];
            UndoPacked undo_info = Move_Gen::do_move(move);
            Move_Gen::generate_check_mask();
            if  ( !( Encoder::move_get_promo_piece(move) != Empty
                    || Encoder::move_get_captured_piece(move) != Empty
                    || Move_Gen::num_attackers > 0  // we ensure here move is check or not
                    )
                )
            {
                Move_Gen::undo_move(move, undo_info);
                continue;
            }
            int next_node_evaluation = quiescence_max(alpha, beta, depth+1);
            Move_Gen::undo_move(move, undo_info);
            if(next_node_evaluation <= alpha)                                            // SOFT/HARD CUT-OFFF
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
    }
    return current_node_best;
}

// for each slider is looking at the enemy king +50 points
static inline __attribute__((always_inline))
int static_evaluation(bool is_max, int depth) {
    using namespace Move_Gen;

    // generate moves so we can detect mate / stalemate
    generate_moves(depth);
    bool in_check = (num_attackers > 0);
    int n_moves   = move_list[depth].count;

    // terminal positions
    if (in_check && n_moves == 0)    // checkmate for side to move
        return infinity[is_max];
    if (!in_check && n_moves == 0)   // stalemate
        return 0;

    // material + PST in “White minus Black” terms
    int eval = 0;
    for (int pt = Pawn; pt <= King; ++pt) {
        // White
        Bitboard wbb = piece_occ_bb[white][pt];
        while (wbb) {
            int sq = LS1B_IDX(wbb);
            wbb &= wbb - 1;
            eval += Tables::PIECE_VALUE[pt];
            eval += Tables::PST[pt][sq];
        }
        // Black
        Bitboard bbb = piece_occ_bb[black][pt];
        while (bbb) {
            int sq = LS1B_IDX(bbb);
            bbb &= bbb - 1;
            eval -= Tables::PIECE_VALUE[pt];
            // mirror so Black’s PST is viewed from White’s side
            eval -= Tables::PST[pt][ Tables::mirror_square(sq) ];
        }
    }
    return eval;
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

// White pieces
static inline __attribute__((always_inline)) int alpha_beta_max(int alpha, int beta, int depth)
{
    // break rule when we reach max depth
    if(depth == search_depth)
    {
        // for insights on the prunning efficiency etc
        //node_count++;
        // we must ensure first this is a quiescence position to have a meaningful evaluation
        // we must return evaluation here
        //return static_evaluation(1, depth);
        return quiescence_max(alpha, beta, depth);
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


    // Sort the list of moves by MVV/LVA table
    sort_moves_by_score(depth);

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
        if(next_node_evaluation >= beta)                                                // SOFT/HARD CUT-OFFF
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

// Black Pieces
static inline __attribute__((always_inline)) int alpha_beta_min(int alpha, int beta, int depth)
{
    if(depth == search_depth)
    {
        //node_count++;
        //return static_evaluation(0, depth);
        return quiescence_min(alpha, beta, depth);
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
    // Sort the list of moves by MVV/LVA table
    sort_moves_by_score(depth);
    //int current_node_best = infinity[!Move_Gen::turn];
    int current_node_best = beta;
    for(int mv=0; mv<Move_Gen::move_list[depth].count; mv++)
    {
        Move move = Move_Gen::move_list[depth].moves[mv];
        UndoPacked undo_info = Move_Gen::do_move(move);
        int next_node_evaluation = alpha_beta_max(alpha, beta, depth+1);
        Move_Gen::undo_move(move, undo_info);
        if(next_node_evaluation <= alpha)                                            // SOFT/HARD CUT-OFFF
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
Move find_best_move_white(int max_depth)
{
    int alpha = std::numeric_limits<int>::lowest();
    int beta  = std::numeric_limits<int>::max();
    int current_node_best = alpha;                              // track the best score
    search_depth = max_depth;
    Move_Gen::generate_moves(0);
    Move best_move = Move_Gen::move_list[0].moves[0];
    // Sort the list of moves by MVV/LVA table
    sort_moves_by_score(0);

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

static inline __attribute__((always_inline))
Move find_best_move_black(int max_depth)
{
    // initial α/β
    int alpha = std::numeric_limits<int>::lowest();
    int beta  = std::numeric_limits<int>::max();
    // for Black we’re looking for the *lowest* score
    int current_node_best = beta;

    search_depth = max_depth;

    // generate and sort Black’s root moves
    Move_Gen::generate_moves(0);
    auto &ML = Move_Gen::move_list[0];
    Move best_move = ML.moves[0];
    std::sort(ML.moves, ML.moves + ML.count, [](Move a, Move b) {
      return Encoder::move_get_score(a) > Encoder::move_get_score(b);
    });

    for (int i = 0; i < ML.count; ++i) {
        Move m = ML.moves[i];
        auto undo_info = Move_Gen::do_move(m);

        // after Black’s move, White to play → a MAX node
        int next_node_evaluation = alpha_beta_max(alpha, beta, 1);

        Move_Gen::undo_move(m, undo_info);

        // update best (i.e. *lowest*) score & move
        if (next_node_evaluation < current_node_best) {
            current_node_best = next_node_evaluation;
            best_move         = m;
            beta              = next_node_evaluation;  // tighten β
        }
        // no α‐cut here, since α starts at –∞ at the root
    }

    pos_eval = current_node_best;
    return best_move;
}

}   // end Search namespace
}   // end RedStone namespace