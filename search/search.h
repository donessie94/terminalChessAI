#pragma once
#include"../move/move_gen.h"
#include <algorithm>
#include <iostream>

namespace RedStone{

namespace Search{

// “Black territory” is the top half (ranks 5–8), “White territory” the bottom half (ranks 1–4):
static constexpr Bitboard BLACK_TERRITORY = 0xFFFFFFFF00000000ULL;
static constexpr Bitboard WHITE_TERRITORY = 0x00000000FFFFFFFFULL;

static inline __attribute__((always_inline)) void print_PV()
{
    using namespace Encoder;
    std::cout << "PV ";
    for (int i = 0; i < principal_variation_length[0]; ++i)
    {
        Move m = principal_variation_move[0][i];
        std::cout << " " << square_to_coord[move_get_from(m)] << square_to_coord[move_get_to(m)];
    }
    std::cout << "\n";
}

// count how many “attack rays” of side c hit any square in the 5×5 zone around sq_king
static inline __attribute__((always_inline))
int zone_pressure(Color c, int sq_king, Bitboard occ) {
    using namespace Move_Gen;
    // grab the 5×5 zone mask for that king square
    Bitboard zone = Tables::KING_ZONE[sq_king];
    int pressure = 0;

    // --- pawn attacks ---
    for (Bitboard bb = piece_occ_bb[c][Pawn]; bb; bb &= bb-1) {
        int sq = LS1B_IDX(bb);
        // every pawn attack into the zone adds 1
        pressure += COUNT_BITS( GET_PAWN_ATTACK(c, sq) & zone );
    }

    // --- knight attacks ---
    for (Bitboard bb = piece_occ_bb[c][Knight]; bb; bb &= bb-1) {
        int sq = LS1B_IDX(bb);
        pressure += COUNT_BITS( GET_KNIGHT_ATTACK(sq) & zone );
    }

    // --- bishop attacks ---
    for (Bitboard bb = piece_occ_bb[c][Bishop]; bb; bb &= bb-1) {
        int sq = LS1B_IDX(bb);
        pressure += COUNT_BITS( GET_BISHOP_ATTACK(occ, sq) & zone );
    }

    // --- rook attacks ---
    for (Bitboard bb = piece_occ_bb[c][Rook]; bb; bb &= bb-1) {
        int sq = LS1B_IDX(bb);
        pressure += COUNT_BITS( GET_ROOK_ATTACK(occ, sq) & zone );
    }

    // --- queen attacks (rook ∪ bishop) ---
    for (Bitboard bb = piece_occ_bb[c][Queen]; bb; bb &= bb-1) {
        int sq = LS1B_IDX(bb);
        Bitboard atk = GET_BISHOP_ATTACK(occ, sq)
                     | GET_ROOK_ATTACK(occ,   sq);
        pressure += COUNT_BITS( atk & zone );
    }

    return pressure;
}

// Helper to build an “attack mask” for a given side
static inline __attribute__((always_inline)) Bitboard compute_attack_mask(Color c, Bitboard occ) {
    using namespace Move_Gen;
    Bitboard attacks = 0ULL;

    // pawn attacks
    if (c == white) {
      for (Bitboard bb = piece_occ_bb[c][Pawn]; bb; bb &= bb-1) {
        int sq = LS1B_IDX(bb);
        attacks |= GET_PAWN_ATTACK(c, sq);
      }
    } else {
      for (Bitboard bb = piece_occ_bb[c][Pawn]; bb; bb &= bb-1) {
        int sq = LS1B_IDX(bb);
        attacks |= GET_PAWN_ATTACK(c, sq);
      }
    }

    // knight attacks
    for (Bitboard bb = piece_occ_bb[c][Knight]; bb; bb &= bb-1) {
      int sq = LS1B_IDX(bb);
      attacks |= GET_KNIGHT_ATTACK(sq);
    }

    // // king attacks
    // for (Bitboard bb = piece_occ_bb[c][King]; bb; bb &= bb-1) {
    //   int sq = LS1B_IDX(bb);
    //   attacks |= GET_KING_ATTACK(sq);
    // }

    // sliding pieces
    for (Bitboard bb = piece_occ_bb[c][Bishop]; bb; bb &= bb-1) {
      int sq = LS1B_IDX(bb);
      attacks |= GET_BISHOP_ATTACK(occ, sq);
    }
    for (Bitboard bb = piece_occ_bb[c][Rook]; bb; bb &= bb-1) {
      int sq = LS1B_IDX(bb);
      attacks |= GET_ROOK_ATTACK(occ, sq);
    }
    for (Bitboard bb = piece_occ_bb[c][Queen]; bb; bb &= bb-1) {
      int sq = LS1B_IDX(bb);
      attacks |= GET_BISHOP_ATTACK(occ, sq)
              |  GET_ROOK_ATTACK(   occ, sq);
    }

    return attacks;
}

// fucntion forward declaration so Max can see it
static inline __attribute__((always_inline)) int alpha_beta_min(int alpha, int beta, int depth);
static inline __attribute__((always_inline)) int quiescence_min(int alpha, int beta, int depth);
static inline __attribute__((always_inline)) int static_evaluation(bool is_max, int depth);

// used for custom max depth search
unsigned search_depth;

// 0 for white (+ infinity) and 1 for black (- infinity)
constexpr const int infinity[2] = { 20000, -20000 };

unsigned long long node_count;
unsigned long long prune_count;
int pos_eval;
constexpr const int PRESSURE_WEIGHT = 5;

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

    // prevents infinity checks loop
    // if (depth >= search_depth + 2)
    //     return static_evaluation(1, depth);

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
    Move_Gen::sort_moves_by_score(depth);

    //  - infinity basically
    //int current_node_best = infinity[!Move_Gen::turn];
    int current_node_best = alpha; // start at the lower bound instead (same thing)

    // lets divide this into in check or not in check
    // we allow all valid moves "in check"
    // NOTE we MUST check the whole attack line, we must allow "check covers moves" and "king escapes moves"
    // the down side is that a a "better position" close to mate (lets say in 2 more moves) is not accounted for
    // if it involves quiet moves in between
    // if(Move_Gen::num_attackers > 0)
    // {
    //     // iterate all moves (we only generate valid moves so all possible moves are going to protect the check)
    //     for(int mv=0; mv<Move_Gen::move_list[depth].count; mv++)
    //     {
    //         Move move = Move_Gen::move_list[depth].moves[mv];

    //         UndoPacked undo_info = Move_Gen::do_move(move);

    //         int next_node_evaluation = quiescence_min(alpha, beta, depth+1);

    //         Move_Gen::undo_move(move, undo_info);

    //         // MAX does cutoff for MIN and viceversa
    //         // we do a soft cuttof (equal positions are still traversed)
    //         if(next_node_evaluation >= beta)                                                // SOFT/HARD CUT-OFFF
    //         {
    //             prune_count++;
    //             return next_node_evaluation;
    //         }

    //         // when we get to the bottom (well 1 up from the bottom since bottom only evaluates)
    //         // we assign the next_node_evaluation (at the bottom is the leaf node eval basically) to current node best
    //         if( next_node_evaluation > current_node_best)
    //         {
    //             // if the above is true then we found a better move for MAX player so we assign it
    //             // as current depth node best move
    //             current_node_best = next_node_evaluation;

    //             // if this evaluation is also best that our current cut-off limit then we assign it as cut-off limit
    //             if(next_node_evaluation >= alpha)
    //                 alpha = next_node_evaluation;
    //         }
    //     }
    // }
    // // non check state -> we only generate checks, promos, captures
    // else
    {
        // iterate only captures/promotion/check moves to find a stable position where a static evluation is meaningful
        for(int mv=0; mv<Move_Gen::move_list[depth].count; mv++)
        {
            Move move = Move_Gen::move_list[depth].moves[mv];
            UndoPacked undo_info;
            if  ( ( Encoder::move_get_promo_piece(move) != Empty
                    || Encoder::move_get_captured_piece(move) != Empty
                    )
                )
            {
                    undo_info = Move_Gen::do_move(move);
            }
            else
            {
                continue;
            }


            // ONLY CAPTURES, CHECK, PROMOTIONS CHECK_BLOCKS, KING_EVASIONS ==========================================================================================
            // note this wont mess up anything since we aready used to generate this depth valid moves
            // so this information is not needed on this dpeth anymore (we already have all the moves)
            // Move_Gen::generate_check_mask();
            // // if the move is not a check, or promotion, or capture we ignore it
            // if  ( !( Encoder::move_get_promo_piece(move) != Empty
            //         || Encoder::move_get_captured_piece(move) != Empty
            //         || Move_Gen::num_attackers > 0  // we ensure here move is check or not
            //         )
            //     )
            // {
            //     Move_Gen::undo_move(move, undo_info);
            //     continue;
            // }
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
                {
                    alpha = next_node_evaluation;
                }

            }
        }
    }
    //
    return current_node_best;
}

static inline __attribute__((always_inline)) int quiescence_min(int alpha, int beta, int depth)
{   //beta +infinity starts
    node_count++;
    // if (depth >= search_depth + 2)
    //     return static_evaluation(0, depth);
    int initial_evaluation = static_evaluation(0, depth);
    if(initial_evaluation<beta)
        beta = initial_evaluation;
    if(initial_evaluation <= alpha)                                                   // SOFT/HARD CUT-OFFF
    {
        prune_count++;
        return initial_evaluation;
    }
    Move_Gen::sort_moves_by_score(depth);
    int current_node_best = beta;
    // if(Move_Gen::num_attackers > 0)
    // {
    //     for(int mv=0; mv<Move_Gen::move_list[depth].count; mv++)
    //     {
    //         Move move = Move_Gen::move_list[depth].moves[mv];
    //         UndoPacked undo_info = Move_Gen::do_move(move);
    //         int next_node_evaluation = quiescence_max(alpha, beta, depth+1);
    //         Move_Gen::undo_move(move, undo_info);
    //         if(next_node_evaluation <= alpha)                                            // SOFT/HARD CUT-OFFF
    //         {
    //             prune_count++;
    //             return next_node_evaluation;
    //         }
    //         if(next_node_evaluation < current_node_best)
    //         {
    //             current_node_best = next_node_evaluation;
    //             if(next_node_evaluation < beta)
    //                 beta = next_node_evaluation;
    //         }
    //     }
    // }
    // else
    {
        for(int mv=0; mv<Move_Gen::move_list[depth].count; mv++)
        {
            Move move = Move_Gen::move_list[depth].moves[mv];
            UndoPacked undo_info;
            if  ( ( Encoder::move_get_promo_piece(move) != Empty
                    || Encoder::move_get_captured_piece(move) != Empty
                    )
                )
            {
                undo_info = Move_Gen::do_move(move);
            }
            else
            {
                continue;
            }

            // Move_Gen::generate_check_mask();
            // if  ( !( Encoder::move_get_promo_piece(move) != Empty
            //         || Encoder::move_get_captured_piece(move) != Empty
            //         || Move_Gen::num_attackers > 0  // we ensure here move is check or not
            //         )
            //     )
            // {
            //     Move_Gen::undo_move(move, undo_info);
            //     continue;
            // }
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

static inline __attribute__((always_inline))
int static_evaluation(bool is_max, int depth) {
    using namespace Move_Gen;

    // --- detect mate/stalemate ---
    generate_moves(depth);
    if (num_attackers > 0 && move_list[depth].count == 0)
    {
        //printf("checkmate dept: %d\n", depth);
        // NOTE: without -depth we would treat all mates equal, so does not matter which we go for it
        // and we most likely will end up chasing different mates each times we do a new search, wich will
        // end up not mating the opponent at all cuz each time is looking for a new mate line instead of sticking to one
        int sign = is_max ? -1 : +1;
        return sign * (infinity[is_max] - depth);
    }

    if (num_attackers == 0 && move_list[depth].count == 0)
        return 0;

    // --- pinned-piece masks ---
    //generate_pin_mask();                 // fills pin_mask for side-to-move’s king
    // what about checks? i need to account that somehwo tho, because no checks maye have no pins but be worse position
    Bitboard my_pin_mask = pin_mask;

    generate_opposite_king_mask();       // now fills pin_mask for the *other* king
    Bitboard opp_pin_mask = pin_mask;

    // --- material + PST (White minus Black) ---
    int raw = 0;
    for (int pt = Pawn; pt <= King; ++pt) {
        for (Bitboard w = piece_occ_bb[white][pt]; w; w &= w-1) {
            int sq = LS1B_IDX(w);
            raw += Tables::PIECE_VALUE[pt] + Tables::PST[pt][sq];
        }
        for (Bitboard b = piece_occ_bb[black][pt]; b; b &= b-1) {
            int sq = LS1B_IDX(b);
            raw -= Tables::PIECE_VALUE[pt]
                 + Tables::PST[pt][ Tables::mirror_square(sq) ];
        }
    }

    // --- pinned-piece penalty (enemy pins minus my pins) ---
    raw += 20 * (COUNT_BITS(opp_pin_mask) - COUNT_BITS(my_pin_mask));

    // --- build occupancy ---
    Bitboard occ = player_occ_bb[all_color];

    // --- space ---
    Bitboard w_space = compute_attack_mask(white, occ) & ~player_occ_bb[white];
    Bitboard b_space = compute_attack_mask(black, occ) & ~player_occ_bb[black];

    // --- global raw space score ---
    raw += (1 * (COUNT_BITS(w_space) - COUNT_BITS(b_space)));

    // how many of Black’s squares does White control?
    int w_terr_ctrl = COUNT_BITS(w_space & BLACK_TERRITORY);

    // how many of White’s squares does Black control?
    int b_terr_ctrl = COUNT_BITS(b_space & WHITE_TERRITORY);

    // and evaluate how many enemy territory squares we control
    raw += 1 * (w_terr_ctrl - b_terr_ctrl);

    // not super happy still in how this works tho, i would like to make sure. know the single square 2, 3 pieces attack
    int w_pressure = zone_pressure(white, king_position[black], occ);
    int b_pressure = zone_pressure(black, king_position[white], occ);

    raw += (PRESSURE_WEIGHT*(w_pressure - b_pressure));

    //printf("Depth: %d\n", depth);

    return raw;
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
// NOTE: Alpha is set in MAX and that value is used for his childs only (unless MIN picks
// this value as its BETA on top of it, where then this value becomes the Beta of the parent)
// And its the same thing for Beta
// so Alpha trasnform into Beta and viceversa when its picked by the parent (when is bets for the parent)
static inline __attribute__((always_inline)) int alpha_beta_max(int alpha, int beta, int depth)
{
    // break rule when we reach max depth
    if(depth == search_depth)
    {
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
        {
            return (-1) * (infinity[0] - depth);
        }
    }
    // stealmate
    else
    {
        if(Move_Gen::move_list[depth].count == 0)
            return 0;
    }
    // ==========================================================================================


    // Sort the list of moves by MVV/LVA table
    Move_Gen::sort_moves_by_score(depth);

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
        // we do a soft cuttof (equal positions are not traversed)
        if(next_node_evaluation >= beta)                                                // SOFT/HARD CUT-OFFF
        {
            // Killer move potential for Whites (MAX), since it forced MIN to avoid this line of search cuz it has something way better elsewhere,
            // basically this move (well technically the position reached after this move) led to a worst position for Black pieces
            //
            // we consider this killer moves because the "best move" or a "very good move" may be the same for many different responses to enemy
            // moves from the same starting position (represented by the depth) example: imagine we have a starting position (the position at depth)
            // where is mate in 2 for whites after a single pawn push, black may have 3 different captures, queen, knight rook, but
            // we have mate in 2 no matter what after the pawn push (the "killer move") is played by us no matter what black does
            //
            // we want to make sure the killer move is non capture non promo since those are already sorted
            // recall we storing the move cuz it may be good or best in response to other moves (at same depth) of the enemy pieces
            // record killer if non‐capture and not already present
            if (Encoder::move_get_captured_piece(move) == Empty && Encoder::move_get_promo_piece(move) == Empty)
            {
                if (move != Encoder::max_killer[0][depth])
                {
                    Encoder::max_killer[1][depth] = Encoder::max_killer[0][depth];
                    Encoder::max_killer[0][depth] = move;
                }

                // Historic moves that has shown in a loosely similar position and depths that are good (forced a cuttof)
                // so historic moves are moves that even out of context (like not same depth or position, but similar) has
                // been good enough to force a prune, so they MAY be a good option to try in our search if possible, this is the idea
                // of course this is the last resource, order of strong moves to try is:
                // captures -> promos -> killer -> historic -> quiet, NOTE: i still need to hook checks somehow in my ordering (and check i think is the top priority)
                //
                // we musut zero it at the start of each new find best move search tho (so old searchs don’t bleed in)
                // note this is not preserved trhought the game only we do this per search of best move
                // Encoder::max_history_move_score[Encoder::move_get_from(move)][Encoder::move_get_to(move)] += 1;
            }
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
            {
                alpha = next_node_evaluation;

                // this is the principal variation we have found basically (alpha for Max and beta for Min)
                // here we record this move at the right depth spot
                Encoder::principal_variation_move[depth][0] = move;

                // copy the rest of the child’s PV into ours
                Encoder::principal_variation_length[depth] = 1 + Encoder::principal_variation_length[depth+1];
                for (int j = 0; j < Encoder::principal_variation_length[depth+1]; ++j)
                    Encoder::principal_variation_move[depth][1+j] = Encoder::principal_variation_move[depth+1][j];
            }
        }
    }

    return current_node_best;
}

// Black Pieces
static inline __attribute__((always_inline)) int alpha_beta_min(int alpha, int beta, int depth)
{
    if(depth == search_depth)
    {
        return quiescence_min(alpha, beta, depth);
    }
    Move_Gen::generate_moves(depth);

    if(Move_Gen::num_attackers > 0)
    {
        if(Move_Gen::move_list[depth].count == 0)
        {
            return (infinity[0] - depth);
        }

    }
    else
    {
        if(Move_Gen::move_list[depth].count == 0)
            return 0;
    }
    // Sort the list of moves by MVV/LVA table
    Move_Gen::sort_moves_by_score(depth);
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
            if (Encoder::move_get_captured_piece(move) == Empty && Encoder::move_get_promo_piece(move) == Empty)
            {
                if (move != Encoder::min_killer[0][depth])
                {
                    Encoder::min_killer[1][depth] = Encoder::min_killer[0][depth];
                    Encoder::min_killer[0][depth] = move;
                }
                // Encoder::min_history_move_score[Encoder::move_get_from(move)][Encoder::move_get_to(move)] += 1;
            }
            prune_count++;
            return next_node_evaluation;
        }
        if(next_node_evaluation < current_node_best)
        {
            current_node_best = next_node_evaluation;
            if(next_node_evaluation < beta)
            {
                beta = next_node_evaluation;
                Encoder::principal_variation_move[depth][0] = move;
                Encoder::principal_variation_length[depth] = 1 + Encoder::principal_variation_length[depth+1];
                for (int j = 0; j < Encoder::principal_variation_length[depth+1]; ++j)
                    Encoder::principal_variation_move[depth][1+j] = Encoder::principal_variation_move[depth+1][j];
            }
        }
    }
    return current_node_best;
}

static inline __attribute__((always_inline))
Move find_best_move_white(int max_depth)
{
    using namespace Encoder;
    // This one i cant zero out but i must overrite from back to front so i can use it in my gets_score_move function() before creating the new iteration one
    //std::fill(principal_variation_length, principal_variation_length + max_depth + 1, 0);

    // zero both tables before each new root search
    // std::memset(max_history_move_score, 0, sizeof(max_history_move_score));
    // std::memset(min_history_move_score, 0, sizeof(min_history_move_score));

    int alpha = std::numeric_limits<int>::lowest();
    int beta  = std::numeric_limits<int>::max();
    int current_node_best = alpha;  // we want the highest score

    search_depth = max_depth;

    // generate and sort White’s root moves (Max = true, depth = 0)
    Move_Gen::generate_moves(0);
    auto &ML = Move_Gen::move_list[0];
    Move best_move = ML.moves[0];
    std::sort(
      ML.moves, ML.moves + ML.count,
      [&](Move a, Move b)
      {
        return move_get_score(a, 0, /*is_max=*/true)
             > move_get_score(b, 0, /*is_max=*/true);
      }
    );

    // NOTE: MAX can prune when it knows a beta and Min can prune when it knows an alpha
    //
    // so by doing this we know the move and expand the next nodes and ensure they can prune right away
    // by updating alpha here manually on top node
    // now search down each root move
    for (int i = 0; i < ML.count; ++i)
    {
        Move m = ML.moves[i];
        auto undo_info = Move_Gen::do_move(m);

        // after White’s move, Black to play → a MIN node
        int next_eval = alpha_beta_min(alpha, beta, /*depth=*/1);

        Move_Gen::undo_move(m, undo_info);

        if (next_eval > current_node_best)
        {
            current_node_best = next_eval;
            best_move         = m;
            alpha             = next_eval;  // tighten α

            beta = alpha+1;//

            // copy the PV from ply 1 into pv_table[0]
            principal_variation_move[0][0] = m;
            principal_variation_length[0]   = 1 + principal_variation_length[1];
            for (int j = 0; j < principal_variation_length[1]; ++j)
                principal_variation_move[0][1+j] = principal_variation_move[1][j];
        }
        // no β‐cut at root
    }

    pos_eval = current_node_best;
    return best_move;
}

static inline __attribute__((always_inline))
Move find_best_move_black(int max_depth)
{
    using namespace Encoder;
    //
    //std::fill(principal_variation_length, principal_variation_length + max_depth + 1, 0);

    // std::memset(max_history_move_score, 0, sizeof (max_history_move_score));
    // std::memset(min_history_move_score, 0, sizeof (min_history_move_score));
    int alpha = std::numeric_limits<int>::lowest();
    int beta  = std::numeric_limits<int>::max();
    int current_node_best = beta;  // we want the lowest score

    search_depth = max_depth;

    // generate and sort Black’s root moves (Max = false, depth = 0)
    Move_Gen::generate_moves(0);
    auto &ML = Move_Gen::move_list[0];
    Move best_move = ML.moves[0];
    std::sort(
      ML.moves, ML.moves + ML.count,
      [&](Move a, Move b)
      {
        // still sort descending by move_get_score, but is_max=false
        return move_get_score(a, 0, /*is_max=*/false)
             > move_get_score(b, 0, /*is_max=*/false);
      }
    );

    // now search down each root move
    for (int i = 0; i < ML.count; ++i)
    {
        Move m = ML.moves[i];
        auto undo_info = Move_Gen::do_move(m);

        // after Black’s move, White to play → a MAX node
        int next_eval = alpha_beta_max(alpha, beta, /*depth=*/1);

        Move_Gen::undo_move(m, undo_info);

        if (next_eval < current_node_best)
        {
            current_node_best = next_eval;
            best_move         = m;
            beta              = next_eval;  // tighten β
            alpha = beta-1;//
            principal_variation_move[0][0] = m;
            principal_variation_length[0]   = 1 + principal_variation_length[1];
            for (int j = 0; j < principal_variation_length[1]; ++j)
                principal_variation_move[0][1+j] = principal_variation_move[1][j];
        }
        // no α‐cut at root
    }

    pos_eval = current_node_best;
    return best_move;
}

static inline __attribute__((always_inline))
Move iterative_deepen(bool white_to_move, int max_depth)
{
    std::fill(Encoder::principal_variation_length, Encoder::principal_variation_length + max_depth + 1, 0);
    std::memset(Encoder::max_killer, 0, sizeof(Encoder::max_killer));
    std::memset(Encoder::min_killer, 0, sizeof(Encoder::min_killer));
    Move best = 0;
    for (int d = 1; d <= max_depth; ++d) {
        if (white_to_move) {
            best = find_best_move_white(d);
        } else {
            best = find_best_move_black(d);
        }
    }
    return best;
}

}   // end Search namespace
}   // end RedStone namespace