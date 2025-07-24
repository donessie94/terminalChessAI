#pragma once
#include"../move/move_gen.h"
#include <algorithm>
#include <iostream>

namespace RedStone{

namespace Search{

#define INF 20000

// “Black territory” is the top half (ranks 5–8), “White territory” the bottom half (ranks 1–4):
static constexpr Bitboard BLACK_TERRITORY = 0xFFFFFFFF00000000ULL;
static constexpr Bitboard WHITE_TERRITORY = 0x00000000FFFFFFFFULL;
// used for custom max depth search
unsigned search_depth;

// now takes depth and is_max‐node so it can look up the right killer array
static inline __attribute__((always_inline)) uint16_t move_get_score(Move m, int depth, bool is_max)
{
    Piece_Type attacker = Encoder::move_get_moved_piece(m);
    Piece_Type victim   = Encoder::move_get_captured_piece(m);

    //uint16_t check_bon = 0;
    //if(move_is_check(m)) return 20;
        //check_bon+=CHECK_BONUS;

    // PV‐move super‐bonus so we try the first "line of moves" we found are best from
    // our previous iteration of the search (iterative deepening)
    // if (m == Encoder::principal_variation_move[0][depth])
    //     return Encoder::PV_BONUS;

    Move_Gen::TTEntry* TT = &Move_Gen::TT[Move_Gen::position_hash & Move_Gen::TT_MASK];

    if(TT->key == Move_Gen::position_hash && TT->bestMove == m && TT->flag == Bound::EXACT)//&& TT->depth >= search_depth - depth)
        return Encoder::TT_BONUS;

    // MVV/LVA captures
    if (victim != Empty
     && attacker < King)    // attacker in [Pawn..Queen]
    {
        return Tables::MVV_LVA[int(attacker)][int(victim)];// + (check_bon);
    }

    // promotions
    Piece_Type promo = Encoder::move_get_promo_piece(m);
    if (promo != Empty)
    {
        switch (promo)
        {
            case Queen:  return Encoder::PROMOTION_BONUS +   0;
            case Rook:   return Encoder::PROMOTION_BONUS -  10;
            case Bishop: return Encoder::PROMOTION_BONUS -  20;
            case Knight: return Encoder::PROMOTION_BONUS -  30;
            default:     return Encoder::PROMOTION_BONUS;
        }
    }

    // killer‐move bonuses
    if (is_max)
    {
        if (m == Encoder::max_killer[0][depth]) return Encoder::KILLER1_BONUS;
        if (m == Encoder::max_killer[1][depth]) return Encoder::KILLER2_BONUS;
    }
    else
    {
        if (m == Encoder::min_killer[0][depth]) return Encoder::KILLER1_BONUS;
        if (m == Encoder::min_killer[1][depth]) return Encoder::KILLER2_BONUS;
    }

    // history heuristic
    // give moves that have caused cutoffs in the past a modest bonus
    // {
    //   int from = Encoder::move_get_from(m);
    //   int to   = Encoder::move_get_to(m);
    //   uint8_t hist = is_max
    //     ? max_history_move_score[from][to]
    //     : min_history_move_score[from][to];
    //   return hist;
    // }

    //
    // if(move_get_moved_piece(m) != Pawn)
    // {
    //     int from_sq = Encoder::move_get_from(m);  // 0..63
    //     int to_sq   = Encoder::move_get_to(m);    // 0..63
    //     int score = 0;

    //     int from_r = from_sq >> 3;   // 0 = rank 8, 1 = rank 7, …, 7 = rank 1
    //     int to_r   = to_sq   >> 3;

    //     // white moves “forward” when it goes toward bigger ranks
    //     // black moves “forward” when it goes toward smaller ranks
    //     int forward = ((!is_max && to_r > from_r)
    //                 | ( is_max && to_r < from_r));  // 0 or 1
    //     int mask    = -forward;                      // 0x00000000 or 0xFFFFFFFF
    //     score += mask & 10;

    //     // truly quiet moves
    //     return score+2;//check_bon;
    // }


    // if(attacker == Pawn) return 4;
    // else if(attacker == Knight) return 5;
    // else if(attacker == Bishop) return 6;
    // else if(attacker == Rook) return 3;
    // else if(attacker == Queen) return 7;

    //if(attacker == Queen) return 7;
    //if(attacker == King) return 0;

    return 0;
}

static inline __attribute__((always_inline)) uint16_t move_get_score_light(Move m, int depth, bool is_max)
{
    Piece_Type attacker = Encoder::move_get_moved_piece(m);
    Piece_Type victim   = Encoder::move_get_captured_piece(m);

    // MVV/LVA captures
    if (victim != Empty
     && attacker < King)    // attacker in [Pawn..Queen]
    {
        return Tables::MVV_LVA[int(attacker)][int(victim)];// + (check_bon);
    }

    // promotions
    Piece_Type promo = Encoder::move_get_promo_piece(m);
    if (promo != Empty)
    {
        switch (promo)
        {
            case Queen:  return Encoder::PROMOTION_BONUS +   0;
            case Rook:   return Encoder::PROMOTION_BONUS -  10;
            case Bishop: return Encoder::PROMOTION_BONUS -  20;
            case Knight: return Encoder::PROMOTION_BONUS -  30;
            default:     return Encoder::PROMOTION_BONUS;
        }
    }

    if(attacker == King) return 0;

    return 2;
}

static inline __attribute__((always_inline))
void sort_moves_by_score_light(int depth, bool skip_first = false) {
    auto &ML = Move_Gen::move_list[depth];
    bool is_max = (Move_Gen::turn == white);
    Move* begin = ML.moves + (skip_first ? 1 : 0);
    size_t n    = ML.count - (skip_first ? 1 : 0);
    std::sort(begin, begin + n,
      [=](Move a, Move b) {
        return move_get_score_light(a, depth, is_max)
             > move_get_score_light(b, depth, is_max);
      }
    );
}

static inline __attribute__((always_inline))
void sort_moves_by_score(int depth, bool skip_first = false) {
    auto &ML = Move_Gen::move_list[depth];
    bool is_max = (Move_Gen::turn == white);
    Move* begin = ML.moves + (skip_first ? 1 : 0);
    size_t n    = ML.count - (skip_first ? 1 : 0);
    std::sort(begin, begin + n,
      [=](Move a, Move b) {
        return move_get_score(a, depth, is_max)
             > move_get_score(b, depth, is_max);
      }
    );
}


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
static inline __attribute__((always_inline)) int pvs_min(int alpha, int beta, int depth);
static inline __attribute__((always_inline)) int static_evaluation(bool is_max, int depth);

// 0 for white (+ infinity) and 1 for black (- infinity)
constexpr const int infinity[2] = { INF, -INF };

unsigned long long node_count;
unsigned long long prune_count;
int pos_eval;
constexpr const int PRESSURE_WEIGHT     = 8;
constexpr const int DPEN                = 16;  // penalty per doubled pawn
constexpr const int TPEN                = 32;  // penalty per tripled pawn
constexpr const int IPEN                = 32;
constexpr const int BISHOP_PAIR_BONUS   = 50;



// wait i am missing here covers of the check or king evasions too because else an attack
// could be not explored (since it will stop middle attack when enemy cant capture or check back)
// ANOTHER IDEA: i eventually need to allow lets say 2-3 quiet moves to check for mates? (but this needs another search, so what to do about it? idk)
//
// THIS IMPLEMENTATION:
// The idea here is: enemy plays last (even depth), and then on my turn i am just allowed to
// capture/promo/check/king ecape/check_block then he is just allowed to do
// same etc til stable position is reached
// i am not convinced 100% about quiesence search tho, it evaluates a lot of nodes (each time we go down a level) and
// it only asnwer us doing nothing vs capture from here evaluation, but the best option may be a check or a quiet move that
// takes you to a worse position and it will miss it so i am not convicned its super usefull
// i think it will be better to give the enemy player an extra move and see where we stand after that tbh
// actually this last idea may favor positions where the queen is hiden behind pieces and cant be capture by the enemy in a single extra move
// so it does not convince me either
//
// NOTE i must FIX the fact that i can simply pad a check position (i must allow check evasion moves instead) since i cant just simply say is better to not do
// anything than move when my king is in check

// Evaluation Table (for saving evaluations)
struct ETEntry {
    uint64_t    key;            // full Zobrist key (64-bit)
    int         evaluation;     // evaluation
};
static constexpr size_t ET_SIZE = 1ULL << 26;
static constexpr size_t ET_MASK = ET_SIZE - 1;
ETEntry   EvalTable[ET_SIZE];

unsigned long long ET_Hits = 0;
unsigned long long ET_Miss = 0;

static inline __attribute__((always_inline)) int quiescence_max(int alpha, int beta, int depth)
{
    node_count++;

    // HASH EVALUATION ========================================================
    uint64_t key = Move_Gen::position_hash & ET_MASK;
    //uint64_t key = ((Move_Gen::position_hash << 6) ^ depth) & ET_MASK;
    ETEntry* ET = &EvalTable[key];
    int initial_evaluation;
    if (ET->key == Move_Gen::position_hash) {
        initial_evaluation = ET->evaluation;
        // if we just got the evaluation form the hash then we must make sure we generate the move for this node
        Move_Gen::generate_moves(depth);
        ET_Hits++;
    } else {
        // this already generates all moves for us and check for checkmate and stalemate
        initial_evaluation  = static_evaluation(1, depth);
        ET->key             = Move_Gen::position_hash;
        ET->evaluation      = initial_evaluation;
        ET_Miss++;
    }
    // ========================================================================

    // prevents infinity checks loop
    if (depth >= search_depth + 8)
        return initial_evaluation;

    int current_node_best = alpha;

    // ensures a mate is not skippped, if initial evaluation is mate (attackers are != 0) then without this safeguard
    // we losse the initial evaluation completly since the iff would be skipped
    // SOLVES THE ISSUE of being in check we still evaluate the position as we can simply do nothing to be better wich makes no sense
    if(Move_Gen::num_attackers == 0 || initial_evaluation < -15000)
    {
        //makes sure we have an alpha to compare against (if moves from here are bad we dont want to take them, example capturing a pawn with a queen)
        if(initial_evaluation>alpha)
            alpha = initial_evaluation;

        if (initial_evaluation >= beta)     // SOFT/HARD cutoff
        {
            prune_count++;
            return initial_evaluation;
        }
        current_node_best = initial_evaluation;
    }

    // Sort the list of moves by MVV/LVA table
    // WE DONT NEED FULL SORTING HERE we dont update killers, or TT, or PV here, so we just need
    // a lightweight MVV/LVA sorting
    sort_moves_by_score_light(depth);

    //
    //int current_node_best = alpha; // start at the lower bound instead (same thing)
    //int current_node_best = initial_evaluation;

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
                if(next_node_evaluation >= alpha)
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
            UndoPacked undo_info;

            // NO CHECKS
            // if  ( ( Encoder::move_get_promo_piece(move) != Empty
            //         || Encoder::move_get_captured_piece(move) != Empty
            //         )
            //     )
            // {
            //         undo_info = Move_Gen::do_move(move);
            // }
            // else
            // {
            //     continue;
            // }


            // ONLY CAPTURES, CHECK, PROMOTIONS ==========================================================================================
            undo_info = Move_Gen::do_move(move);
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
            // we do a soft cuttof (equal positions are not traversed)
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
    // HASH EVALUATION ========================================================
    uint64_t key = Move_Gen::position_hash & ET_MASK;
    //uint64_t key = ((Move_Gen::position_hash << 6) ^ depth) & ET_MASK;
    ETEntry* ET = &EvalTable[key];
    int initial_evaluation;
    if (ET->key == Move_Gen::position_hash) {
        initial_evaluation = ET->evaluation;
        // if we just got the evaluation form the hash then we must make sure we generate the move for this node
        Move_Gen::generate_moves(depth);
        ET_Hits++;
    } else {
        // this already generates all moves for us and check for checkmate and stalemate
        initial_evaluation  = static_evaluation(1, depth);
        ET->key             = Move_Gen::position_hash;
        ET->evaluation      = initial_evaluation;
        ET_Miss++;
    }
    // ========================================================================
    if (depth >= search_depth + 8)
        return initial_evaluation;
    int current_node_best = beta;
    //
    if(Move_Gen::num_attackers == 0 || initial_evaluation > 15000)
    {
        if(initial_evaluation<beta)
            beta = initial_evaluation;
        if(initial_evaluation <= alpha)                                                   // SOFT/HARD CUT-OFFF
        {
            prune_count++;
            return initial_evaluation;
        }
        current_node_best = initial_evaluation;
    }

    sort_moves_by_score_light(depth);
    //int current_node_best = beta;
    //int current_node_best = initial_evaluation;
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

            UndoPacked undo_info;

            // NO CHECK
            // if  ( ( Encoder::move_get_promo_piece(move) != Empty
            //         || Encoder::move_get_captured_piece(move) != Empty
            //         )
            //     )
            // {
            //     undo_info = Move_Gen::do_move(move);
            // }
            // else
            // {
            //     continue;
            // }

            // INCLUDES CHECK
            undo_info = Move_Gen::do_move(move);
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


// static_evaluation: Combines tactical and positional factors into a single centipawn score.
//   is_max: true if evaluating for the side to move in Max nodes, false for Min.
//   depth:  current search depth (used to adjust mate scores for proximity).
// how many attacking pieces (initial idea pieces that are not in starting ranks 12)
// more bigger pieces is better than better pawn structure or 3 pawns almsot alwasy worse than a knight or bishop
static inline __attribute__((always_inline))
int static_evaluation(bool is_max, int depth) {
    using namespace Move_Gen;

    // --- 1) Detect checkmate or stalemate ---
    // generate_moves populates move_list and sets num_attackers if in check.
    generate_moves(depth);
    // If in check and no legal moves → checkmate. Closer mate (smaller depth) is better.
    if (num_attackers > 0 && move_list[depth].count == 0) {
        // For Max nodes, being checkmated is worst: return -(infinity-depth).
        // For Min nodes, inverse sign: return +(infinity-depth).
        int sign = is_max ? -1 : +1;
        return sign * (infinity[is_max] - depth);
    }
    // If not in check and no moves → stalemate (draw = 0 score).
    if (num_attackers == 0 && move_list[depth].count == 0)
        return 0;

    // --- 2) Compute pinned-piece masks ---
    // pin_mask currently contains bits for pieces pinned to our king.
    Bitboard my_pin_mask = pin_mask;
    // Now generate pin mask for the opponent by toggling side-to-move.
    generate_opposite_king_mask();
    Bitboard opp_pin_mask = pin_mask;

    // --- 3) Material + Piece-Square Table (PST) ---
    // raw accumulates White minus Black material/PST score.
    int raw = 0;
    for (int pt = Pawn; pt <= King; ++pt) {
        // Iterate all White pieces of type pt via bitboard scan.
        for (Bitboard w = piece_occ_bb[white][pt]; w; w &= w - 1) {
            int sq = LS1B_IDX(w);
            // Add base value + positional bonus from PST
            raw += Tables::PIECE_VALUE[pt] + Tables::PST[pt][sq];
        }
        // Iterate Black pieces similarly, subtracting their contributions.
        for (Bitboard b = piece_occ_bb[black][pt]; b; b &= b - 1) {
            int sq = LS1B_IDX(b);
            // Mirror square for Black PST indexing
            raw -= Tables::PIECE_VALUE[pt]
                 + Tables::PST[pt][Tables::mirror_square(sq)];
        }
    }

    // --- 4) Pawn-structure penalties: doubled, tripled, and isolated pawns ---
    // DPEN: penalty for each extra pawn on the same file beyond the first
    // TPEN: additional penalty when a third pawn appears on a file
    // IPEN: penalty for any pawn with no friendly pawn on adjacent files

    // 4a) Count doubled & tripled pawns
    int w_doub = 0, w_trip = 0;
    Bitboard wp = piece_occ_bb[white][Pawn];
    for (int f = 0; f < 8; ++f) {
        // Extract only the pawns on file f
        Bitboard onf = wp & Tables::FILE_MASK[f];
        int cnt = COUNT_BITS(onf);  // number of pawns on this file
        // If there are 2, one is 'doubled'; if 3, two are 'doubled' and one is 'tripled'
        if (cnt >= 2) w_doub += cnt - 1;
        if (cnt >= 3) w_trip += cnt - 2;
    }
    int b_doub = 0, b_trip = 0;
    Bitboard bp = piece_occ_bb[black][Pawn];
    for (int f = 0; f < 8; ++f) {
        Bitboard onf = bp & Tables::FILE_MASK[f];
        int cnt = COUNT_BITS(onf);
        if (cnt >= 2) b_doub += cnt - 1;
        if (cnt >= 3) b_trip += cnt - 2;
    }
    // Apply White's penalties by subtracting from raw (raw = W - B)
    raw -= DPEN * w_doub + TPEN * w_trip;
    // Apply Black's penalties by adding
    raw += DPEN * b_doub + TPEN * b_trip;

    // 4b) Count isolated pawns (no friendly pawn on adjacent file)
    auto isolated_mask = [&](Bitboard pawn_bb) {
        // Shift east: only pawns not on H-file, then <<1
        Bitboard east = (pawn_bb & ~Tables::FILE_MASK[7]) << 1;
        // Shift west: only pawns not on A-file, then >>1
        Bitboard west = (pawn_bb & ~Tables::FILE_MASK[0]) >> 1;
        // Pawns with a neighbor on file-adjacent are 'defended'
        Bitboard defended = east | west;
        // Isolated pawns = all pawns minus defended
        return pawn_bb & ~defended;
    };
    int w_iso = COUNT_BITS(isolated_mask(wp));
    int b_iso = COUNT_BITS(isolated_mask(bp));
    raw -= IPEN * w_iso;
    raw += IPEN * b_iso;

    // --- 5) Pinned-piece penalty ---
    // Penalize difference in number of pinned pieces: enemy pins minus our pins
    raw += 16 * (COUNT_BITS(opp_pin_mask) - COUNT_BITS(my_pin_mask));

    // --- 6) Space evaluation ---
    // Compute occupancy once, then masked attack bitboards
    Bitboard occ = player_occ_bb[all_color];
    Bitboard w_space = compute_attack_mask(white, occ) & ~player_occ_bb[white];
    Bitboard b_space = compute_attack_mask(black, occ) & ~player_occ_bb[black];
    // Global space difference
    raw += 2*(COUNT_BITS(w_space) - COUNT_BITS(b_space));
    // Territory control: how many squares in enemy half are attacked
    raw += 2*(COUNT_BITS(w_space & BLACK_TERRITORY)
          - COUNT_BITS(b_space & WHITE_TERRITORY));

    // --- 7) King pressure ---
    // zone_pressure measures cumulative attackers around a king square
    int w_pressure = zone_pressure(white, king_position[black], occ);
    int b_pressure = zone_pressure(black, king_position[white], occ);
    raw += PRESSURE_WEIGHT * (w_pressure - b_pressure);

    // White bishop‐pair
    int w_bish = COUNT_BITS(piece_occ_bb[white][Bishop]);
    if (w_bish >= 2)
        raw += BISHOP_PAIR_BONUS;

    // Black bishop‐pair
    int b_bish = COUNT_BITS(piece_occ_bb[black][Bishop]);
    if (b_bish >= 2)
        raw -= BISHOP_PAIR_BONUS;

    // // Count White’s minor pieces (Knights + Bishops)
    // Bitboard w_minors_bb = piece_occ_bb[white][Knight] | piece_occ_bb[white][Bishop];
    // int w_minors = COUNT_BITS(w_minors_bb);

    // // Count Black’s minor pieces
    // Bitboard b_minors_bb = piece_occ_bb[black][Knight] | piece_occ_bb[black][Bishop];
    // int b_minors = COUNT_BITS(b_minors_bb);

    // int minor_diff = (w_minors - b_minors) * 64;
    // raw += minor_diff;  // positive favors White, negative favors Black

    // Return final centipawn evaluation: positive = White better, negative = Black better
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
        //return quiescence_max(alpha, beta, depth);
        node_count++;
        return static_evaluation(1, depth);
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
            return (-1) * (INF - depth);
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
    sort_moves_by_score(depth);

    //  - infinity basically
    //int current_node_best = -INF;
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

                //
                //Encoder::max_history_move_score[Encoder::move_get_from(move)][Encoder::move_get_to(move)] += 1;
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
            //if(next_node_evaluation > alpha)
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
        //return quiescence_min(alpha, beta, depth);
        node_count++;
        return static_evaluation(0, depth);
    }
    Move_Gen::generate_moves(depth);

    if(Move_Gen::num_attackers > 0)
    {
        if(Move_Gen::move_list[depth].count == 0)
        {
            return (INF - depth);
        }

    }
    else
    {
        if(Move_Gen::move_list[depth].count == 0)
            return 0;
    }
    // Sort the list of moves by MVV/LVA table
    sort_moves_by_score(depth);
    //int current_node_best = INF;
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
                //Encoder::min_history_move_score[Encoder::move_get_from(move)][Encoder::move_get_to(move)] += 1;
            }
            prune_count++;
            return next_node_evaluation;
        }
        if(next_node_evaluation < current_node_best)
        {
            current_node_best = next_node_evaluation;
            //if(next_node_evaluation < beta)
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


// Define some const needed for debug and delta for the window width
static constexpr int DELTA = 1;
unsigned long long re_search_count;
unsigned long long probe_fail_high = 0, probe_fail_low = 0;

static inline __attribute__((always_inline)) int pvs_max(int alpha, int beta, int depth)
{
    if (depth == search_depth)
    {
        return quiescence_max(alpha, beta, depth);
        // node_count++;
        // return static_evaluation(1, depth);
    }

    // Transposition Table=====================================================
    Move_Gen::TTEntry* TT = &Move_Gen::TT[Move_Gen::position_hash & Move_Gen::TT_MASK];
    int remaining_depth = search_depth - depth;
    bool better_depth = (remaining_depth > TT->depth)?true:false;
    // bool better_depth = true;
    if(TT->key == Move_Gen::position_hash)
    {
        if(TT->flag == Bound::EXACT && TT->depth >= remaining_depth)
        {
            return TT->score;
        }
        // LOWER bound (fail‐high): true_score ≥ TT->score ≥ β => safe to cutoff at β
        else if(TT->flag == Bound::LOWER && TT->score >= beta && TT->depth >= remaining_depth)
        {
            // return beta;
            return TT->score;
        }
        // UPPER bound (fail‐low): true_score ≤ TT->score ≤ α => safe to cutoff at α
        else if(TT->flag == Bound::UPPER && TT->score <= alpha && TT->depth >= remaining_depth)
        {
            return TT->score;
            //return alpha;
        }
    }
    // ========================================================================

    Move_Gen::generate_moves(depth);
    if (Move_Gen::move_list[depth].count == 0)
        return Move_Gen::num_attackers > 0 ? - (INF - depth) : 0;

    sort_moves_by_score(depth);
    int best = alpha;
    int orig_alpha = alpha, orig_beta = beta;

    for (int i = 0; i < Move_Gen::move_list[depth].count; ++i) {
        Move m = Move_Gen::move_list[depth].moves[i];
        UndoPacked undo = Move_Gen::do_move(m);

        int score;
        if (i == 0) {
            // first move = full window
            score = pvs_min(alpha, beta, depth + 1);
        } else {
            // null-window probe
            score = pvs_min(alpha, alpha + DELTA, depth + 1);

            // fail-high: if probe ≥ beta, undo & cutoff immediately
            if (score >= beta) {
                probe_fail_high++;
                Move_Gen::undo_move(m, undo);

                // Killer Moves============================================================
                if (Encoder::move_get_captured_piece(m) == Empty &&
                    Encoder::move_get_promo_piece(m)     == Empty &&
                    //!Encoder::move_is_check(m) &&
                    m != Encoder::max_killer[0][depth])
                {
                    Encoder::max_killer[1][depth] = Encoder::max_killer[0][depth];
                    Encoder::max_killer[0][depth] = m;
                }
                // ========================================================================

                // Transposition Table=====================================================
                if(better_depth)
                {
                    TT->key         = Move_Gen::position_hash;
                    TT->bestMove    = m;
                    TT->score       = score;
                    TT->flag        = Bound::LOWER;
                    TT->depth       = remaining_depth;
                    // TT->age         += 1;
                }
                // ========================================================================

                prune_count++;
                return score;
            }

            // re-search full if it might beat alpha
            if (score > alpha) {
                re_search_count++;
                score = pvs_min(alpha, beta, depth + 1);
            }
        }

        Move_Gen::undo_move(m, undo);

        // immediate β-cutoff
        if (score >= beta) {
            if (Encoder::move_get_captured_piece(m) == Empty &&
                Encoder::move_get_promo_piece(m)     == Empty &&
                //!Encoder::move_is_check(m) &&
                m != Encoder::max_killer[0][depth])
            {
                Encoder::max_killer[1][depth] = Encoder::max_killer[0][depth];
                Encoder::max_killer[0][depth] = m;
            }
            // Transposition Table=====================================================
            if(better_depth)
            {
                TT->key         = Move_Gen::position_hash;
                TT->bestMove    = m;
                TT->score       = score;
                TT->flag        = Bound::LOWER;
                TT->depth       = remaining_depth;
                // TT->age         += 1;
            }
            // ========================================================================

            prune_count++;
            return score;
        }

        // α / PV update
        if (score > best) {
            best = score;
            if (score > alpha) {
                alpha = score;
                // update PV
                Encoder::principal_variation_move[depth][0] = m;
                int tail = Encoder::principal_variation_length[depth + 1];
                Encoder::principal_variation_length[depth] = 1 + tail;
                for (int j = 0; j < tail; ++j)
                    Encoder::principal_variation_move[depth][j+1] =
                        Encoder::principal_variation_move[depth + 1][j];
            }
        }
    }

    // Transposition Table=====================================================
    if(better_depth)
    {
        TT->key                                 = Move_Gen::position_hash;
        TT->bestMove                            = Encoder::principal_variation_move[depth][0];
        TT->score                               = best;
        if(best >= orig_beta)          TT->flag = Bound::LOWER;
        else if(best <= orig_alpha)    TT->flag = Bound::UPPER;
        else                           TT->flag = Bound::EXACT;
        TT->depth                               = remaining_depth;
        // TT->age                                 += 1;
    }
    // ========================================================================

    return best;
}

static inline __attribute__((always_inline)) int pvs_min(int alpha, int beta, int depth)
{
    if (depth == search_depth)
    {
        return quiescence_min(alpha, beta, depth);
        // node_count++;
        // return static_evaluation(0, depth);
    }

    // Transposition Table=====================================================
    Move_Gen::TTEntry* TT = &Move_Gen::TT[Move_Gen::position_hash & Move_Gen::TT_MASK];
    // remaining depth cuz we evaluated this position when we had "X" remaining depth and
    // since evaluation is bottom up this translates to this evaluation is "X" level deep
    int remaining_depth = search_depth - depth;
    bool better_depth = (remaining_depth > TT->depth)?true:false;
    // bool better_depth = true;
    if(TT->key == Move_Gen::position_hash)
    {
        if(TT->flag == Bound::EXACT && TT->depth >= remaining_depth)
        {
            return TT->score;
        }
        else if(TT->flag == Bound::UPPER && TT->score <= alpha && TT->depth >= remaining_depth)
        {
            // return alpha;
            return TT->score;
        }
        else if(TT->flag == Bound::LOWER && TT->score >= beta && TT->depth >= remaining_depth)
        {
            return TT->score;
            // return beta;
        }
    }
    // ========================================================================

    Move_Gen::generate_moves(depth);
    if (Move_Gen::move_list[depth].count == 0)
        return Move_Gen::num_attackers > 0 ?   (INF - depth) : 0;

    sort_moves_by_score(depth);
    int best = beta;
    int orig_alpha = alpha, orig_beta = beta;

    for (int i = 0; i < Move_Gen::move_list[depth].count; ++i) {
        Move m = Move_Gen::move_list[depth].moves[i];
        UndoPacked undo = Move_Gen::do_move(m);

        int score;
        if (i == 0) {
            // first move = full window
            score = pvs_max(alpha, beta, depth + 1);
        } else {
            // null-window probe
            score = pvs_max(beta - DELTA, beta, depth + 1);

            // fail-low: if probe ≤ alpha, undo & cutoff immediately
            if (score <= alpha) {
                probe_fail_low++;
                Move_Gen::undo_move(m, undo);

                // Killer Moves============================================================
                if (Encoder::move_get_captured_piece(m) == Empty &&
                    Encoder::move_get_promo_piece(m)     == Empty &&
                    //!Encoder::move_is_check(m) &&
                    m != Encoder::min_killer[0][depth])
                {
                    Encoder::min_killer[1][depth] = Encoder::min_killer[0][depth];
                    Encoder::min_killer[0][depth] = m;
                }
                // ========================================================================

                // Transposition Table=====================================================
                if(better_depth)
                {
                    TT->key         = Move_Gen::position_hash;
                    TT->bestMove    = m;
                    TT->score       = score;
                    TT->flag        = Bound::UPPER;
                    TT->depth       = remaining_depth;
                    // TT->age         += 1;
                }
                // ========================================================================

                prune_count++;
                return score;
            }

            // re-search full if it might beat beta
            if (score < beta) {
                re_search_count++;
                score = pvs_max(alpha, beta, depth + 1);
            }
        }

        Move_Gen::undo_move(m, undo);

        // immediate α-cutoff
        if (score <= alpha) {
            if (Encoder::move_get_captured_piece(m) == Empty &&
                Encoder::move_get_promo_piece(m)     == Empty &&
                //!Encoder::move_is_check(m) &&
                m != Encoder::min_killer[0][depth])
            {
                Encoder::min_killer[1][depth] = Encoder::min_killer[0][depth];
                Encoder::min_killer[0][depth] = m;
            }
            // Transposition Table=====================================================
            if(better_depth)
            {
                TT->key         = Move_Gen::position_hash;
                TT->bestMove    = m;
                TT->score       = score;
                TT->flag        = Bound::UPPER;
                TT->depth       = remaining_depth;
                // TT->age         += 1;
            }
            // ========================================================================

            prune_count++;
            return score;
        }

        // β / PV update
        if (score < best) {
            best = score;
            if (score < beta) {
                beta = score;
                // update PV
                Encoder::principal_variation_move[depth][0] = m;
                int tail = Encoder::principal_variation_length[depth + 1];
                Encoder::principal_variation_length[depth] = 1 + tail;
                for (int j = 0; j < tail; ++j)
                    Encoder::principal_variation_move[depth][j+1] =
                        Encoder::principal_variation_move[depth + 1][j];
            }
        }
    }

    // Transposition Table=====================================================
    if(better_depth)
    {
        TT->key                                 = Move_Gen::position_hash;
        TT->bestMove                            = Encoder::principal_variation_move[depth][0];
        TT->score                               = best;
        if(best >= orig_beta)          TT->flag = Bound::LOWER;
        else if(best <= orig_alpha)    TT->flag = Bound::UPPER;
        else                           TT->flag = Bound::EXACT;
        TT->depth                               = remaining_depth;
        // TT->age                                 += 1;
    }
    // ========================================================================

    return best;
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

    int alpha = -INF;
    int beta  = INF;
    int current_node_best = alpha;  // we want the highest score

    search_depth = max_depth;

    // Transposition Table=====================================================
    // Move_Gen::TTEntry* TT = &Move_Gen::TT[Move_Gen::position_hash & Move_Gen::TT_MASK];
    // ========================================================================


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
        //int next_eval = alpha_beta_min(alpha, beta, /*depth=*/1);
        int next_eval = pvs_min(alpha, beta, /*depth=*/1);

        //
        if (next_eval > current_node_best)
        {
            // our probe searched proved us wrong, we found a better possible move so we research fully for the exact evaluation of this line of play
            if(i!=0)
            {
                // next_eval = alpha_beta_min(alpha,  INF, /*depth=*/1);
                re_search_count++;
                next_eval = pvs_min(alpha,  INF, /*depth=*/1);
            }


            current_node_best = next_eval;
            best_move         = m;
            alpha             = next_eval;  // tighten α

            beta              = alpha+DELTA;    // we found a good move and since we assume good ordering we will betfrom now on this is the best move, so only probe searches

            // copy the PV from ply 1 into pv_table[0]
            principal_variation_move[0][0] = m;
            principal_variation_length[0]   = 1 + principal_variation_length[1];
            for (int j = 0; j < principal_variation_length[1]; ++j)
                principal_variation_move[0][1+j] = principal_variation_move[1][j];
        }
        // no β‐cut at root

        //
        Move_Gen::undo_move(m, undo_info);
    }

    // Transposition Table=====================================================
    // TT->key                                 = Move_Gen::position_hash;
    // TT->bestMove                            = Encoder::principal_variation_move[0][0];
    // TT->score                               = alpha;
    // TT->flag                                = Bound::EXACT;
    // TT->depth                               = search_depth;
    // ========================================================================

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
    int alpha = -INF;
    int beta  = INF;
    int current_node_best = beta;  // we want the lowest score

    // Transposition Table=====================================================
    // Move_Gen::TTEntry* TT = &Move_Gen::TT[Move_Gen::position_hash & Move_Gen::TT_MASK];
    // ========================================================================

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
        //int next_eval = alpha_beta_max(alpha, beta, /*depth=*/1);
        int next_eval = pvs_max(alpha, beta, /*depth=*/1);

        if (next_eval < current_node_best)
        {
            if(i!=0)
            {
                //next_eval = alpha_beta_max(-INF, beta, /*depth=*/1);
                next_eval = pvs_max(-INF, beta, /*depth=*/1);
                re_search_count++;
            }
            current_node_best = next_eval;
            best_move         = m;
            beta              = next_eval;  // tighten β
            alpha             = beta-DELTA;
            principal_variation_move[0][0] = m;
            principal_variation_length[0]   = 1 + principal_variation_length[1];
            for (int j = 0; j < principal_variation_length[1]; ++j)
                principal_variation_move[0][1+j] = principal_variation_move[1][j];
        }
        // no α‐cut at root
        Move_Gen::undo_move(m, undo_info);
    }
    // Transposition Table=====================================================
    // TT->key                                 = Move_Gen::position_hash;
    // TT->bestMove                            = Encoder::principal_variation_move[0][0];
    // TT->score                               = beta;
    // TT->flag                                = Bound::EXACT;
    // TT->depth                               = search_depth;
    // ========================================================================
    pos_eval = current_node_best;
    return best_move;
}

// serves as counter for clearing the evaluation chache
static uint8_t busquedas = 0;
static inline __attribute__((always_inline))
Move iterative_deepen(bool white_to_move, int max_depth)
{
    std::fill(Encoder::principal_variation_length, Encoder::principal_variation_length + max_depth + 1, 0);
    std::memset(Encoder::max_killer, 0, sizeof(Encoder::max_killer));
    std::memset(Encoder::min_killer, 0, sizeof(Encoder::min_killer));

    // if(busquedas == 4)
    // {
    //     busquedas = 0;
    //     std::memset(EvalTable, 0, sizeof(EvalTable));
    // }

    // clear the transposition table once per move
    std::memset(Move_Gen::TT, 0, sizeof(Move_Gen::TT));

    Move best = 0;
    for (int d = 1; d <= max_depth; ++d) {
        ET_Hits = 0;    ET_Miss = 0;
        node_count = 0;
        re_search_count = 0;
        probe_fail_high = 0; probe_fail_low = 0;
        if (white_to_move) {
            best = find_best_move_white(d);
        } else {
            best = find_best_move_black(d);
        }
        printf("Depth: %d, Node Count: %llu, Re-Search Count: %llu (%.1f%%) ", d, node_count, re_search_count, 100.0 * re_search_count / (node_count + 1));
        printf("Probe cutoffs: fail-high=%llu, fail-low=%llu , ",
       probe_fail_high, probe_fail_low);
       printf("ET_Hits: %llu, ET_Miss: %llu (%.1f%%)\n", ET_Hits, ET_Miss, 100.0*ET_Hits/(ET_Hits+ET_Miss));
    }
    // busquedas++;
    return best;
}

}   // end Search namespace
}   // end RedStone namespace