#pragma once
#include"../utils/misc.h"
#include"tables.h"

namespace RedStone{

static constexpr unsigned long MAX_DEPTH = 64;
static constexpr int MAX_MOVES = 218;

// Helpers Macros ===================================================================================================================================================

// flips the nth bit of a bitboard
#define FLIP_BIT(x, n)  ((x) ^= (1ULL << (n)))

// macro to clear (pop, set to 0 same) all specific (given) bit from a given side
// RECALL:
//  extern Bitboard piece_occ_bb[2][6];     // [side][piece_type]
//  extern Bitboard player_occ_bb[3];       // [WHITE=0], [BLACK=1], [ALL=2]
//  enum Piece_Type { Pawn, Knight, Bishop, Rook, Queen, King, Empty };
#define CLEAR_ALL_SQUARE(side, idx)                     \
    {                                                   \
        for (int _pt = 0; _pt < 6; ++_pt) {             \
            POP_BIT(Move_Gen::piece_occ_bb[(side)][_pt], (idx));  \
        }                                               \
        POP_BIT(Move_Gen::player_occ_bb[(side)], (idx));          \
        POP_BIT(Move_Gen::player_occ_bb[2], (idx));               \
    }

#define SET_SQUARE_SIMPLE(side, ptype, idx)            \
    {                                                  \
        SET_BIT(Move_Gen::piece_occ_bb[(side)][(ptype)], (idx)); \
        SET_BIT(Move_Gen::player_occ_bb[(side)], (idx));         \
        SET_BIT(Move_Gen::player_occ_bb[2], (idx));              \
    }

//
#define PACK_MOVE(from_sq, to_sq, mv_piece, cp_piece, promo_piece, flags_byte) \
  Encoder::construct_move(                                                   \
    (from_sq),                                                               \
    (to_sq),                                                                 \
    (mv_piece),                                                              \
    (cp_piece),                                                              \
    (promo_piece),                                                           \
    (flags_byte),                                                            \
    /* score */ 0u                                                           \
  )



// IS A SQUARE ATTACKED? ===========================================================================================================================================

// Single macro: returns non-zero (true) if attacked, zero (false) otherwise.
// Note: in C/C++, a non-zero Bitboard in boolean context is true. We compare to 0ULL explicitly for clarity.
//
// white point of view (pawn) (ill explain the logic trick here):
// the cleaver observation here is: if a black pawn was placed on this square we are currently checking, and this black pawn
// attacks ANY white pawn on the board then this square is attacked by a white pawn tudum
//
// so we get the attack mask from a black (opossite) color pawn placed on this square
// and AND it with the position of all white pawns on the board
// if this return true then the square is attacked by a white pawn
// Bishops & Queens (diagonals): (same but we of course need the board state to find the correct attack mask of the "placed on this square enemy bishop/queen")
// then we OR the queen and bishops of the player at turn to get the Bitboard with all bishop and queen of this player
// and finally we AND the above Bitboard with the attacks mask of the "enemy bishop/queen placed on the square we checkign"

#define IS_SQUARE_ATTACKED(turn, occ, idx)                                                                      \
    (                                                                                                           \
      /* Pawn attacks: if any pawn of 'turn' attacks idx */                                                     \
      ((Tables::pawn_attack_bb[!(turn)][(idx)] & Move_Gen::piece_occ_bb[(turn)][Piece_Type::Pawn]) != 0ULL)                       \
      ||                                                                                                        \
      /* Knight attacks */                                                                                      \
      ((Tables::knight_attack_bb[(idx)] & Move_Gen::piece_occ_bb[(turn)][Piece_Type::Knight]) != 0ULL)                            \
      ||                                                                                                        \
      /* King attacks (adjacency) */                                                                            \
      ((Tables::king_attack_bb[(idx)] & Move_Gen::piece_occ_bb[(turn)][Piece_Type::King]) != 0ULL)                                \
      ||                                                                                                        \
      /* Bishop/Queen diagonal attacks */                                                                       \
      ((GET_BISHOP_ATTACK((occ),(idx)) &                                                                        \
         (Move_Gen::piece_occ_bb[(turn)][Piece_Type::Bishop] | Move_Gen::piece_occ_bb[(turn)][Piece_Type::Queen])) != 0ULL)         \
      ||                                                                                                        \
      /* Rook/Queen orthogonal attacks */                                                                       \
      ((GET_ROOK_ATTACK((occ),(idx)) &                                                                          \
         (Move_Gen::piece_occ_bb[(turn)][Piece_Type::Rook]   | Move_Gen::piece_occ_bb[(turn)][Piece_Type::Queen])) != 0ULL)         \
    )

// =========================================================================================================================================================

//------------------------------------------------------------------------------
//    The entry “flags” tell us whether the stored score is an exact value, a
//    lower bound (β cutoff), or an upper bound (α cutoff).
//------------------------------------------------------------------------------
enum class Bound : uint8_t {
    EXACT,      // exact score (true minimax value was computed, no prunes, seacrh all the way to depth)
    LOWER,      // score ≥ storedScore  (beta cutoff)
    UPPER       // score ≤ storedScore  (alpha cutoff)
};

namespace Move_Gen{

// Globals====================================================================================================================================================

// big enough so all possible move in any position fit here
// tracks how many moves were generated on this turn
// here we use the fact DFS goes all the way down and alwasy have 1 valid moves per depth
// you overwrite it over and over aagin
// this struct willl holds the valid move information
struct Move_List {
  Move  moves[MAX_MOVES];
  int   count;
};
// one MoveList per depth
extern Move_List move_list[MAX_DEPTH];


extern int king_position[2];
extern int castle_right;
extern int en_passant;
extern bool turn;

// this one stores the pinned pieces occupancy Bitboard
extern Bitboard pin_mask;
// this one stores the possible moves Bitboard for pinned pieces from a given square
extern Bitboard pin_ray_mask[64];

// same as above but for discovered check potential pieces
extern Bitboard discovered_mask;
extern Bitboard discovered_ray_mask[64];

// contain (if attacker to the king is 1) the only possible blocking/capture moves for player
extern Bitboard check_mask;
// contains the number of attackers to the king
// (if count is 0 = no check, if count is 1 (single attacker), if count is 2 (multiple attackers (MOVE UNBLOCKABLE, KING MUST MOVE)))
extern int num_attackers;

// 0 -> White pieces bitboards   or   1 -> Black pieces bitboards
// 0 trhough 6 the pieces in P, N, B, R, Q, K order
// piece occupancy bitbord basically
extern Bitboard piece_occ_bb[2][6];     // [side][piece_type]
extern Bitboard player_occ_bb[3];       // [WHITE=0], [BLACK=1], [ALL=2]
extern Piece_Type mailbox[2][64];

// =========================================================================================================================================================

// TRANSPOSITION TABLE =============================================================================================

//------------------------------------------------------------------------------
//    Each bucket holds one TT entry.  The best move from this position is found here,
//    the evaluation or cutoff score, the depth-at-which it was searched.
//------------------------------------------------------------------------------
struct TTEntry {
    uint64_t    key;        // full Zobrist key (64-bit)
    Move        bestMove;   // best move from this position
    int16_t     score;      // score from the search
    int8_t      depth;      // search depth at which score was computed
    Bound       flag;       // exact / lower / upper bound
    //uint8_t     age;        // for aging out old entries
};

//------------------------------------------------------------------------------
//    The table itself is just a power-of-two array of buckets.  We mask the
//    Zobrist key to pick an index.
//------------------------------------------------------------------------------
static constexpr size_t TT_SIZE = 1ULL << 24;        // (24)~16M entries, 512 mib or so, we can push 1<<27 (4 gigas) for test or locally play
static constexpr size_t TT_MASK = TT_SIZE - 1;
extern TTEntry   TT[TT_SIZE];

// unique number identifying a position
extern uint64_t position_hash;

// ==================================================================================================================

// clear past pin_mask and pin_ray_mask and generate the new ones for the current position
static inline __attribute__((always_inline)) void generate_pin_mask()
{
    /* reset pin_mask first */
    //pin_mask = 0ULL;
    // std::memset(pin_ray_mask, 0, sizeof(pin_ray_mask));
    std::memset(pin_ray_mask, 0, 512);

    /* we want to AND the rays outwards from the king position (orthogonal and diagonal rays) */
    /* with the slider pieces occupancy Bitboards of the enemy (queen/rook -> orthogonal | queen/bishop -> diagonal) */
    /* this way we obtain a potential attackers to our king Bitboard mask in the corresponding direction */
    /* P(0), N(1), B(2), R(3), Q(4), K(5) */

    // save our king pos
    int king_sq = king_position[turn];

    // opponent sliders occupancy Bitboard
    Bitboard opp_queen_rook_occ = piece_occ_bb[!turn][Rook] | piece_occ_bb[!turn][Queen];
    Bitboard opp_queen_bishop_occ = piece_occ_bb[!turn][Bishop] | piece_occ_bb[!turn][Queen];

    //
    Bitboard potential_diag_attackers = Tables::diagonal_ray_mask[king_sq] & opp_queen_bishop_occ;
    Bitboard potential_orth_attackers = Tables::orthogonal_ray_mask[king_sq] & opp_queen_rook_occ;

    /* now we must check if from these potential sliders attackers positions there are a single blocking piece to the king */
    /* basically "in between" king square and potential slider square precomputed table will give us a Bitboard we can use */
    /* to AND with the all occupancy Bitboard to look for how many pieces are blocking this ray */
    /* if there is a single blocker in between this attacker and the king (if it is allied piece) then this blocker is definetly Pinned (we can use bitcount here) */
    /* if there is a single blocker in between this attacker and the king (enemy piece) then this is a potential discovered attack */

    /* if the potential attackers is zero no worries in this direction, else we check possible blocker count between the potential pinner and our king */
    /* note while (potential_diag_attackers) already takes care of this check so thats cool (since that wont run if that is equal to zero) */
    /* while there are still bits to check (potential attackers basically) */
    while (potential_diag_attackers) {

        /* LS1B_IDX(potential_diag_attackers) -> get the index of the LS1B to know from which square to check */
        int attacker_sq = LS1B_IDX(potential_diag_attackers);

        // contains a Bitboard with only the "in between" bits between these 2 positions set
        Bitboard between = Tables::between_squares_mask[attacker_sq][king_sq];

        potential_diag_attackers &= potential_diag_attackers - 1;   // clear LS1B from this Bitboard for next iteration

        Bitboard blockers = between & player_occ_bb[all_color]; /* it means all occupancy Bitboard (both black and white) */

        /* if the bit count of between_bb is not 1 (0 -> no blockers so this is actually check lol, 2 or more no pins nor discovered attacks from this position) */
        if (COUNT_BITS(blockers) != 1) { continue; }

        /* if it is equal to 1 there are 2 possible choices here */
        /* enemy piece to the king -> we have a discovered check potential */
        /* allied piece to the king -> pinned piece, it can only move along the pinned direction (back or forward and of course capture the attacker that is pinning the piece) */
        /* so lets check if this square is on the allied pieces occupancy squares (if this is true basically) */
        if (blockers & player_occ_bb[(turn)]) {
            int blocker_sq = LS1B_IDX(blockers);
            /* then we can add that piece to the pin_mask for this (allied to the king) side */
            SET_BIT(pin_mask, blocker_sq);

            /* compute pin-ray mask: */
            /* basically from the king (in the pinned direction) all the way to the attacking piece (the one pinning the piece) */
            /* save all possible moves for this pinned piece for the move generation functionality */
            pin_ray_mask[blocker_sq] = between | (1ULL << attacker_sq);
        }
    }
    // same for orthogonal attackers
    while (potential_orth_attackers) {
        int attacker_sq = LS1B_IDX(potential_orth_attackers);
        Bitboard between_bb = Tables::between_squares_mask[attacker_sq][king_sq];
        potential_orth_attackers &= potential_orth_attackers - 1;
        Bitboard blockers = between_bb & player_occ_bb[all_color];
        if (COUNT_BITS(blockers) != 1) { continue; }
        if (blockers & player_occ_bb[(turn)]) {
            int blocker_sq = LS1B_IDX(blockers);
            SET_BIT(pin_mask, blocker_sq);
            pin_ray_mask[blocker_sq] = between_bb | (1ULL << attacker_sq);
        }
    }
}

static inline __attribute__((always_inline))
void generate_opposite_king_mask()
{
    // clear out last run
    pin_mask = 0ULL;
    std::memset(pin_ray_mask, 0, 512);

    // map your bool 'turn' into the Color enum
    Color me   = turn ? black : white;
    Color them = turn ? white : black;

    // their king’s square
    int ksq = king_position[them];

    // build our slider occupancy
    Bitboard our_QR = piece_occ_bb[me][Rook]   | piece_occ_bb[me][Queen];
    Bitboard our_QB = piece_occ_bb[me][Bishop] | piece_occ_bb[me][Queen];

    // all-pieces occupancy
    Bitboard occ    = player_occ_bb[all_color];

    // potential pinners along rays
    Bitboard diag_sliders  = Tables::diagonal_ray_mask[ksq]   & our_QB;
    Bitboard ortho_sliders = Tables::orthogonal_ray_mask[ksq] & our_QR;

    // check diagonal pins
    while (diag_sliders) {
      int a = LS1B_IDX(diag_sliders);
      diag_sliders &= diag_sliders - 1;

      Bitboard between  = Tables::between_squares_mask[a][ksq];
      Bitboard blockers = between & occ;

      if (COUNT_BITS(blockers) == 1 && (blockers & player_occ_bb[them])) {
        int b = LS1B_IDX(blockers);
        SET_BIT(pin_mask, b);
        pin_ray_mask[b] = between | (1ULL << a);
      }
    }

    // check orthogonal pins
    while (ortho_sliders) {
      int a = LS1B_IDX(ortho_sliders);
      ortho_sliders &= ortho_sliders - 1;

      Bitboard between  = Tables::between_squares_mask[a][ksq];
      Bitboard blockers = between & occ;

      if (COUNT_BITS(blockers) == 1 && (blockers & player_occ_bb[them])) {
        int b = LS1B_IDX(blockers);
        SET_BIT(pin_mask, b);
        pin_ray_mask[b] = between | (1ULL << a);
      }
    }
}

// i could know which move is check while generating them by using this (for discovered attacks on enemy king)
// simply before creating moves test for it and we will figure it out but idk if its necessary so right now i am not using it
static inline __attribute__((always_inline)) void generate_discovered_mask()
{
    /* reset discovered masks first */
    discovered_mask = 0ULL;
    // std::memset(discovered_ray_mask, 0, sizeof(discovered_ray_mask));
    std::memset(discovered_ray_mask, 0, 512);

    // save opponent king square
    int opp_king_sq = king_position[!turn];
    // all occupancy bitboard
    Bitboard all_occ_bb = player_occ_bb[all_color];
    // our occupancy bitboard
    Bitboard our_occ_bb =  player_occ_bb[turn];

    /*
    for Discovered checks we kind of do the sameg thing of for the pinned mask (but
    opposite king)
    Precomputed ray masks from king square (all squares along diagonal/orth from king,
    including distant ones)
    */
    Bitboard opp_king_diag_rays = Tables::diagonal_ray_mask[king_position[!turn]];
    Bitboard opp_king_orth_rays = Tables::orthogonal_ray_mask[king_position[!turn]];

    //
    Bitboard ally_queen_rook_occ = piece_occ_bb[turn][Rook] | piece_occ_bb[turn][Queen];
    Bitboard ally_queen_bishop_occ = piece_occ_bb[turn][Bishop] | piece_occ_bb[turn][Queen];


    /* Mask of possible of our potentially discover check options */
    Bitboard potential_diag_attackers = opp_king_diag_rays & ally_queen_bishop_occ;
    Bitboard potential_orth_attackers = opp_king_orth_rays & ally_queen_rook_occ;

    // while there are potential diag discovered checks attackers for us
    while (potential_diag_attackers)
    {
        int attacker_sq = LS1B_IDX(potential_diag_attackers);               /* get the idx of the ls1b */
        potential_diag_attackers &= potential_diag_attackers - 1;           /* clear the LS1B for next */

        Bitboard between = Tables::between_squares_mask[opp_king_sq][attacker_sq];
        Bitboard candidate = between & all_occ_bb;

        /* NOTE: pawns and the king can move on the same diagonal or col (and king file) and
           not check the enemy king so we must capture the direction of moves too for them */
        /* if the blockers count between the dicovered option piece and the enemy king is
           exactly one then this piece is either a pinned piece (for the enemy) or a potential
           discovered check pieces (for us, as long as it does not move on the same aligned
           ray), so for the sake of making sure its ours, we will
           check it vs our pieces (to see if the result is 0 (not our piece) or >0 (our piece))*/

        if ((COUNT_BITS(candidate) == 1) && ((candidate & our_occ_bb) != 0ULL))
        {
            int candidate_idx = LS1B_IDX(candidate);
            SET_BIT(discovered_mask, candidate_idx);

            /* basically from the king (in the "discovered" direction) all the way to the
               attacking piece (the one that potentially will discovered check the king)
               save all possible moves that this piece can do to provide a discovered attack
               on the enemy king */
            /* use only ‘between’ since slider_sq is occupied by our own piece and moves there will be filtered out */
            discovered_ray_mask[candidate_idx] = ~(between);
        }
    }

    // same for orthogonal possible discovered attack choices for this player
    while (potential_orth_attackers) {
        int attacker_sq = LS1B_IDX(potential_orth_attackers);                       /* get the idx of LS1B */
        potential_orth_attackers &= potential_orth_attackers - 1;                   /* clear LS1B for next */
        Bitboard between = Tables::between_squares_mask[opp_king_sq][attacker_sq];
        Bitboard candidate = between & all_occ_bb;
        if ((COUNT_BITS(candidate) == 1) && ((candidate & our_occ_bb) != 0ULL))
        {
            int candidate_idx = LS1B_IDX(candidate);
            SET_BIT(discovered_mask, candidate_idx);
            discovered_ray_mask[candidate_idx] = ~(between);
        }
    }

}

static inline __attribute__((always_inline)) void generate_check_mask()
{
    // full occupancy bitboard
    Bitboard all_occ = player_occ_bb[all_color];

    // enemy attackers on our king position
    Bitboard attackers = 0ULL;
    bool opp = !turn;
    int ally_king_sq = king_position[turn];
    Bitboard opp_pawn_occ = piece_occ_bb[opp][Pawn];
    Bitboard opp_knight_occ = piece_occ_bb[opp][Knight];
    Bitboard opp_bishop_occ = piece_occ_bb[opp][Bishop];
    Bitboard opp_queen_occ = piece_occ_bb[opp][Queen];
    Bitboard opp_rook_occ = piece_occ_bb[opp][Rook];

    // build the all attackers bitboard here:
    /* Pawn attacks: if an enemy pawn on some sq attacks our king_sq */
    attackers |= ( Tables::pawn_attack_bb[turn][ally_king_sq] & opp_pawn_occ ); // opp for turn
    /* Knight attacks */
    attackers |= ( Tables::knight_attack_bb[ally_king_sq] & opp_knight_occ );
    // diagonal attacks
    Bitboard bishop_like = opp_bishop_occ | opp_queen_occ;
    attackers |= GET_BISHOP_ATTACK(all_occ, ally_king_sq) & bishop_like;
    // orthogonal attacks
    Bitboard rook_like = opp_rook_occ | opp_queen_occ;
    attackers |= GET_ROOK_ATTACK(all_occ, ally_king_sq) & rook_like;

    // count how many attackers currently to our king and save it in num_attackers global
    num_attackers = COUNT_BITS(attackers);

    /* build check_mask Global if exactly one attacker else keep it as 0ULL */
    check_mask = 0ULL;
    if (num_attackers == 1)
    {
        /* isolate the single attacker square */
        int att_sq = LS1B_IDX(attackers);

        /* decide if this attacker is slider-type (bishop/rook/queen) or a leaper (pawn/knight) */
        //bool is_slider = false;

        // /* reuse bishop-like test: if attacker bit is in enemy bishop_or_queen */
        // if ( (opp_bishop_occ | opp_queen_occ) & (1ULL << att_sq) )
        //     is_slider = true;

        // /* reuse rook-like test: if attacker bit is in enemy rook_or_queen */
        // if ((opp_rook_occ | opp_queen_occ) & (1ULL << att_sq))
        //     is_slider = true;

        Piece_Type attacker = mailbox[opp][att_sq];

        bool is_slider = (attacker == Bishop || attacker == Rook || attacker == Queen);

        if (is_slider)                                      /* slider: blockable by interposition */
        {
            /* squares strictly between attacker and king */
            Bitboard between = Tables::between_squares_mask[att_sq][ally_king_sq];
            // plus the attacker square (since capturing him take us out of check)
            check_mask = between | (1ULL << att_sq);
        }
        else
            check_mask = (1ULL << att_sq);                  /* pawn or knight (or king) attacker: only capture-the-attacker works */
    }
    /* if num_attackers == 0: check_mask remains 0ULL (unused) */
    /* if num_attackers >= 2: double‐check, no non-king moves allowed (check_mask unused) */
}

static inline __attribute__((always_inline)) void generate_non_capture_pawn_moves(int depth)
{
    Move_List &ML = move_list[depth];
    Move*      buf = ML.moves;
    int&       cnt = ML.count;

    Bitboard ally_pawn_occ = (turn == white) ? piece_occ_bb[white][Pawn] : piece_occ_bb[black][Pawn];
    Bitboard ally_home_pawns = (turn == white) ? ( ally_pawn_occ & Tables::Rank2 ) : ( ally_pawn_occ & Tables::Rank7 );
    Bitboard ally_to_promo_pawns = (turn == white) ? ( ally_pawn_occ & Tables::Rank7 ) : ( ally_pawn_occ & Tables::Rank2 );
    Bitboard all_occ = player_occ_bb[all_color];
    int push_offset = (turn == white) ? -8 : 8;
    Bitboard ally_regular_pawns = ally_pawn_occ & ~ally_to_promo_pawns; // note home pawns can also move 1 step so we include them

    // double push
    while (ally_regular_pawns)
    {
        int ally_pwn_sq = LS1B_IDX(ally_regular_pawns);
        ally_regular_pawns &= ally_regular_pawns - 1;     // clear LS1B

        // get the destination square index
        int to_sq = ally_pwn_sq + push_offset;
        int to_sq2 = ally_pwn_sq + (2 * push_offset);
        uint32_t move_flags = 0ULL;

        //
        Bitboard ally_pwn_occ_bb = (1ULL << ally_pwn_sq);

        // lets create the push1 forward occupancy bitboard
        Bitboard push1_occ = (turn == white) ? ( ally_pwn_occ_bb >> 8 ) : ( ally_pwn_occ_bb << 8 );

        // if this is an empty square
        if( (push1_occ & all_occ) == 0 )
        {
            // now we know the move (single push) is pseudo valid, now we check if it does not leave our king in check
            // if this is non_zero it means this pawn was in the flaged pinned pieces squares
            if(pin_mask & ally_pwn_occ_bb)
            {
                // only valid moves are along the same ray this piece was blocking the enemy attacker
                // pin_ray_mask contains only such moves marked
                // so if this is nonzero we save the move cuz its still blocking the check
                if(pin_ray_mask[ally_pwn_sq] & ( 1ULL << to_sq ))
                {
                    // construct the move and save it
                    buf[cnt++] = Encoder::construct_move(ally_pwn_sq, to_sq, Pawn, Empty, Empty, move_flags);
                }
                else
                    continue;   //  if the move was not in the right direction to keep blocking the check we skip to next pawn (this pawn push leaves king in check)
            }
            // if square was not on pinned pieces then save the move
            else
            {
                // construct the move and save it
                buf[cnt++] = Encoder::construct_move(ally_pwn_sq, to_sq, Pawn, Empty, Empty, move_flags);
            }

            // if candidate for a 2 push (if it was an ally home pawn basically)
            // or in toher words if this AND is not zero
            if(ally_pwn_occ_bb & ally_home_pawns)
            {
                // then lets check the next one
                Bitboard push2_occ = (turn == white) ? ( push1_occ >> 8 ) : ( push1_occ << 8 );

                // if also empty square then move is valid
                // note no need to check for pin here since if the execution gets here for sure this move is not pinned
                // or the move is in the right direction (still blocks)
                if( (push2_occ & all_occ) == 0 )
                {
                    move_flags |= Encoder::FLAG_DOUBLE_PAWN;
                    buf[cnt++] = Encoder::construct_move(ally_pwn_sq, to_sq2, Pawn, Empty, Empty, move_flags);
                }
            }
        }
    }

    //
    while (ally_to_promo_pawns)
    {
        int ally_pwn_sq = LS1B_IDX(ally_to_promo_pawns);
        ally_to_promo_pawns &= ally_to_promo_pawns - 1;     // clear LS1B

        // get the destination square index
        int to_sq = ally_pwn_sq + push_offset;
        // Note we can figure out the flag from the promo piece so we dont need the flag
        //uint32_t move_flags = Encoder::FLAG_PROMOTION;    // not needed

        Bitboard ally_pwn_occ_bb = (1ULL << ally_pwn_sq);

        // lets create the push1 forward occupancy bitboard
        Bitboard push1_occ = (turn == white) ? ( ally_pwn_occ_bb >> 8 ) : ( ally_pwn_occ_bb << 8 );

        // if this is an empty square
        if( (push1_occ & all_occ) == 0 )
        {
            // now we know the move (single push promo) is pseudo valid, now we check if it does not leave our king in check
            // if this is non_zero it means this pawn was in the flaged pinned pieces squares
            if(pin_mask & ally_pwn_occ_bb)
            {
                // only valid moves are along the same ray this piece was blocking the enemy attacker
                // pin_ray_mask contains only such moves marked
                // so if this is nonzero we save the move cuz its still blocking the check
                if(pin_ray_mask[ally_pwn_sq] & ( 1ULL << to_sq ))
                {
                    // construct the move and save it
                    buf[cnt++] = Encoder::construct_move(ally_pwn_sq, to_sq, Pawn, Empty, Queen, 0u);
                    buf[cnt++] = Encoder::construct_move(ally_pwn_sq, to_sq, Pawn, Empty, Rook, 0u);
                    buf[cnt++] = Encoder::construct_move(ally_pwn_sq, to_sq, Pawn, Empty, Bishop, 0u);
                    buf[cnt++] = Encoder::construct_move(ally_pwn_sq, to_sq, Pawn, Empty, Knight, 0u);
                }
                else
                    continue;   //  if the move was not in the right direction to keep blocking the check we skip to next pawn (this pawn push leaves king in check)
            }
            // if square was not on pinned pieces then save the move
            else
            {
                // construct the move and save it
                buf[cnt++] = Encoder::construct_move(ally_pwn_sq, to_sq, Pawn, Empty, Queen, 0u);
                buf[cnt++] = Encoder::construct_move(ally_pwn_sq, to_sq, Pawn, Empty, Rook, 0u);
                buf[cnt++] = Encoder::construct_move(ally_pwn_sq, to_sq, Pawn, Empty, Bishop, 0u);
                buf[cnt++] = Encoder::construct_move(ally_pwn_sq, to_sq, Pawn, Empty, Knight, 0u);
            }
        }
    }
}

static inline __attribute__((always_inline)) void generate_capture_pawn_moves(int depth)
{
    Move_List &ML = move_list[depth];
    Move*      buf = ML.moves;
    int&       cnt = ML.count;

    Bitboard ally_pawn_occ = (turn == white) ? piece_occ_bb[white][Pawn] : piece_occ_bb[black][Pawn];
    Bitboard ally_to_promo_pawns = (turn == white) ? ( ally_pawn_occ & Tables::Rank7 ) : ( ally_pawn_occ & Tables::Rank2 );
    Bitboard ally_regular_pawns = ally_pawn_occ & ~ally_to_promo_pawns;
    Bitboard opp_occ = player_occ_bb[!turn];

    // // non promo captures (including en-passant)
    // while (ally_regular_pawns)
    // {
    //     int ally_pwn_sq = LS1B_IDX(ally_regular_pawns);
    //     ally_regular_pawns &= ally_regular_pawns - 1;     // clear LS1B

    //     Bitboard ally_pwn_occ_bb = 1ULL << ally_pwn_sq;

    //     // get the possible attack bitboard mask
    //     Bitboard possible_pawn_attacks = GET_PAWN_ATTACK(turn, ally_pwn_sq);

    //     if(en_passant == no_sqr)
    //         possible_pawn_attacks &= opp_occ;   // only valid pawn captures land on enemy pieces
    //     else
    //         possible_pawn_attacks &= (opp_occ | ( 1ULL << en_passant ));    // if enpassant is active we add enpassant square to the bitboard mask

    //     // now we traverse the possible attacks and save the moves if they dont leave our king in check
    //     while (possible_pawn_attacks)
    //     {
    //         int move_idx = LS1B_IDX(possible_pawn_attacks);
    //         possible_pawn_attacks &= possible_pawn_attacks - 1;
    //         uint32_t move_flags = 0ULL;

    //         Bitboard move_bb = 1ULL << move_idx;

    //         // we know the move captures an enemy piece so far (pseudo valid)
    //         // so we have to make sure it does not leave our king in check
    //         if(pin_mask & ally_pwn_occ_bb)
    //         {
    //             // if piece is pinned but the move still blocks (along the same pinned ray) then move is valid
    //             if(pin_ray_mask[ally_pwn_sq] & move_bb)
    //             {
    //                 // lets figure out what piece this move is capturing (using our mailbox representation)
    //                 Piece_Type captured = mailbox[!turn][move_idx];

    //                 // en passant is anoyying ==============================================================
    //                 // en_passant captures removes 2 pieces so check mask wont catch it :/
    //                 if (en_passant == move_idx)
    //                 {
    //                     move_flags |= Encoder::FLAG_EN_PASSANT;
    //                     captured = Pawn;

    //                     // precompute the relevant bit-masks
    //                     Bitboard cap_bb    = 1ULL << (move_idx + (turn==white ? 8 : -8));

    //                     // compute the “after” occupancy of *all* pieces
    //                     // remove pawn on from_sq, place pawn on to_sq, remove captured pawn
    //                     Bitboard occ_before = player_occ_bb[all_color];
    //                     Bitboard occ_after  = occ_before
    //                                         ^ ally_pwn_occ_bb       // pawn leaves from_sq
    //                                         ^ move_bb               // pawn arrives at to_sq
    //                                         ^ cap_bb;               // captured pawn disappears

    //                     // test king safety on the new occ
    //                     int king_sq = king_position[turn];
    //                     if (IS_SQUARE_ATTACKED(!turn, occ_after, king_sq)) {
    //                     // illegal EP, skip it
    //                     continue;
    //                     }
    //                 }
    //                 // ==========================================================================================

    //                 buf[cnt++] = Encoder::construct_move(ally_pwn_sq, move_idx, Pawn, captured, Empty, move_flags); ////////////////////////////


    //             }
    //         }
    //         // piece is not pinned so move is valid
    //         else
    //         {
    //             // lets figure out what piece this move is capturing
    //             Piece_Type captured = mailbox[!turn][move_idx];

    //             // en passant is anoyying ==============================================================
    //             // en_passant captures removes 2 pieces so check mask wont catch it :/
    //             if (en_passant == move_idx)
    //             {
    //                 move_flags |= Encoder::FLAG_EN_PASSANT;
    //                 captured = Pawn;

    //                 // precompute the relevant bit-masks
    //                 Bitboard cap_bb    = 1ULL << (move_idx + (turn==white ? 8 : -8));

    //                 // compute the “after” occupancy of *all* pieces
    //                 // remove pawn on from_sq, place pawn on to_sq, remove captured pawn
    //                 Bitboard occ_before = player_occ_bb[all_color];
    //                 Bitboard occ_after  = occ_before
    //                                     ^ ally_pwn_occ_bb       // pawn leaves from_sq
    //                                     ^ move_bb               // pawn arrives at to_sq
    //                                     ^ cap_bb;               // captured pawn disappears

    //                 // test king safety on the new occ
    //                 int king_sq = king_position[turn];
    //                 if (IS_SQUARE_ATTACKED(!turn, occ_after, king_sq)) {
    //                 // illegal EP, skip it
    //                 continue;
    //                 }
    //             }
    //             // ==========================================================================================

    //             buf[cnt++] = Encoder::construct_move(ally_pwn_sq, move_idx, Pawn, captured, Empty, move_flags);

    //         }
    //     }
    // }
    // non-promo pawn captures, branchless ===============================================================================================
    Bitboard occ_before = player_occ_bb[all_color];

    while (ally_regular_pawns) {
        int sq = LS1B_IDX(ally_regular_pawns);
        ally_regular_pawns &= ally_regular_pawns - 1;
        Bitboard pbb = 1ULL<<sq;

        // mask of possible landings (incl. EP square if any)
        Bitboard attack_mask = GET_PAWN_ATTACK(turn, sq)
                            & ( en_passant==no_sqr
                                ? opp_occ
                                : (opp_occ | (1ULL<<en_passant)) );

        while (attack_mask) {
            int to = LS1B_IDX(attack_mask);
            attack_mask &= attack_mask - 1;
            Bitboard to_bb = 1ULL<<to;

            bool  is_ep = (en_passant == to);
            uint32_t flags = is_ep ? Encoder::FLAG_EN_PASSANT : 0u;
            Piece_Type raw_cap = mailbox[!turn][to];
            Piece_Type cap     = is_ep ? Pawn : raw_cap;

            // build move once
            Move M = Encoder::construct_move(sq, to, Pawn, cap, Empty, flags);

            // pin test
            uint64_t ok_pin = (pin_mask & pbb)
                            ? ((pin_ray_mask[sq] & to_bb) != 0)
                            : 1;

            // en-passant safety: remove pawn from sq, add at to, remove captured pawn at cap_sq
            int cap_sq = to + (turn==white ? 8 : -8);
            Bitboard cap_bb = is_ep ? (1ULL<<cap_sq) : 0ULL;
            Bitboard occ_after = occ_before ^ pbb ^ to_bb ^ cap_bb;

            // ok_ep = either not ep, or king is not attacked after
            uint64_t ok_ep = !is_ep | (!IS_SQUARE_ATTACKED(!turn, occ_after, king_position[turn]));

            uint64_t ok = ok_pin & ok_ep;

            cnt += ok & ((buf[cnt] = M), 1);
        }
    }
    // ============================================================================================================================

    // handles promo captures here
    while (ally_to_promo_pawns)
    {
        int ally_pwn_sq = LS1B_IDX(ally_to_promo_pawns);
        ally_to_promo_pawns &= ally_to_promo_pawns - 1;     // clear LS1B

        Bitboard ally_pwn_occ_bb = 1ULL << ally_pwn_sq;

        // get the possible attack bitboard mask
        Bitboard possible_pawn_attacks = GET_PAWN_ATTACK(turn, ally_pwn_sq);

        // no enpassant possible on promoting rank
        possible_pawn_attacks &= opp_occ;   // only valid pawn captures land on enemy pieces

        // now we traverse the possible attacks and save the moves if they dont leave our king in check
        while (possible_pawn_attacks)
        {
            int move_idx = LS1B_IDX(possible_pawn_attacks);
            possible_pawn_attacks &= possible_pawn_attacks - 1;

            Bitboard move_bb = 1ULL << move_idx;

            // we know the move captures an enemy piece so far (pseudo valid)
            // so we have to make sure it does not leave our king in check
            if(pin_mask & ally_pwn_occ_bb)
            {
                // if piece is pinned but the move still blocks (along the same pinned ray) then move is valid
                if(pin_ray_mask[ally_pwn_sq] & move_bb)
                {
                    // lets figure out what piece this move is capturing (using our mailbox representation)
                    Piece_Type captured = mailbox[!turn][move_idx];

                    buf[cnt++] = Encoder::construct_move(ally_pwn_sq, move_idx, Pawn, captured, Queen, 0u);

                    buf[cnt++] = Encoder::construct_move(ally_pwn_sq, move_idx, Pawn, captured, Rook, 0u);

                    buf[cnt++] = Encoder::construct_move(ally_pwn_sq, move_idx, Pawn, captured, Bishop, 0u);

                    buf[cnt++] = Encoder::construct_move(ally_pwn_sq, move_idx, Pawn, captured, Knight, 0u);

                }// buf[cnt++]
            }
            // piece is not pinned so move is valid
            else
            {
                // lets figure out what piece this move is capturing
                Piece_Type captured = mailbox[!turn][move_idx];
                buf[cnt++] = Encoder::construct_move(ally_pwn_sq, move_idx, Pawn, captured, Queen, 0u);

                buf[cnt++] = Encoder::construct_move(ally_pwn_sq, move_idx, Pawn, captured, Rook, 0u);

                buf[cnt++] = Encoder::construct_move(ally_pwn_sq, move_idx, Pawn, captured, Bishop, 0u);

                buf[cnt++] = Encoder::construct_move(ally_pwn_sq, move_idx, Pawn, captured, Knight, 0u);

            }
        }
    }
}

static inline __attribute__((always_inline)) void generate_king_moves(int depth)
{
    Move_List &ML = move_list[depth];
    Move*      buf = ML.moves;
    int&       cnt = ML.count;
    Bitboard opp_king_occ = piece_occ_bb[!turn][King];
    Bitboard opp_occ = player_occ_bb[!turn];
    Bitboard empty_occ = ~player_occ_bb[all_color];
    Bitboard all_occ = player_occ_bb[all_color];
    int king_sq = king_position[turn];

    // get the possible attack bitboard mask
    Bitboard possible_moves = GET_KING_ATTACK(king_sq);

    Bitboard capture_moves = possible_moves & opp_occ;
    Bitboard non_capture_moves = possible_moves & empty_occ;

    //
    while (capture_moves)
    {
        int move_idx = LS1B_IDX(capture_moves);
        capture_moves &= capture_moves - 1;

        Bitboard occ_after = (all_occ ^ (1ULL<<king_sq)) | (1ULL<<move_idx);

        // if enemy king is too close move is not valid
        if(GET_KING_ATTACK(move_idx) & opp_king_occ)
            continue;

        // if king ends in check move is not valid
        if(IS_SQUARE_ATTACKED(!turn, occ_after ,move_idx))
            continue;

        // lets figure out what piece this move is capturing
        Piece_Type captured = mailbox[!turn][move_idx];
        buf[cnt++] = Encoder::construct_move(king_sq, move_idx, King, captured, Empty, 0u);

    }

    // non capture moves
    while (non_capture_moves)
    {
        int move_idx = LS1B_IDX(non_capture_moves);
        non_capture_moves &= non_capture_moves - 1;

        Bitboard occ_after = (all_occ ^ (1ULL<<king_sq)) | (1ULL<<move_idx);

        // if enemy king is too close move is not valid
        if(GET_KING_ATTACK(move_idx) & opp_king_occ)
            continue;

        // if king ends in check move is not valid
        if(IS_SQUARE_ATTACKED(!turn, occ_after ,move_idx))
            continue;


        buf[cnt++] = Encoder::construct_move(king_sq, move_idx, King, Empty, Empty, 0u);

    }
}

static inline __attribute__((always_inline)) void generate_castle_moves(int depth)
{
    Move_List &ML = move_list[depth];
    Move*      buf = ML.moves;
    int&       cnt = ML.count;
    Bitboard all_occ = player_occ_bb[all_color];

    if(turn == white)
    {
        Bitboard KC_squares_mask = Encoder::CASTLE_EMPTY_MASK[white][0];
        Bitboard QC_squares_mask = Encoder::CASTLE_EMPTY_MASK[white][1];

        // king side (if en_passant has the King side castle up and the king side squares for castling are empty)
        // then castling this side is pseudo possible (still need to check the trhough squares for squares controlled by the enemy)
        if( (castle_right & KC) ? ( all_occ & KC_squares_mask ) == 0 : false )
        {
            // if neither f1 or g1 is attacked by enemy pieces then the castle move is valid
            if( !(IS_SQUARE_ATTACKED(black, all_occ, f1)) && !(IS_SQUARE_ATTACKED(black, all_occ, g1)) )
            {
                uint32_t move_flags = 0ULL;
                move_flags |= Encoder::FLAG_CASTLE_KINGSIDE;
                buf[cnt++] = Encoder::construct_move(e1, g1, King, Empty, Empty, move_flags);

            }
        }
        if( (castle_right & QC) ? ( all_occ & QC_squares_mask ) == 0 : false )
        {
            if( !(IS_SQUARE_ATTACKED(black, all_occ, c1)) && !(IS_SQUARE_ATTACKED(black, all_occ, d1)) )
            {
                uint32_t move_flags = 0ULL;
                move_flags |= Encoder::FLAG_CASTLE_QUEENSIDE;
                buf[cnt++] = Encoder::construct_move(e1, c1, King, Empty, Empty, move_flags);

            }
        }
    }
    // black castling
    else
    {
        Bitboard kc_squares_mask = Encoder::CASTLE_EMPTY_MASK[black][0];
        Bitboard qc_squares_mask = Encoder::CASTLE_EMPTY_MASK[black][1];

        // king side (if en_passant has the King side castle up and the king side squares for castling are empty)
        // then castling this side is pseudo possible (still need to check the trhough squares for squares controlled by the enemy)
        if( (castle_right & kc) ? ( all_occ & kc_squares_mask ) == 0 : false )
        {
            // if neither f1 or g1 is attacked by enemy pieces then the castle move is valid
            if( !(IS_SQUARE_ATTACKED(white, all_occ, f8)) && !(IS_SQUARE_ATTACKED(white, all_occ, g8)) )
            {
                uint32_t move_flags = 0ULL;
                move_flags |= Encoder::FLAG_CASTLE_KINGSIDE;
                buf[cnt++] = Encoder::construct_move(e8, g8, King, Empty, Empty, move_flags);

            }
        }
        if( (castle_right & qc) ? ( all_occ & qc_squares_mask ) == 0 : false )
        {
            if( !(IS_SQUARE_ATTACKED(white, all_occ, c8)) && !(IS_SQUARE_ATTACKED(white, all_occ, d8)) )
            {
                uint32_t move_flags = 0ULL;
                move_flags |= Encoder::FLAG_CASTLE_QUEENSIDE;
                buf[cnt++] = Encoder::construct_move(e8, c8, King, Empty, Empty, move_flags);

            }
        }
    }
}

static inline __attribute__((always_inline)) void generate_knight_moves(int depth)
{
    Move_List &ML = move_list[depth];
    Move*      buf = ML.moves;
    int&       cnt = ML.count;
    Bitboard ally_knights_occ = piece_occ_bb[turn][Knight];
    Bitboard opp_occ = player_occ_bb[!turn];
    Bitboard empty_occ = ~player_occ_bb[all_color];

    while (ally_knights_occ)
    {
        int this_knight_sq = LS1B_IDX(ally_knights_occ);
        ally_knights_occ &= ally_knights_occ - 1;     // clear LS1B

        Bitboard this_knight_occ = 1ULL << this_knight_sq;

        // get the possible attack bitboard mask
        Bitboard possible_moves = GET_KNIGHT_ATTACK(this_knight_sq);

        Bitboard capture_moves = possible_moves & opp_occ;
        Bitboard non_capture_moves = possible_moves & empty_occ;

        //
        while (capture_moves)
        {
            int move_idx = LS1B_IDX(capture_moves);
            capture_moves &= capture_moves - 1;

            // we know the move captures an enemy piece so far (pseudo valid)
            // so we have to make sure it does not leave our king in check
            // knihgts cant possible land on the same rank or file or diagonal so if knight is pinned
            // it stays there
            // if(pin_mask & this_knight_occ)
            //     continue;
            // // piece is not pinned so move is valid
            // else
            // {
            //     // lets figure out what piece this move is capturing
            //     Piece_Type captured = mailbox[!turn][move_idx];
            //     buf[cnt++] = Encoder::construct_move(this_knight_sq, move_idx, Knight, captured, Empty, 0u);

            // }

            // NOTE the trick we are using here to go branchless to avoid wasting CPU cycles if missprediciton of the branch(cuz CPU needs to flush the instructions if so)
            // we use the comma operator for expressions in C/C++ --> (a, b) (this translates to evaluate 'a' and make its side effects then discard its value and
            // the resultant expression is the value of 'b')
            Piece_Type captured = mailbox[!turn][move_idx];
            Move m = Encoder::construct_move(this_knight_sq, move_idx, Knight, captured, Empty, 0u);
            cnt += ((pin_mask & this_knight_occ) == 0) & (buf[cnt] = m, 1);
        }

        // non capture moves
        while (non_capture_moves)
        {
            int move_idx = LS1B_IDX(non_capture_moves);
            non_capture_moves &= non_capture_moves - 1;

            Move m = Encoder::construct_move(this_knight_sq, move_idx, Knight, Empty, Empty, 0u);
            cnt += ((pin_mask & this_knight_occ) == 0) & (buf[cnt] = m, 1);
            // if(pin_mask & this_knight_occ)
            //     continue;
            // // piece is not pinned so move is valid
            // else
            // {
            //     buf[cnt++] = Encoder::construct_move(this_knight_sq, move_idx, Knight, Empty, Empty, 0u);

            // }
        }
    }
}

static inline __attribute__((always_inline)) void generate_bishop_moves(int depth)
{
    Move_List &ML = move_list[depth];
    Move*      buf = ML.moves;
    int&       cnt = ML.count;
    Bitboard ally_bishops_occ = piece_occ_bb[turn][Bishop];
    Bitboard opp_occ = player_occ_bb[!turn];
    Bitboard empty_occ = ~player_occ_bb[all_color];
    Bitboard all_occ = player_occ_bb[all_color];

    while (ally_bishops_occ)
    {
        int this_bishop_sq = LS1B_IDX(ally_bishops_occ);
        ally_bishops_occ &= ally_bishops_occ - 1;     // clear LS1B

        Bitboard this_bishop_occ = 1ULL << this_bishop_sq;

        // get the possible attack bitboard mask
        Bitboard possible_moves = GET_BISHOP_ATTACK(all_occ, this_bishop_sq);

        Bitboard capture_moves = possible_moves & opp_occ;
        Bitboard non_capture_moves = possible_moves & empty_occ;

        //
        while (capture_moves)
        {
            int move_idx = LS1B_IDX(capture_moves);
            capture_moves &= capture_moves - 1;
            Bitboard move_bb = 1ULL << move_idx;

            // we know the move captures an enemy piece so far (pseudo valid)
            // so we have to make sure it does not leave our king in check
            // knihgts cant possible land on the same rank or file or diagonal so if knight is pinned
            // it stays there
            // if(pin_mask & this_bishop_occ)
            // {
            //     // if piece is pinned but the move still blocks (along the same pinned ray) then move is valid
            //     if(pin_ray_mask[this_bishop_sq] & move_bb)
            //     {
            //         // lets figure out what piece this move is capturing (using our mailbox representation)
            //         Piece_Type captured = mailbox[!turn][move_idx];
            //         buf[cnt++] = Encoder::construct_move(this_bishop_sq, move_idx, Bishop, captured, Empty, 0u);

            //     }
            // }
            // // piece is not pinned so move is valid
            // else
            // {
            //     // lets figure out what piece this move is capturing
            //     Piece_Type captured = mailbox[!turn][move_idx];
            //     buf[cnt++] = Encoder::construct_move(this_bishop_sq, move_idx, Bishop, captured, Empty, 0u);

            // }

            Piece_Type captured = mailbox[!turn][move_idx];
            Move m = Encoder::construct_move(this_bishop_sq, move_idx, Bishop, captured, Empty, 0u);

            bool allowed = (pin_mask & this_bishop_occ)
                            ? (pin_ray_mask[this_bishop_sq] & move_bb)
                            : 1;

            cnt += allowed & ((buf[cnt] = m), 1);
        }
        // non capture moves
        while (non_capture_moves)
        {
            int move_idx = LS1B_IDX(non_capture_moves);
            non_capture_moves &= non_capture_moves - 1;
            Bitboard move_bb = 1ULL << move_idx;

            Move m = Encoder::construct_move(this_bishop_sq, move_idx, Bishop, Empty, Empty, 0u);

            bool allowed = (pin_mask & this_bishop_occ)
                            ? (pin_ray_mask[this_bishop_sq] & move_bb)
                            : 1;

            cnt += allowed & ((buf[cnt] = m), 1);

            // if(pin_mask & this_bishop_occ)
            // {
            //     if(pin_ray_mask[this_bishop_sq] & move_bb)
            //     {
            //         buf[cnt++] = Encoder::construct_move(this_bishop_sq, move_idx, Bishop, Empty, Empty, 0u);

            //     }
            // }
            // else
            // {
            //     buf[cnt++] = Encoder::construct_move(this_bishop_sq, move_idx, Bishop, Empty, Empty, 0u);

            // }
        }
    }
}

static inline __attribute__((always_inline)) void generate_rook_moves(int depth)
{
    Move_List &ML = move_list[depth];
    Move*      buf = ML.moves;
    int&       cnt = ML.count;
    Bitboard ally_rooks_occ    = piece_occ_bb[turn][Rook];
    Bitboard opp_occ           = player_occ_bb[!turn];
    Bitboard empty_occ         = ~player_occ_bb[all_color];
    Bitboard all_occ           = player_occ_bb[all_color];

    while (ally_rooks_occ)
    {
        int this_rook_sq = LS1B_IDX(ally_rooks_occ);
        ally_rooks_occ &= ally_rooks_occ - 1;

        Bitboard this_rook_occ = 1ULL << this_rook_sq;

        Bitboard possible_moves     = GET_ROOK_ATTACK(all_occ, this_rook_sq);
        Bitboard capture_moves      = possible_moves & opp_occ;
        Bitboard non_capture_moves  = possible_moves & empty_occ;

        while (capture_moves)
        {
            int move_idx = LS1B_IDX(capture_moves);
            capture_moves &= capture_moves - 1;
            Bitboard move_bb = 1ULL << move_idx;

            Piece_Type captured = mailbox[!turn][move_idx];
            Move m = Encoder::construct_move(this_rook_sq, move_idx, Rook, captured, Empty, 0u);

            bool allowed = (pin_mask & this_rook_occ)
                            ? (pin_ray_mask[this_rook_sq] & move_bb)
                            : 1;

            cnt += allowed & ((buf[cnt] = m), 1);

            // if (pin_mask & this_rook_occ)
            // {
            //     if (pin_ray_mask[this_rook_sq] & move_bb)
            //     {
            //         Piece_Type captured = mailbox[!turn][move_idx];
            //         buf[cnt++] = Encoder::construct_move(this_rook_sq, move_idx, Rook, captured, Empty, 0u);

            //     }
            // }
            // else
            // {
            //     Piece_Type captured = mailbox[!turn][move_idx];
            //     buf[cnt++] = Encoder::construct_move(this_rook_sq, move_idx, Rook, captured, Empty, 0u);

            // }
        }

        while (non_capture_moves)
        {
            int move_idx = LS1B_IDX(non_capture_moves);
            non_capture_moves &= non_capture_moves - 1;
            Bitboard move_bb = 1ULL << move_idx;

            Move m = Encoder::construct_move(this_rook_sq, move_idx, Rook, Empty, Empty, 0u);

            bool allowed = (pin_mask & this_rook_occ)
                            ? (pin_ray_mask[this_rook_sq] & move_bb)
                            : 1;

            cnt += allowed & ((buf[cnt] = m), 1);

            // if (pin_mask & this_rook_occ)
            // {
            //     if (pin_ray_mask[this_rook_sq] & move_bb)
            //     {
            //         buf[cnt++] = Encoder::construct_move(this_rook_sq, move_idx, Rook, Empty, Empty, 0u);

            //     }
            // }
            // else
            // {
            //     buf[cnt++] = Encoder::construct_move(this_rook_sq, move_idx, Rook, Empty, Empty, 0u);

            // }
        }
    }
}

static inline __attribute__((always_inline)) void generate_queen_moves(int depth)
{
    Move_List &ML = move_list[depth];
    Move*      buf = ML.moves;
    int&       cnt = ML.count;
    Bitboard ally_queens_occ    = piece_occ_bb[turn][Queen];
    Bitboard opp_occ            = player_occ_bb[!turn];
    Bitboard empty_occ          = ~player_occ_bb[all_color];
    Bitboard all_occ            = player_occ_bb[all_color];

    while (ally_queens_occ)
    {
        int this_queen_sq = LS1B_IDX(ally_queens_occ);
        ally_queens_occ &= ally_queens_occ - 1;

        Bitboard this_queen_occ = 1ULL << this_queen_sq;

        Bitboard possible_moves     = GET_QUEEN_ATTACK(all_occ, this_queen_sq);
        Bitboard capture_moves      = possible_moves & opp_occ;
        Bitboard non_capture_moves  = possible_moves & empty_occ;

        while (capture_moves)
        {
            int move_idx = LS1B_IDX(capture_moves);
            capture_moves &= capture_moves - 1;
            Bitboard move_bb = 1ULL << move_idx;

            Piece_Type captured = mailbox[!turn][move_idx];
            Move m = Encoder::construct_move(this_queen_sq, move_idx, Queen, captured, Empty, 0u);

            // compute a 0/1 “allowed” mask:
            //    - if pinned, only allow when pin_ray_mask lines up
            //    - else always allow
            bool allowed = (pin_mask & this_queen_occ)
                            ? (pin_ray_mask[this_queen_sq] & move_bb)
                            : 1;

            // branch-free store + count increment:
            //    - (buf[cnt] = m),1  does the write then yields 1
            //    - allowed & (…)  is 1 only when allowed==1
            cnt += allowed & ((buf[cnt] = m), 1);

            // if (pin_mask & this_queen_occ)
            // {
            //     if (pin_ray_mask[this_queen_sq] & move_bb)
            //     {
            //         Piece_Type captured = mailbox[!turn][move_idx];
            //         buf[cnt++] = Encoder::construct_move(this_queen_sq, move_idx, Queen, captured, Empty, 0u);

            //     }
            // }
            // else
            // {
            //     Piece_Type captured = mailbox[!turn][move_idx];
            //     buf[cnt++] = Encoder::construct_move(this_queen_sq, move_idx, Queen, captured, Empty, 0u);

            // }
        }

        while (non_capture_moves)
        {
            int move_idx = LS1B_IDX(non_capture_moves);
            non_capture_moves &= non_capture_moves - 1;
            Bitboard move_bb = 1ULL << move_idx;

            Move m = Encoder::construct_move(this_queen_sq, move_idx, Queen, Empty, Empty, 0u);

            bool allowed = (pin_mask & this_queen_occ)
                            ? (pin_ray_mask[this_queen_sq] & move_bb)
                            : 1;

            cnt += allowed & ((buf[cnt] = m), 1);

            // if (pin_mask & this_queen_occ)
            // {
            //     if (pin_ray_mask[this_queen_sq] & move_bb)
            //     {
            //         buf[cnt++] = Encoder::construct_move(this_queen_sq, move_idx, Queen, Empty, Empty, 0u);

            //     }
            // }
            // else
            // {
            //     buf[cnt++] = Encoder::construct_move(this_queen_sq, move_idx, Queen, Empty, Empty, 0u);

            // }
        }
    }
}











// CHECK ===========
// static inline __attribute__((always_inline)) void generate_knight_moves(int depth) {
//     Move_List &ML = move_list[depth];
//     Move* buf = ML.moves;
//     int& cnt = ML.count;
//     Bitboard knights = piece_occ_bb[turn][Knight];
//     Bitboard opp_occ = player_occ_bb[!turn];
//     Bitboard all_occ = player_occ_bb[all_color];
//     Bitboard king_bb = 1ULL << king_position[!turn];
//     while (knights) {
//         int from = LS1B_IDX(knights);
//         knights &= knights - 1;
//         Bitboard from_bb = 1ULL << from;
//         Bitboard moves = GET_KNIGHT_ATTACK(from) & (opp_occ | ~all_occ);
//         while (moves) {
//             int to = LS1B_IDX(moves);
//             moves &= moves - 1;
//             Bitboard to_bb = 1ULL << to;

//             // ok = 1 if legal (not pinned or still along pin ray), else 0
//             uint32_t ok = !((pin_mask & from_bb) && !(pin_ray_mask[from] & to_bb));

//             // direct = 1 if knight at 'to' would check king
//             uint32_t direct = !!(GET_KNIGHT_ATTACK(to) & king_bb);

//             // disc = 1 if moving from 'from' unmasked a slider
//             uint32_t disc = !!(from_bb & discovered_mask);

//             // mask = 0xFFFFFFFF if (direct|disc)==1 else 0
//             uint32_t mask = - (direct | disc);

//             // flags = FLAG_CHECK if mask==0xFFFFFFFF, else 0
//             uint32_t flags = mask & Encoder::FLAG_CHECK;

//             Piece_Type cap = (opp_occ & to_bb) ? mailbox[!turn][to] : Empty;
//             Move m = Encoder::construct_move(from, to, Knight, cap, Empty, flags);

//             cnt += ok & ((buf[cnt] = m), 1);
//         }
//     }
// }

// static inline __attribute__((always_inline)) void generate_bishop_moves(int depth) {
//     Move_List &ML = move_list[depth];
//     Move* buf = ML.moves;
//     int& cnt = ML.count;
//     Bitboard bishops = piece_occ_bb[turn][Bishop];
//     Bitboard opp_occ = player_occ_bb[!turn];
//     Bitboard all_occ = player_occ_bb[all_color];
//     Bitboard king_bb = 1ULL << king_position[!turn];
//     while (bishops) {
//         int from = LS1B_IDX(bishops);
//         bishops &= bishops - 1;
//         Bitboard from_bb = 1ULL << from;
//         Bitboard moves = GET_BISHOP_ATTACK(all_occ, from) & (opp_occ | ~all_occ);
//         while (moves) {
//             int to = LS1B_IDX(moves);
//             moves &= moves - 1;
//             Bitboard to_bb = 1ULL << to;

//             uint32_t ok    = !((pin_mask & from_bb) && !(pin_ray_mask[from] & to_bb));
//             uint32_t direct= !!(GET_BISHOP_ATTACK(all_occ, to) & king_bb);
//             uint32_t disc  = !!((from_bb & discovered_mask) && (discovered_ray_mask[from] & to_bb));
//             uint32_t mask  = - (direct | disc);
//             uint32_t flags = mask & Encoder::FLAG_CHECK;

//             Piece_Type cap = (opp_occ & to_bb) ? mailbox[!turn][to] : Empty;
//             Move m = Encoder::construct_move(from, to, Bishop, cap, Empty, flags);

//             cnt += ok & ((buf[cnt] = m), 1);
//         }
//     }
// }

// static inline __attribute__((always_inline)) void generate_rook_moves(int depth) {
//     Move_List &ML = move_list[depth];
//     Move* buf = ML.moves;
//     int& cnt = ML.count;
//     Bitboard rooks = piece_occ_bb[turn][Rook];
//     Bitboard opp_occ = player_occ_bb[!turn];
//     Bitboard all_occ = player_occ_bb[all_color];
//     Bitboard king_bb = 1ULL << king_position[!turn];
//     while (rooks) {
//         int from = LS1B_IDX(rooks);
//         rooks &= rooks - 1;
//         Bitboard from_bb = 1ULL << from;
//         Bitboard moves = GET_ROOK_ATTACK(all_occ, from) & (opp_occ | ~all_occ);
//         while (moves) {
//             int to = LS1B_IDX(moves);
//             moves &= moves - 1;
//             Bitboard to_bb = 1ULL << to;

//             uint32_t ok    = !((pin_mask & from_bb) && !(pin_ray_mask[from] & to_bb));
//             uint32_t direct= !!(GET_ROOK_ATTACK(all_occ, to) & king_bb);
//             uint32_t disc  = !!((from_bb & discovered_mask) && (discovered_ray_mask[from] & to_bb));
//             uint32_t mask  = - (direct | disc);
//             uint32_t flags = mask & Encoder::FLAG_CHECK;

//             Piece_Type cap = (opp_occ & to_bb) ? mailbox[!turn][to] : Empty;
//             Move m = Encoder::construct_move(from, to, Rook, cap, Empty, flags);

//             cnt += ok & ((buf[cnt] = m), 1);
//         }
//     }
// }

// static inline __attribute__((always_inline)) void generate_queen_moves(int depth) {
//     Move_List &ML = move_list[depth];
//     Move* buf = ML.moves;
//     int& cnt = ML.count;
//     Bitboard queens = piece_occ_bb[turn][Queen];
//     Bitboard opp_occ = player_occ_bb[!turn];
//     Bitboard all_occ = player_occ_bb[all_color];
//     Bitboard king_bb = 1ULL << king_position[!turn];
//     while (queens) {
//         int from = LS1B_IDX(queens);
//         queens &= queens - 1;
//         Bitboard from_bb = 1ULL << from;
//         Bitboard moves = GET_QUEEN_ATTACK(all_occ, from) & (opp_occ | ~all_occ);
//         while (moves) {
//             int to = LS1B_IDX(moves);
//             moves &= moves - 1;
//             Bitboard to_bb = 1ULL << to;

//             uint32_t ok    = !((pin_mask & from_bb) && !(pin_ray_mask[from] & to_bb));
//             uint32_t direct= !!(GET_QUEEN_ATTACK(all_occ, to) & king_bb);
//             uint32_t disc  = !!((from_bb & discovered_mask) && (discovered_ray_mask[from] & to_bb));
//             uint32_t mask  = - (direct | disc);
//             uint32_t flags = mask & Encoder::FLAG_CHECK;

//             Piece_Type cap = (opp_occ & to_bb) ? mailbox[!turn][to] : Empty;
//             Move m = Encoder::construct_move(from, to, Queen, cap, Empty, flags);

//             cnt += ok & ((buf[cnt] = m), 1);
//         }
//     }
// }













// ONLY QUIET CHECKS ===============
// static inline __attribute__((always_inline))
// void generate_knight_moves(int depth)
// {
//     Move_List &ML       = move_list[depth];
//     Move*      buf      = ML.moves;
//     int&       cnt      = ML.count;
//     Bitboard   knights  = piece_occ_bb[turn][Knight];
//     Bitboard   opp_occ  = player_occ_bb[!turn];
//     Bitboard   empty_bb = ~player_occ_bb[all_color];
//     Bitboard   king_bb  = 1ULL << king_position[!turn];

//     while (knights) {
//         int from = LS1B_IDX(knights);
//         knights &= knights - 1;
//         Bitboard from_bb = 1ULL << from;
//         Bitboard attacks = GET_KNIGHT_ATTACK(from);

//         // — captures: no check flags, knight pinned→no move
//         Bitboard caps = attacks & opp_occ;
//         while (caps) {
//             int to = LS1B_IDX(caps);
//             caps &= caps - 1;
//             Piece_Type cap = mailbox[!turn][to];
//             Move m = Encoder::construct_move(from, to, Knight, cap, Empty, 0u);
//             cnt += ((pin_mask & from_bb) == 0) & (buf[cnt] = m, 1);
//         }

//         // — non-captures: branchless check detection, knight pinned→no move
//         Bitboard moves = attacks & empty_bb;
//         while (moves) {
//             int to = LS1B_IDX(moves);
//             moves &= moves - 1;
//             Bitboard to_bb = 1ULL << to;

//             // direct check?
//             uint32_t direct = !!(GET_KNIGHT_ATTACK(to) & king_bb);
//             // discovered?
//             uint32_t disc   = !!(from_bb & discovered_mask);
//             uint32_t flags  = -(direct | disc) & Encoder::FLAG_CHECK;

//             Move m = Encoder::construct_move(from, to, Knight, Empty, Empty, flags);
//             cnt += ((pin_mask & from_bb) == 0) & (buf[cnt] = m, 1);
//         }
//     }
// }

// static inline __attribute__((always_inline))
// void generate_bishop_moves(int depth)
// {
//     Move_List &ML       = move_list[depth];
//     Move*      buf      = ML.moves;
//     int&       cnt      = ML.count;
//     Bitboard   bishops  = piece_occ_bb[turn][Bishop];
//     Bitboard   opp_occ  = player_occ_bb[!turn];
//     Bitboard   empty_bb = ~player_occ_bb[all_color];
//     Bitboard   all_bb   = player_occ_bb[all_color];
//     Bitboard   king_bb  = 1ULL << king_position[!turn];

//     while (bishops) {
//         int from = LS1B_IDX(bishops);
//         bishops &= bishops - 1;
//         Bitboard from_bb = 1ULL << from;
//         Bitboard attacks = GET_BISHOP_ATTACK(all_bb, from);

//         // — captures: no check flags, pinned→only along pin‐ray
//         Bitboard caps = attacks & opp_occ;
//         while (caps) {
//             int to = LS1B_IDX(caps);
//             caps &= caps - 1;
//             Bitboard to_bb = 1ULL << to;

//             Piece_Type cap = mailbox[!turn][to];
//             Move m = Encoder::construct_move(from, to, Bishop, cap, Empty, 0u);

//             cnt += ((pin_mask & from_bb) != 0
//                     ? ((pin_ray_mask[from] & to_bb) != 0)
//                     : 1)
//                    & (buf[cnt] = m, 1);
//         }

//         // — non-captures: branchless check flags, pinned→only along pin‐ray
//         Bitboard moves = attacks & empty_bb;
//         while (moves) {
//             int to = LS1B_IDX(moves);
//             moves &= moves - 1;
//             Bitboard to_bb = 1ULL << to;

//             uint32_t direct = !!(GET_BISHOP_ATTACK(all_bb, to) & king_bb);
//             uint32_t disc   = !!((from_bb & discovered_mask)
//                                && (discovered_ray_mask[from] & to_bb));
//             uint32_t flags  = -(direct | disc) & Encoder::FLAG_CHECK;

//             Move m = Encoder::construct_move(from, to, Bishop, Empty, Empty, flags);
//             cnt += ((pin_mask & from_bb) != 0
//                     ? ((pin_ray_mask[from] & to_bb) != 0)
//                     : 1)
//                    & (buf[cnt] = m, 1);
//         }
//     }
// }

// static inline __attribute__((always_inline))
// void generate_rook_moves(int depth)
// {
//     Move_List &ML       = move_list[depth];
//     Move*      buf      = ML.moves;
//     int&       cnt      = ML.count;
//     Bitboard   rooks    = piece_occ_bb[turn][Rook];
//     Bitboard   opp_occ  = player_occ_bb[!turn];
//     Bitboard   empty_bb = ~player_occ_bb[all_color];
//     Bitboard   all_bb   = player_occ_bb[all_color];
//     Bitboard   king_bb  = 1ULL << king_position[!turn];

//     while (rooks) {
//         int from = LS1B_IDX(rooks);
//         rooks &= rooks - 1;
//         Bitboard from_bb = 1ULL << from;
//         Bitboard attacks = GET_ROOK_ATTACK(all_bb, from);

//         // — captures: no check flags, pinned→only along pin‐ray
//         Bitboard caps = attacks & opp_occ;
//         while (caps) {
//             int to = LS1B_IDX(caps);
//             caps &= caps - 1;
//             Bitboard to_bb = 1ULL << to;

//             Piece_Type cap = mailbox[!turn][to];
//             Move m = Encoder::construct_move(from, to, Rook, cap, Empty, 0u);

//             cnt += ((pin_mask & from_bb) != 0
//                     ? ((pin_ray_mask[from] & to_bb) != 0)
//                     : 1)
//                    & (buf[cnt] = m, 1);
//         }

//         // — non-captures: branchless check flags, pinned→only along pin‐ray
//         Bitboard moves = attacks & empty_bb;
//         while (moves) {
//             int to = LS1B_IDX(moves);
//             moves &= moves - 1;
//             Bitboard to_bb = 1ULL << to;

//             uint32_t direct = !!(GET_ROOK_ATTACK(all_bb, to) & king_bb);
//             uint32_t disc   = !!((from_bb & discovered_mask)
//                                && (discovered_ray_mask[from] & to_bb));
//             uint32_t flags  = -(direct | disc) & Encoder::FLAG_CHECK;

//             Move m = Encoder::construct_move(from, to, Rook, Empty, Empty, flags);
//             cnt += ((pin_mask & from_bb) != 0
//                     ? ((pin_ray_mask[from] & to_bb) != 0)
//                     : 1)
//                    & (buf[cnt] = m, 1);
//         }
//     }
// }

// static inline __attribute__((always_inline))
// void generate_queen_moves(int depth)
// {
//     Move_List &ML       = move_list[depth];
//     Move*      buf      = ML.moves;
//     int&       cnt      = ML.count;
//     Bitboard   queens   = piece_occ_bb[turn][Queen];
//     Bitboard   opp_occ  = player_occ_bb[!turn];
//     Bitboard   empty_bb = ~player_occ_bb[all_color];
//     Bitboard   all_bb   = player_occ_bb[all_color];
//     Bitboard   king_bb  = 1ULL << king_position[!turn];

//     while (queens) {
//         int from = LS1B_IDX(queens);
//         queens &= queens - 1;
//         Bitboard from_bb = 1ULL << from;
//         Bitboard attacks = GET_QUEEN_ATTACK(all_bb, from);

//         // — captures: no check flags, pinned→only along pin‐ray
//         Bitboard caps = attacks & opp_occ;
//         while (caps) {
//             int to = LS1B_IDX(caps);
//             caps &= caps - 1;
//             Bitboard to_bb = 1ULL << to;

//             Piece_Type cap = mailbox[!turn][to];
//             Move m = Encoder::construct_move(from, to, Queen, cap, Empty, 0u);

//             cnt += ((pin_mask & from_bb) != 0
//                     ? ((pin_ray_mask[from] & to_bb) != 0)
//                     : 1)
//                    & (buf[cnt] = m, 1);
//         }

//         // — non-captures: branchless check flags, pinned→only along pin‐ray
//         Bitboard moves = attacks & empty_bb;
//         while (moves) {
//             int to = LS1B_IDX(moves);
//             moves &= moves - 1;
//             Bitboard to_bb = 1ULL << to;

//             uint32_t direct = !!(GET_QUEEN_ATTACK(all_bb, to) & king_bb);
//             uint32_t disc   = !!((from_bb & discovered_mask)
//                                && (discovered_ray_mask[from] & to_bb));
//             uint32_t flags  = -(direct | disc) & Encoder::FLAG_CHECK;

//             Move m = Encoder::construct_move(from, to, Queen, Empty, Empty, flags);
//             cnt += ((pin_mask & from_bb) != 0
//                     ? ((pin_ray_mask[from] & to_bb) != 0)
//                     : 1)
//                    & (buf[cnt] = m, 1);
//         }
//     }
// }







static inline __attribute__((always_inline)) void generate_non_capture_pawn_moves_in_check(int depth)
{
    Move_List &ML = move_list[depth];
    Move*      buf = ML.moves;
    int&       cnt = ML.count;
    Bitboard ally_pawn_occ = (turn == white) ? piece_occ_bb[white][Pawn] : piece_occ_bb[black][Pawn];
    Bitboard ally_home_pawns = (turn == white) ? ( ally_pawn_occ & Tables::Rank2 ) : ( ally_pawn_occ & Tables::Rank7 );
    Bitboard ally_to_promo_pawns = (turn == white) ? ( ally_pawn_occ & Tables::Rank7 ) : ( ally_pawn_occ & Tables::Rank2 );
    Bitboard all_occ = player_occ_bb[all_color];
    int push_offset = (turn == white) ? -8 : 8;
    Bitboard ally_regular_pawns = ally_pawn_occ & ~ally_to_promo_pawns; // note home pawns can also move 1 step so we include them

    // double push
    while (ally_regular_pawns)
    {
        int ally_pwn_sq = LS1B_IDX(ally_regular_pawns);
        ally_regular_pawns &= ally_regular_pawns - 1;     // clear LS1B

        // get the destination square index
        int to_sq = ally_pwn_sq + push_offset;
        int to_sq2 = ally_pwn_sq + (2 * push_offset);
        uint32_t move_flags = 0ULL;
        //bool push2 = true;  bool push1 = true;

        // lets create the push1 forward occupancy bitboard
        // Bitboard push1_occ = (turn == white) ? ( ally_pwn_occ_bb >> 8 ) : ( ally_pwn_occ_bb << 8 );
        Bitboard push1_occ = (1ULL << to_sq);
        Bitboard push2_occ = (1ULL << to_sq2);

        // check if either push 1 or push 2 is allowed (do they block the king who is currently in check?)
        //if( (check_mask & push1_occ) == 0 )
        //    push1 = false;

        // if( (check_mask & push2_occ) == 0 )
        //     push2 = false;

        // if this is an empty square
        if( (push1_occ & all_occ) == 0 )
        {

            Bitboard ally_pwn_occ_bb = 1ULL << ally_pwn_sq;


            if( (check_mask & push1_occ) == 0 )
            {
                // do nothing, move does not block the king check attack
            }
            // now we know the move (single push) is pseudo valid (and blocks the enemy check), now we check if it does not leave our king in check
            // if this is non_zero it means this pawn was in the flaged pinned pieces squares
            else if(pin_mask & ally_pwn_occ_bb)
            {
                // only valid moves are along the same ray this piece was blocking the enemy attacker
                // pin_ray_mask contains only such moves marked
                // so if this is nonzero we save the move cuz its still blocking the check
                if(pin_ray_mask[ally_pwn_sq] & ( 1ULL << to_sq ))
                {
                    // construct the move and save it
                    buf[cnt++] = Encoder::construct_move(ally_pwn_sq, to_sq, Pawn, Empty, Empty, move_flags);

                }
                else
                    continue;   //  if the move was not in the right direction to keep blocking the check we skip to next pawn (this pawn push leaves king in check)
            }
            // if square was not on pinned pieces then save the move
            else
            {
                // construct the move and save it
                buf[cnt++] = Encoder::construct_move(ally_pwn_sq, to_sq, Pawn, Empty, Empty, move_flags);

            }

            // if move does not block we continue
            if( (check_mask & push2_occ) == 0 ) continue;

            // if candidate for a 2 push (if it was an ally home pawn basically)
            // or in toher words if this AND is not zero
            if(ally_pwn_occ_bb & ally_home_pawns)
            {
                // then lets check the next one
                //Bitboard push2_occ = (turn == white) ? ( push1_occ >> 8 ) : ( push1_occ << 8 );

                // if also empty square then move is valid
                // note no need to check for pin here since if the execution gets here for sure this move is not pinned
                // or the move is in the right direction (still blocks)
                if( (push2_occ & all_occ) == 0 )
                {
                    move_flags |= Encoder::FLAG_DOUBLE_PAWN;
                    buf[cnt++] = Encoder::construct_move(ally_pwn_sq, to_sq2, Pawn, Empty, Empty, move_flags);

                }
            }
        }
    }

    //
    while (ally_to_promo_pawns)
    {
        int ally_pwn_sq = LS1B_IDX(ally_to_promo_pawns);
        ally_to_promo_pawns &= ally_to_promo_pawns - 1;     // clear LS1B

        // get the destination square index
        int to_sq = ally_pwn_sq + push_offset;
        // Note we can figure out the flag from the promo piece so we dont need the flag
        //uint32_t move_flags = Encoder::FLAG_PROMOTION;    // not needed

        // lets create the push1 forward occupancy bitboard
        //Bitboard push1_occ = (turn == white) ? ( ally_pwn_occ_bb >> 8 ) : ( ally_pwn_occ_bb << 8 );
        Bitboard push1_occ =  ( 1ULL << to_sq );

         // if move does not land on the ray mask (the squares must be blocked)
        // pawn cant capture forwards so we must account for it too
        // so even if the move lands on the ray mask but it also lands on enemy occupancy bitboard then we know this is the square the attacker is on
        // thus we cant move a pawn "forward" there
        // NOTE we already checked for this
        if( (check_mask & push1_occ) == 0 )
            continue;

        // if this is an empty square
        if( (push1_occ & all_occ) == 0 )
        {
            Bitboard ally_pwn_occ_bb = 1ULL << ally_pwn_sq;

            // now we know the move (single push promo) is pseudo valid, now we check if it does not leave our king in check
            // if this is non_zero it means this pawn was in the flaged pinned pieces squares
            if(pin_mask & ally_pwn_occ_bb)
            {
                // only valid moves are along the same ray this piece was blocking the enemy attacker
                // pin_ray_mask contains only such moves marked
                // so if this is nonzero we save the move cuz its still blocking the check
                if(pin_ray_mask[ally_pwn_sq] & ( 1ULL << to_sq ))
                {
                    // construct the move and save it
                    buf[cnt++] = Encoder::construct_move(ally_pwn_sq, to_sq, Pawn, Empty, Queen, 0u);

                    buf[cnt++] = Encoder::construct_move(ally_pwn_sq, to_sq, Pawn, Empty, Rook, 0u);

                    buf[cnt++] = Encoder::construct_move(ally_pwn_sq, to_sq, Pawn, Empty, Bishop, 0u);

                    buf[cnt++] = Encoder::construct_move(ally_pwn_sq, to_sq, Pawn, Empty, Knight, 0u);

                }
                else
                    continue;   //  if the move was not in the right direction to keep blocking the check we skip to next pawn (this pawn push leaves king in check)
            }
            // if square was not on pinned pieces then save the move
            else
            {
                // construct the move and save it
                buf[cnt++] = Encoder::construct_move(ally_pwn_sq, to_sq, Pawn, Empty, Queen, 0u);

                buf[cnt++] = Encoder::construct_move(ally_pwn_sq, to_sq, Pawn, Empty, Rook, 0u);

                buf[cnt++] = Encoder::construct_move(ally_pwn_sq, to_sq, Pawn, Empty, Bishop, 0u);

                buf[cnt++] = Encoder::construct_move(ally_pwn_sq, to_sq, Pawn, Empty, Knight, 0u);

            }
        }
    }
}

static inline __attribute__((always_inline)) void generate_capture_pawn_moves_in_check(int depth)
{
    Move_List &ML = move_list[depth];
    Move*      buf = ML.moves;
    int&       cnt = ML.count;
    Bitboard ally_pawn_occ = (turn == white) ? piece_occ_bb[white][Pawn] : piece_occ_bb[black][Pawn];
    Bitboard ally_to_promo_pawns = (turn == white) ? ( ally_pawn_occ & Tables::Rank7 ) : ( ally_pawn_occ & Tables::Rank2 );
    Bitboard ally_regular_pawns = ally_pawn_occ & ~ally_to_promo_pawns;
    Bitboard opp_occ = player_occ_bb[!turn];

    // non promo captures (including en-passant)
    while (ally_regular_pawns)
    {
        int ally_pwn_sq = LS1B_IDX(ally_regular_pawns);
        ally_regular_pawns &= ally_regular_pawns - 1;     // clear LS1B

        Bitboard ally_pwn_occ_bb = 1ULL << ally_pwn_sq;

        // get the possible attack bitboard mask
        Bitboard possible_pawn_attacks = GET_PAWN_ATTACK(turn, ally_pwn_sq);

        if(en_passant == no_sqr)
            possible_pawn_attacks &= opp_occ;   // only valid pawn captures land on enemy pieces
        else
            possible_pawn_attacks &= (opp_occ | ( 1ULL << en_passant ));    // if enpassant is active we add enpassant square to the bitboard mask

        // now we traverse the possible attacks and save the moves if they dont leave our king in check
        while (possible_pawn_attacks)
        {
            int move_idx = LS1B_IDX(possible_pawn_attacks);
            possible_pawn_attacks &= possible_pawn_attacks - 1;
            uint32_t move_flags = 0ULL;

            Bitboard move_bb = 1ULL << move_idx;

            // if move does not block or captue the enemy piece attacking our king then skip
            if((move_bb & check_mask) == 0)
                continue;

            // we know the move captures an enemy piece so far (pseudo valid)
            // so we have to make sure it does not leave our king in check
            if(pin_mask & ally_pwn_occ_bb)
            {
                // if piece is pinned but the move still blocks (along the same pinned ray) then move is valid
                if(pin_ray_mask[ally_pwn_sq] & move_bb)
                {
                    // lets figure out what piece this move is capturing (using our mailbox representation)
                    Piece_Type captured = mailbox[!turn][move_idx];

                    // en passant is anoyying ==============================================================
                    // en_passant captures removes 2 pieces so check mask wont catch it :/
                    if (en_passant == move_idx)
                    {
                        move_flags |= Encoder::FLAG_EN_PASSANT;
                        captured = Pawn;

                        // precompute the relevant bit-masks
                        Bitboard cap_bb    = 1ULL << (move_idx + (turn==white ? 8 : -8));

                        // compute the “after” occupancy of *all* pieces
                        // remove pawn on from_sq, place pawn on to_sq, remove captured pawn
                        Bitboard occ_before = player_occ_bb[all_color];
                        Bitboard occ_after  = occ_before
                                            ^ ally_pwn_occ_bb       // pawn leaves from_sq
                                            ^ move_bb               // pawn arrives at to_sq
                                            ^ cap_bb;               // captured pawn disappears

                        // test king safety on the new occ
                        int king_sq = king_position[turn];
                        if (IS_SQUARE_ATTACKED(!turn, occ_after, king_sq)) {
                        // illegal EP, skip it
                        continue;
                        }
                    }
                    // ==========================================================================================

                    buf[cnt++] = Encoder::construct_move(ally_pwn_sq, move_idx, Pawn, captured, Empty, move_flags);

                }
            }
            // piece is not pinned so move is valid
            else
            {
                // lets figure out what piece this move is capturing (using our mailbox representation)
                Piece_Type captured = mailbox[!turn][move_idx];

                // en passant is anoyying ==============================================================
                // en_passant captures removes 2 pieces so check mask wont catch it :/
                if (en_passant == move_idx)
                {
                    move_flags |= Encoder::FLAG_EN_PASSANT;
                    captured = Pawn;

                    // precompute the relevant bit-masks
                    Bitboard cap_bb    = 1ULL << (move_idx + (turn==white ? 8 : -8));

                    // compute the “after” occupancy of *all* pieces
                    // remove pawn on from_sq, place pawn on to_sq, remove captured pawn
                    Bitboard occ_before = player_occ_bb[all_color];
                    Bitboard occ_after  = occ_before
                                        ^ ally_pwn_occ_bb       // pawn leaves from_sq
                                        ^ move_bb               // pawn arrives at to_sq
                                        ^ cap_bb;               // captured pawn disappears

                    // test king safety on the new occ
                    int king_sq = king_position[turn];
                    if (IS_SQUARE_ATTACKED(!turn, occ_after, king_sq)) {
                    // illegal EP, skip it
                    continue;
                    }
                }
                // ==========================================================================================

                buf[cnt++] = Encoder::construct_move(ally_pwn_sq, move_idx, Pawn, captured, Empty, move_flags);

            }
        }
    }

    // handles promo captures here
    while (ally_to_promo_pawns)
    {
        int ally_pwn_sq = LS1B_IDX(ally_to_promo_pawns);
        ally_to_promo_pawns &= ally_to_promo_pawns - 1;     // clear LS1B

        Bitboard ally_pwn_occ_bb = 1ULL << ally_pwn_sq;

        // get the possible attack bitboard mask
        Bitboard possible_pawn_attacks = GET_PAWN_ATTACK(turn, ally_pwn_sq);

        // no enpassant possible on promoting rank
        possible_pawn_attacks &= opp_occ;   // only valid pawn captures land on enemy pieces

        // now we traverse the possible attacks and save the moves if they dont leave our king in check
        while (possible_pawn_attacks)
        {
            int move_idx = LS1B_IDX(possible_pawn_attacks);
            possible_pawn_attacks &= possible_pawn_attacks - 1;

            Bitboard move_bb = 1ULL << move_idx;

            // if move does not block or captue the enemy piece attacking our king then skip
            if((move_bb & check_mask) == 0)
                continue;

            // we know the move captures an enemy piece so far (pseudo valid)
            // so we have to make sure it does not leave our king in check
            if(pin_mask & ally_pwn_occ_bb)
            {
                // if piece is pinned but the move still blocks (along the same pinned ray) then move is valid
                if(pin_ray_mask[ally_pwn_sq] & move_bb)
                {
                    // lets figure out what piece this move is capturing (using our mailbox representation)
                    Piece_Type captured = mailbox[!turn][move_idx];

                    buf[cnt++] = Encoder::construct_move(ally_pwn_sq, move_idx, Pawn, captured, Queen, 0u);

                    buf[cnt++] = Encoder::construct_move(ally_pwn_sq, move_idx, Pawn, captured, Rook, 0u);

                    buf[cnt++] = Encoder::construct_move(ally_pwn_sq, move_idx, Pawn, captured, Bishop, 0u);

                    buf[cnt++] = Encoder::construct_move(ally_pwn_sq, move_idx, Pawn, captured, Knight, 0u);

                }
            }
            // piece is not pinned so move is valid
            else
            {
                // lets figure out what piece this move is capturing
                Piece_Type captured = mailbox[!turn][move_idx];
                buf[cnt++] = Encoder::construct_move(ally_pwn_sq, move_idx, Pawn, captured, Queen, 0u);

                buf[cnt++] = Encoder::construct_move(ally_pwn_sq, move_idx, Pawn, captured, Rook, 0u);

                buf[cnt++] = Encoder::construct_move(ally_pwn_sq, move_idx, Pawn, captured, Bishop, 0u);

                buf[cnt++] = Encoder::construct_move(ally_pwn_sq, move_idx, Pawn, captured, Knight, 0u);

            }
        }
    }
}

static inline __attribute__((always_inline)) void generate_king_moves_single_attacker(int depth)
{
    Move_List &ML = move_list[depth];
    Move*      buf = ML.moves;
    int&       cnt = ML.count;
    Bitboard opp_king_occ = piece_occ_bb[!turn][King];
    Bitboard opp_occ = player_occ_bb[!turn];
    Bitboard empty_occ = ~player_occ_bb[all_color];
    Bitboard all_occ = player_occ_bb[all_color];
    int king_sq = king_position[turn];

    // get the possible attack bitboard mask
    Bitboard possible_moves = GET_KING_ATTACK(king_sq);

    Bitboard capture_moves = possible_moves & opp_occ;
    Bitboard non_capture_moves = possible_moves & empty_occ;

    //
    while (capture_moves)
    {
        int move_idx = LS1B_IDX(capture_moves);
        capture_moves &= capture_moves - 1;

        Bitboard move_bb = 1ULL << move_idx;

        // i dont see how to shorcircuit here tho

        Bitboard occ_after = (all_occ ^ (1ULL<<king_sq)) | (1ULL<<move_idx);
        // if king ends in check move is not valid
        if(IS_SQUARE_ATTACKED(!turn, occ_after ,move_idx))
            continue;

        // if enemy king is too close move is not valid
        if(GET_KING_ATTACK(move_idx) & opp_king_occ)
            continue;

        // lets figure out what piece this move is capturing
        Piece_Type captured = mailbox[!turn][move_idx];
        buf[cnt++] = Encoder::construct_move(king_sq, move_idx, King, captured, Empty, 0u);

    }

    // non capture moves
    while (non_capture_moves)
    {
        int move_idx = LS1B_IDX(non_capture_moves);
        non_capture_moves &= non_capture_moves - 1;

        Bitboard move_bb = 1ULL << move_idx;
        // if move does not captue the enemy piece attacking our king then skip
        // NOTE here is non_capture_moves so we know for sure that if move is on the ray
        // attacked by the enemy piece then this move is not valid, NOTE yes it could
        // be the square the piece attaking our king lives on since thats included you may ask,
        // but not really cuz we know this is a NON cpture move
        if((move_bb & check_mask) == 1)
            continue;

        Bitboard occ_after = (all_occ ^ (1ULL<<king_sq)) | (1ULL<<move_idx);
        // if king ends in check move is not valid
        if(IS_SQUARE_ATTACKED(!turn, occ_after ,move_idx))
            continue;

        // if enemy king is too close move is not valid
        if(GET_KING_ATTACK(move_idx) & opp_king_occ)
            continue;

        buf[cnt++] = Encoder::construct_move(king_sq, move_idx, King, Empty, Empty, 0u);

    }
}

static inline __attribute__((always_inline)) void generate_knight_moves_in_check(int depth)
{
    Move_List &ML = move_list[depth];
    Move*      buf = ML.moves;
    int&       cnt = ML.count;
    Bitboard ally_knights_occ = piece_occ_bb[turn][Knight];
    Bitboard opp_occ = player_occ_bb[!turn];
    Bitboard empty_occ = ~player_occ_bb[all_color];

    while (ally_knights_occ)
    {
        int this_knight_sq = LS1B_IDX(ally_knights_occ);
        ally_knights_occ &= ally_knights_occ - 1;     // clear LS1B

        Bitboard this_knight_occ = 1ULL << this_knight_sq;

        // get the possible attack bitboard mask
        Bitboard possible_moves = GET_KNIGHT_ATTACK(this_knight_sq);

        Bitboard capture_moves = possible_moves & opp_occ;
        Bitboard non_capture_moves = possible_moves & empty_occ;

        //
        while (capture_moves)
        {
            int move_idx = LS1B_IDX(capture_moves);
            capture_moves &= capture_moves - 1;

            Bitboard move_bb = 1ULL << move_idx;
            // if move does not block or captue the enemy piece attacking our king then skip
            if((move_bb & check_mask) == 0)
                continue;

            // we know the move captures an enemy piece so far (pseudo valid)
            // so we have to make sure it does not leave our king in check
            // knihgts cant possible land on the same rank or file or diagonal so if knight is pinned
            // it stays there
            if(pin_mask & this_knight_occ)
                continue;
            // piece is not pinned so move is valid
            else
            {
                // lets figure out what piece this move is capturing
                Piece_Type captured = mailbox[!turn][move_idx];
                buf[cnt++] = Encoder::construct_move(this_knight_sq, move_idx, Knight, captured, Empty, 0u);

            }
        }

        // non capture moves
        while (non_capture_moves)
        {
            int move_idx = LS1B_IDX(non_capture_moves);
            non_capture_moves &= non_capture_moves - 1;
            Bitboard move_bb = 1ULL << move_idx;
            // if move does not block or captue the enemy piece attacking our king then skip
            if((move_bb & check_mask) == 0)
                continue;
            if(pin_mask & this_knight_occ)
                continue;
            // piece is not pinned so move is valid
            else
            {
                buf[cnt++] = Encoder::construct_move(this_knight_sq, move_idx, Knight, Empty, Empty, 0u);

            }
        }
    }
}

static inline __attribute__((always_inline)) void generate_bishop_moves_in_check(int depth)
{
    Move_List &ML = move_list[depth];
    Move*      buf = ML.moves;
    int&       cnt = ML.count;
    Bitboard ally_bishops_occ = piece_occ_bb[turn][Bishop];
    Bitboard opp_occ = player_occ_bb[!turn];
    Bitboard empty_occ = ~player_occ_bb[all_color];
    Bitboard all_occ = player_occ_bb[all_color];

    while (ally_bishops_occ)
    {
        int this_bishop_sq = LS1B_IDX(ally_bishops_occ);
        ally_bishops_occ &= ally_bishops_occ - 1;     // clear LS1B

        Bitboard this_bishop_occ = 1ULL << this_bishop_sq;

        // get the possible attack bitboard mask
        Bitboard possible_moves = GET_BISHOP_ATTACK(all_occ, this_bishop_sq);

        Bitboard capture_moves = possible_moves & opp_occ;
        Bitboard non_capture_moves = possible_moves & empty_occ;

        //
        while (capture_moves)
        {
            int move_idx = LS1B_IDX(capture_moves);
            capture_moves &= capture_moves - 1;
            Bitboard move_bb = 1ULL << move_idx;
            // if move does not block or captue the enemy piece attacking our king then skip
            if((move_bb & check_mask) == 0)
                continue;

            // we know the move captures an enemy piece so far (pseudo valid)
            // so we have to make sure it does not leave our king in check
            // knihgts cant possible land on the same rank or file or diagonal so if knight is pinned
            // it stays there
            if(pin_mask & this_bishop_occ)
            {
                // if piece is pinned but the move still blocks (along the same pinned ray) then move is valid
                if(pin_ray_mask[this_bishop_sq] & move_bb)
                {
                    // lets figure out what piece this move is capturing (using our mailbox representation)
                    Piece_Type captured = mailbox[!turn][move_idx];
                    buf[cnt++] = Encoder::construct_move(this_bishop_sq, move_idx, Bishop, captured, Empty, 0u);

                }
            }
            // piece is not pinned so move is valid
            else
            {
                // lets figure out what piece this move is capturing
                Piece_Type captured = mailbox[!turn][move_idx];
                buf[cnt++] = Encoder::construct_move(this_bishop_sq, move_idx, Bishop, captured, Empty, 0u);

            }
        }
        // non capture moves
        while (non_capture_moves)
        {
            int move_idx = LS1B_IDX(non_capture_moves);
            non_capture_moves &= non_capture_moves - 1;
            Bitboard move_bb = 1ULL << move_idx;
            // if move does not block or captue the enemy piece attacking our king then skip
            if((move_bb & check_mask) == 0)
                continue;
            if(pin_mask & this_bishop_occ)
            {
                if(pin_ray_mask[this_bishop_sq] & move_bb)
                {
                    buf[cnt++] = Encoder::construct_move(this_bishop_sq, move_idx, Bishop, Empty, Empty, 0u);

                }
            }
            else
            {
                buf[cnt++] = Encoder::construct_move(this_bishop_sq, move_idx, Bishop, Empty, Empty, 0u);

            }
        }
    }
}

static inline __attribute__((always_inline)) void generate_rook_moves_in_check(int depth)
{
    Move_List &ML = move_list[depth];
    Move*      buf = ML.moves;
    int&       cnt = ML.count;
    Bitboard ally_rooks_occ    = piece_occ_bb[turn][Rook];
    Bitboard opp_occ           = player_occ_bb[!turn];
    Bitboard empty_occ         = ~player_occ_bb[all_color];
    Bitboard all_occ           = player_occ_bb[all_color];

    while (ally_rooks_occ)
    {
        int this_rook_sq = LS1B_IDX(ally_rooks_occ);
        ally_rooks_occ &= ally_rooks_occ - 1;

        Bitboard this_rook_occ = 1ULL << this_rook_sq;

        Bitboard possible_moves     = GET_ROOK_ATTACK(all_occ, this_rook_sq);
        Bitboard capture_moves      = possible_moves & opp_occ;
        Bitboard non_capture_moves  = possible_moves & empty_occ;

        // while (capture_moves)
        // {
        //     int move_idx = LS1B_IDX(capture_moves);
        //     capture_moves &= capture_moves - 1;
        //     Bitboard move_bb = 1ULL << move_idx;
        //     // if move does not block or captue the enemy piece attacking our king then skip
        //     if((move_bb & check_mask) == 0)
        //         continue;

        //     if (pin_mask & this_rook_occ)
        //     {
        //         if (pin_ray_mask[this_rook_sq] & move_bb)
        //         {
        //             Piece_Type captured = mailbox[!turn][move_idx];
        //             buf[cnt++] = Encoder::construct_move(this_rook_sq, move_idx, Rook, captured, Empty, 0u);

        //         }
        //     }
        //     else
        //     {
        //         Piece_Type captured = mailbox[!turn][move_idx];
        //         buf[cnt++] = Encoder::construct_move(this_rook_sq, move_idx, Rook, captured, Empty, 0u);

        //     }
        // }
        // branchless capture rook moves
        while (capture_moves) {
            int to = LS1B_IDX(capture_moves);
            capture_moves &= capture_moves - 1;
            Bitboard to_bb = 1ULL << to;

            // build the move unconditionally
            Piece_Type cap = mailbox[!turn][to];
            Move M = Encoder::construct_move(this_rook_sq, to, Rook, cap, Empty, 0u);

            // must block/resolve check
            uint64_t ok_check = ((to_bb & check_mask) != 0);

            // pin constraint: either not pinned, or pinned-but-still-on-the-ray
            uint64_t ok_pin = (pin_mask & this_rook_occ)
                            ? ((pin_ray_mask[this_rook_sq] & to_bb) != 0)
                            : 1;

            uint64_t ok = ok_check & ok_pin;

            // only write & count when ok==1
            cnt += ok & ((buf[cnt] = M), 1);
        }

        // while (non_capture_moves)
        // {
        //     int move_idx = LS1B_IDX(non_capture_moves);
        //     non_capture_moves &= non_capture_moves - 1;
        //     Bitboard move_bb = 1ULL << move_idx;
        //     // if move does not block or captue the enemy piece attacking our king then skip
        //     if((move_bb & check_mask) == 0)
        //         continue;

        //     if (pin_mask & this_rook_occ)
        //     {
        //         if (pin_ray_mask[this_rook_sq] & move_bb)
        //         {
        //             buf[cnt++] = Encoder::construct_move(this_rook_sq, move_idx, Rook, Empty, Empty, 0u);

        //         }
        //     }
        //     else
        //     {
        //         buf[cnt++] = Encoder::construct_move(this_rook_sq, move_idx, Rook, Empty, Empty, 0u);

        //     }
        // }
        // branchless non-capture rook moves
        while (non_capture_moves) {
            int to = LS1B_IDX(non_capture_moves);
            non_capture_moves &= non_capture_moves - 1;
            Bitboard to_bb = 1ULL << to;

            // build the move unconditionally
            Move M = Encoder::construct_move(this_rook_sq, to, Rook, Empty, Empty, 0u);

            // must block/resolve check
            uint64_t ok_check = ((to_bb & check_mask) != 0);

            // pin constraint: either not pinned, or pinned-but-still-on-the-ray
            uint64_t ok_pin = (pin_mask & this_rook_occ)
                            ? ((pin_ray_mask[this_rook_sq] & to_bb) != 0)
                            : 1;

            uint64_t ok = ok_check & ok_pin;

            // only write & count when ok==1
            cnt += ok & ((buf[cnt] = M), 1);
        }
    }
}

static inline __attribute__((always_inline)) void generate_queen_moves_in_check(int depth)
{
    Move_List &ML = move_list[depth];
    Move*      buf = ML.moves;
    int&       cnt = ML.count;

    Bitboard ally_queens_occ    = piece_occ_bb[turn][Queen];
    Bitboard opp_occ            = player_occ_bb[!turn];
    Bitboard empty_occ          = ~player_occ_bb[all_color];
    Bitboard all_occ            = player_occ_bb[all_color];

    while (ally_queens_occ)
    {
        int this_queen_sq = LS1B_IDX(ally_queens_occ);
        ally_queens_occ &= ally_queens_occ - 1;

        Bitboard this_queen_occ = 1ULL << this_queen_sq;

        Bitboard possible_moves     = GET_QUEEN_ATTACK(all_occ, this_queen_sq);
        Bitboard capture_moves      = possible_moves & opp_occ;
        Bitboard non_capture_moves  = possible_moves & empty_occ;

        // while (capture_moves)
        // {
        //     int move_idx = LS1B_IDX(capture_moves);
        //     capture_moves &= capture_moves - 1;
        //     Bitboard move_bb = 1ULL << move_idx;
        //     // if move does not block or captue the enemy piece attacking our king then skip
        //     if((move_bb & check_mask) == 0)
        //         continue;

        //     if (pin_mask & this_queen_occ)
        //     {
        //         if (pin_ray_mask[this_queen_sq] & move_bb)
        //         {
        //             Piece_Type captured = mailbox[!turn][move_idx];
        //             buf[cnt++] = Encoder::construct_move(this_queen_sq, move_idx, Queen, captured, Empty, 0u);

        //         }
        //     }
        //     else
        //     {
        //         Piece_Type captured = mailbox[!turn][move_idx];
        //         buf[cnt++] = Encoder::construct_move(this_queen_sq, move_idx, Queen, captured, Empty, 0u);

        //     }
        // }
        // branchless capture queen moves
        while (capture_moves) {
            int to = LS1B_IDX(capture_moves);
            capture_moves &= capture_moves - 1;
            Bitboard move_bb = 1ULL << to;

            // build the move once
            Piece_Type cap = mailbox[!turn][to];
            Move M = Encoder::construct_move(this_queen_sq, to, Queen, cap, Empty, 0u);

            // must block/resolve check
            uint64_t ok_check = ((move_bb & check_mask) != 0);

            // pin constraint
            uint64_t ok_pin = (pin_mask & this_queen_occ)
                            ? ((pin_ray_mask[this_queen_sq] & move_bb) != 0)
                            : 1;

            uint64_t ok = ok_check & ok_pin;

            // store+count only if ok==1
            cnt += ok & ((buf[cnt] = M), 1);
        }

        // while (non_capture_moves)
        // {
        //     int move_idx = LS1B_IDX(non_capture_moves);
        //     non_capture_moves &= non_capture_moves - 1;
        //     Bitboard move_bb = 1ULL << move_idx;
        //     // if move does not block or captue the enemy piece attacking our king then skip
        //     if((move_bb & check_mask) == 0)
        //         continue;

        //     if (pin_mask & this_queen_occ)
        //     {
        //         if (pin_ray_mask[this_queen_sq] & move_bb)
        //         {
        //             buf[cnt++] = Encoder::construct_move(this_queen_sq, move_idx, Queen, Empty, Empty, 0u);

        //         }
        //     }
        //     else
        //     {
        //         buf[cnt++] = Encoder::construct_move(this_queen_sq, move_idx, Queen, Empty, Empty, 0u);

        //     }
        // }
        // branch-free non-capture queen moves
        while (non_capture_moves) {
            int to = LS1B_IDX(non_capture_moves);
            non_capture_moves &= non_capture_moves - 1;
            Bitboard to_bb = 1ULL << to;

            // prebuild the move
            Move M = Encoder::construct_move(this_queen_sq, to, Queen, Empty, Empty, 0u);

            // does it block the check ray?
            uint64_t ok_check = ((to_bb & check_mask) != 0);

            // pin test
            uint64_t ok_pin = (pin_mask & this_queen_occ)
                            ? ((pin_ray_mask[this_queen_sq] & to_bb) != 0)
                            : 1;

            uint64_t ok = ok_check & ok_pin;

            cnt += ok & ((buf[cnt] = M), 1);
        }
    }
}




static inline __attribute__((always_inline)) void generate_moves(int depth)
{
    // zero the move count
    move_list[depth].count = 0;
    pin_mask = 0ULL;

    //
    generate_check_mask();

    // unblckable check
    if(num_attackers > 1)
    {
        generate_king_moves(depth);

        // if move count is zero here this is check mate
    }
    // blockable check
    else if(num_attackers == 1)
    {
        generate_pin_mask();
        generate_capture_pawn_moves_in_check(depth);
        generate_knight_moves_in_check(depth);
        generate_bishop_moves_in_check(depth);
        generate_rook_moves_in_check(depth);
        generate_queen_moves_in_check(depth);
        generate_non_capture_pawn_moves_in_check(depth);
        generate_king_moves_single_attacker(depth);

        // if move count is zero here this is check mate
    }
    // no check
    else
    {
        generate_pin_mask();
        //generate_discovered_mask();
        generate_castle_moves(depth);
        generate_capture_pawn_moves(depth);
        generate_knight_moves(depth);
        generate_bishop_moves(depth);
        generate_rook_moves(depth);
        generate_queen_moves(depth);
        generate_non_capture_pawn_moves(depth);
        generate_king_moves(depth);

        // if move count is zero here this is stalemate
    }
}

//------------------------------------------------------------------------------
// do_move : plays 'move', updates bitboards/mailbox/castle/EP, toggles hash.
//           Returns a full UndoPacked we feed to undo_move.
//------------------------------------------------------------------------------
static inline __attribute__((always_inline))
UndoPacked do_move(Move move)
{
    // Snapshot EP+castle flags and full 64‐bit hash
    UndoPacked undo_info;
    undo_info.flags = Encoder::pack_flags(en_passant, castle_right);
    undo_info.hash  = position_hash;

    // Remove old en_passant from hash
    if (en_passant != no_sqr) {
        int old_file = en_passant & 7;
        position_hash ^= Tables::zobrist_aux[ZOB_EP_FILE_A + old_file];
    }
    en_passant = no_sqr;

    // Remove old castling bits from hash
    if (castle_right & Castle_Right::KC) position_hash ^= Tables::zobrist_aux[ZOB_CASTLE_WK];
    if (castle_right & Castle_Right::QC) position_hash ^= Tables::zobrist_aux[ZOB_CASTLE_WQ];
    if (castle_right & Castle_Right::kc) position_hash ^= Tables::zobrist_aux[ZOB_CASTLE_BK];
    if (castle_right & Castle_Right::qc) position_hash ^= Tables::zobrist_aux[ZOB_CASTLE_BQ];

    // Decode move
    int         from_sq         = Encoder::move_get_from(move);
    int         to_sq           = Encoder::move_get_to(move);
    Piece_Type  moved_piece     = Encoder::move_get_moved_piece(move);
    Piece_Type  captured        = Encoder::move_get_captured_piece(move);
    Piece_Type  promotion       = Encoder::move_get_promo_piece(move);
    bool        double_push     = Encoder::move_is_double_pawn(move);
    bool        ep_capture      = Encoder::move_is_en_passant(move);
    bool        castle_K        = Encoder::move_is_castle_kingside(move);
    bool        castle_Q        = Encoder::move_is_castle_queenside(move);
    bool        opp             = !turn;
    int         push_offset     = (turn == white) ? -8 : +8;

    // Your pawn logic, with Zobrist hooks:
    if (moved_piece == Pawn) {
        if (captured != Empty) {
            // --- en passant capture ---
            if (ep_capture) {
                // flip our pawn off from_sq
                FLIP_BIT(piece_occ_bb[turn][Pawn], from_sq);
                position_hash ^= Tables::zobrist_piece[(turn==white?Pawn:Pawn+6)][from_sq];

                // place pawn on to_sq
                FLIP_BIT(piece_occ_bb[turn][Pawn], to_sq);
                position_hash ^= Tables::zobrist_piece[(turn==white?Pawn:Pawn+6)][to_sq];

                // remove the captured pawn "behind" to_sq
                int ep_sq = to_sq + (-push_offset);
                FLIP_BIT(piece_occ_bb[opp][Pawn], ep_sq);
                position_hash ^= Tables::zobrist_piece[(opp==white?Pawn:Pawn+6)][ep_sq];

                // mailbox
                mailbox[turn][from_sq] = Empty;
                mailbox[turn][to_sq]   = Pawn;
                mailbox[opp][ep_sq]    = Empty;
            }
            // --- promotion capture ---
            else if (promotion != Empty) {
                // remove pawn from from_sq
                FLIP_BIT(piece_occ_bb[turn][Pawn], from_sq);
                position_hash ^= Tables::zobrist_piece[(turn==white?Pawn:Pawn+6)][from_sq];

                // place promoted piece on to_sq
                FLIP_BIT(piece_occ_bb[turn][promotion], to_sq);
                position_hash ^= Tables::zobrist_piece[(turn==white?promotion:promotion+6)][to_sq];

                // remove captured piece there
                FLIP_BIT(piece_occ_bb[opp][captured], to_sq);
                position_hash ^= Tables::zobrist_piece[(opp==white?captured:captured+6)][to_sq];

                mailbox[turn][from_sq] = Empty;
                mailbox[turn][to_sq]   = promotion;
                mailbox[opp][to_sq]    = Empty;
            }
            // --- normal capture ---
            else {
                // pawn off from_sq
                FLIP_BIT(piece_occ_bb[turn][Pawn], from_sq);
                position_hash ^= Tables::zobrist_piece[(turn==white?Pawn:Pawn+6)][from_sq];

                // pawn onto to_sq
                FLIP_BIT(piece_occ_bb[turn][Pawn], to_sq);
                position_hash ^= Tables::zobrist_piece[(turn==white?Pawn:Pawn+6)][to_sq];

                // remove victim
                FLIP_BIT(piece_occ_bb[opp][captured], to_sq);
                position_hash ^= Tables::zobrist_piece[(opp==white?captured:captured+6)][to_sq];

                mailbox[turn][from_sq] = Empty;
                mailbox[turn][to_sq]   = Pawn;
                mailbox[opp][to_sq]    = Empty;
            }
        }
        // --- quiet pawn moves ---
        else {
            // double push → set new EP
            if (double_push) {
                // pawn off from_sq
                FLIP_BIT(piece_occ_bb[turn][Pawn], from_sq);
                position_hash ^= Tables::zobrist_piece[(turn==white?Pawn:Pawn+6)][from_sq];

                // pawn onto to_sq
                FLIP_BIT(piece_occ_bb[turn][Pawn], to_sq);
                position_hash ^= Tables::zobrist_piece[(turn==white?Pawn:Pawn+6)][to_sq];

                mailbox[turn][from_sq] = Empty;
                mailbox[turn][to_sq]   = Pawn;

                en_passant = to_sq + (-push_offset);
                position_hash ^= Tables::zobrist_aux[ZOB_EP_FILE_A + (en_passant & 7)];
            }
            // single‐push promotion
            else if (promotion != Empty) {
                FLIP_BIT(piece_occ_bb[turn][Pawn], from_sq);
                position_hash ^= Tables::zobrist_piece[(turn==white?Pawn:Pawn+6)][from_sq];

                FLIP_BIT(piece_occ_bb[turn][promotion], to_sq);
                position_hash ^= Tables::zobrist_piece[(turn==white?promotion:promotion+6)][to_sq];

                mailbox[turn][from_sq] = Empty;
                mailbox[turn][to_sq]   = promotion;
            }
            // ordinary pawn move
            else {
                FLIP_BIT(piece_occ_bb[turn][Pawn], from_sq);
                position_hash ^= Tables::zobrist_piece[(turn==white?Pawn:Pawn+6)][from_sq];

                FLIP_BIT(piece_occ_bb[turn][Pawn], to_sq);
                position_hash ^= Tables::zobrist_piece[(turn==white?Pawn:Pawn+6)][to_sq];

                mailbox[turn][from_sq] = Empty;
                mailbox[turn][to_sq]   = Pawn;
            }
        }
    }
    // King & castling (with hashing)
    else if (moved_piece == King) {
        king_position[turn] = to_sq;

        if (captured != Empty) {
            FLIP_BIT(piece_occ_bb[turn][King], from_sq);
            position_hash ^= Tables::zobrist_piece[(turn==white?King:King+6)][from_sq];

            FLIP_BIT(piece_occ_bb[turn][King], to_sq);
            position_hash ^= Tables::zobrist_piece[(turn==white?King:King+6)][to_sq];

            FLIP_BIT(piece_occ_bb[opp][captured], to_sq);
            position_hash ^= Tables::zobrist_piece[(opp==white?captured:captured+6)][to_sq];

            mailbox[turn][from_sq] = Empty;
            mailbox[turn][to_sq]   = King;
            mailbox[opp][to_sq]    = Empty;
        }
        else if (castle_K) {
            // king
            FLIP_BIT(piece_occ_bb[turn][King], from_sq);
            position_hash ^= Tables::zobrist_piece[(turn==white?King:King+6)][from_sq];

            FLIP_BIT(piece_occ_bb[turn][King], to_sq);
            position_hash ^= Tables::zobrist_piece[(turn==white?King:King+6)][to_sq];

            mailbox[turn][from_sq] = Empty;
            mailbox[turn][to_sq]   = King;

            // rook hop
            int rf = (turn==white? h1 : h8);
            int rt = (turn==white? f1 : f8);
            FLIP_BIT(piece_occ_bb[turn][Rook], rf);
            position_hash ^= Tables::zobrist_piece[(turn==white?Rook:Rook+6)][rf];

            FLIP_BIT(piece_occ_bb[turn][Rook], rt);
            position_hash ^= Tables::zobrist_piece[(turn==white?Rook:Rook+6)][rt];

            mailbox[turn][rf] = Empty;
            mailbox[turn][rt] = Rook;
        }
        else if (castle_Q) {
            FLIP_BIT(piece_occ_bb[turn][King], from_sq);
            position_hash ^= Tables::zobrist_piece[(turn==white?King:King+6)][from_sq];

            FLIP_BIT(piece_occ_bb[turn][King], to_sq);
            position_hash ^= Tables::zobrist_piece[(turn==white?King:King+6)][to_sq];

            mailbox[turn][from_sq] = Empty;
            mailbox[turn][to_sq]   = King;

            int rf = (turn==white? a1 : a8);
            int rt = (turn==white? d1 : d8);
            FLIP_BIT(piece_occ_bb[turn][Rook], rf);
            position_hash ^= Tables::zobrist_piece[(turn==white?Rook:Rook+6)][rf];

            FLIP_BIT(piece_occ_bb[turn][Rook], rt);
            position_hash ^= Tables::zobrist_piece[(turn==white?Rook:Rook+6)][rt];

            mailbox[turn][rf] = Empty;
            mailbox[turn][rt] = Rook;
        }
        else {
            FLIP_BIT(piece_occ_bb[turn][King], from_sq);
            position_hash ^= Tables::zobrist_piece[(turn==white?King:King+6)][from_sq];

            FLIP_BIT(piece_occ_bb[turn][King], to_sq);
            position_hash ^= Tables::zobrist_piece[(turn==white?King:King+6)][to_sq];

            mailbox[turn][from_sq] = Empty;
            mailbox[turn][to_sq]   = King;
        }
    }
    // Knight/Bishop/Rook/Queen (with hashing)
    else {
        if (captured != Empty) {
            FLIP_BIT(piece_occ_bb[turn][moved_piece], from_sq);
            position_hash ^= Tables::zobrist_piece[(turn==white?moved_piece:moved_piece+6)][from_sq];

            FLIP_BIT(piece_occ_bb[turn][moved_piece], to_sq);
            position_hash ^= Tables::zobrist_piece[(turn==white?moved_piece:moved_piece+6)][to_sq];

            FLIP_BIT(piece_occ_bb[opp][captured], to_sq);
            position_hash ^= Tables::zobrist_piece[(opp==white?captured:captured+6)][to_sq];

            mailbox[turn][from_sq] = Empty;
            mailbox[turn][to_sq]   = moved_piece;
            mailbox[opp][to_sq]    = Empty;
        } else {
            FLIP_BIT(piece_occ_bb[turn][moved_piece], from_sq);
            position_hash ^= Tables::zobrist_piece[(turn==white?moved_piece:moved_piece+6)][from_sq];

            FLIP_BIT(piece_occ_bb[turn][moved_piece], to_sq);
            position_hash ^= Tables::zobrist_piece[(turn==white?moved_piece:moved_piece+6)][to_sq];

            mailbox[turn][from_sq] = Empty;
            mailbox[turn][to_sq]   = moved_piece;
        }
    }

    // rebuild occupancy
    std::memset(player_occ_bb, 0, 24);
    for (int pt = Pawn; pt <= King; ++pt) {
      player_occ_bb[white] |= piece_occ_bb[white][pt];
      player_occ_bb[black] |= piece_occ_bb[black][pt];
    }
    player_occ_bb[all_color] = player_occ_bb[white] | player_occ_bb[black];

    // update castling rights (no hash here—will be cleared next move)
    castle_right &= Tables::castling_table[from_sq];
    castle_right &= Tables::castling_table[to_sq];

    if (castle_right & Castle_Right::KC) position_hash ^= Tables::zobrist_aux[ZOB_CASTLE_WK];
    if (castle_right & Castle_Right::QC) position_hash ^= Tables::zobrist_aux[ZOB_CASTLE_WQ];
    if (castle_right & Castle_Right::kc) position_hash ^= Tables::zobrist_aux[ZOB_CASTLE_BK];
    if (castle_right & Castle_Right::qc) position_hash ^= Tables::zobrist_aux[ZOB_CASTLE_BQ];

    // toggle side‐to‐move in hash
    turn = opp;
    position_hash ^= Tables::zobrist_aux[ZOB_SIDE_TO_MOVE];

    return undo_info;
}

//------------------------------------------------------------------------------
// undo_move : exact inverse of do_move above. We pass in the UndoPacked it
//             returned, and it restores both our Zobrist key + EP/castle flags,
//             flips turn back, then re-runs our bitboard/mailbox undoes.
//------------------------------------------------------------------------------
static inline __attribute__((always_inline))
void undo_move(Move move, UndoPacked undo)
{
    // restore the whole hash + EP/castle flags
    position_hash  = undo.hash;
    {
      uint16_t f = undo.flags;
      en_passant   = Encoder::unpack_ep(f);
      castle_right = Encoder::unpack_cr(f);
    }

    // flip side-to-move back
    turn = !turn;
    bool opp = !turn;
    int  push_offset = (turn == white) ? -8 : +8;

    // decode exactly as in do_move
    int         from_sq = Encoder::move_get_from(move);
    int         to_sq   = Encoder::move_get_to(move);
    Piece_Type  mp      = Encoder::move_get_moved_piece(move);
    Piece_Type  cap     = Encoder::move_get_captured_piece(move);
    Piece_Type  promo   = Encoder::move_get_promo_piece(move);
    bool        dbl_push= Encoder::move_is_double_pawn(move);
    bool        ep_cap  = Encoder::move_is_en_passant(move);
    bool        kc      = Encoder::move_is_castle_kingside(move);
    bool        qc      = Encoder::move_is_castle_queenside(move);

    // exact reverse bitboard/mailbox logic:
    if (mp == Pawn) {
      if (cap != Empty) {
        if (ep_cap) {
          FLIP_BIT(piece_occ_bb[turn][Pawn], from_sq);
          FLIP_BIT(piece_occ_bb[turn][Pawn], to_sq);
          int ep_sq = to_sq + (-push_offset);
          FLIP_BIT(piece_occ_bb[opp][Pawn], ep_sq);
          mailbox[turn][from_sq] = Pawn;
          mailbox[turn][to_sq]   = Empty;
          mailbox[opp][ep_sq]    = Pawn;
        }
        else if (promo != Empty) {
          FLIP_BIT(piece_occ_bb[turn][Pawn], from_sq);
          FLIP_BIT(piece_occ_bb[turn][promo], to_sq);
          FLIP_BIT(piece_occ_bb[opp][cap],    to_sq);
          mailbox[turn][from_sq] = Pawn;
          mailbox[turn][to_sq]   = Empty;
          mailbox[opp][to_sq]    = cap;
        }
        else {
          FLIP_BIT(piece_occ_bb[turn][Pawn], from_sq);
          FLIP_BIT(piece_occ_bb[turn][Pawn], to_sq);
          FLIP_BIT(piece_occ_bb[opp][cap],    to_sq);
          mailbox[turn][from_sq] = Pawn;
          mailbox[turn][to_sq]   = Empty;
          mailbox[opp][to_sq]    = cap;
        }
      } else {
        if (dbl_push) {
          FLIP_BIT(piece_occ_bb[turn][Pawn], from_sq);
          FLIP_BIT(piece_occ_bb[turn][Pawn], to_sq);
          mailbox[turn][from_sq] = Pawn;
          mailbox[turn][to_sq]   = Empty;
        }
        else if (promo != Empty) {
          FLIP_BIT(piece_occ_bb[turn][Pawn],  from_sq);
          FLIP_BIT(piece_occ_bb[turn][promo], to_sq);
          mailbox[turn][from_sq] = Pawn;
          mailbox[turn][to_sq]   = Empty;
        }
        else {
          FLIP_BIT(piece_occ_bb[turn][Pawn], from_sq);
          FLIP_BIT(piece_occ_bb[turn][Pawn], to_sq);
          mailbox[turn][from_sq] = Pawn;
          mailbox[turn][to_sq]   = Empty;
        }
      }
    }
    else if (mp == King) {
      king_position[turn] = from_sq;
      if (cap != Empty) {
        FLIP_BIT(piece_occ_bb[turn][King], from_sq);
        FLIP_BIT(piece_occ_bb[turn][King], to_sq);
        FLIP_BIT(piece_occ_bb[opp][cap],   to_sq);
        mailbox[turn][from_sq] = King;
        mailbox[turn][to_sq]   = Empty;
        mailbox[opp][to_sq]    = cap;
      }
      else if (kc) {
        FLIP_BIT(piece_occ_bb[turn][King], from_sq);
        FLIP_BIT(piece_occ_bb[turn][King], to_sq);
        mailbox[turn][from_sq] = King;
        mailbox[turn][to_sq]   = Empty;
        int rf = (turn==white? h1:h8), rt = (turn==white? f1:f8);
        FLIP_BIT(piece_occ_bb[turn][Rook], rf);
        FLIP_BIT(piece_occ_bb[turn][Rook], rt);
        mailbox[turn][rf] = Rook;
        mailbox[turn][rt] = Empty;
      }
      else if (qc) {
        FLIP_BIT(piece_occ_bb[turn][King], from_sq);
        FLIP_BIT(piece_occ_bb[turn][King], to_sq);
        mailbox[turn][from_sq] = King;
        mailbox[turn][to_sq]   = Empty;
        int rf = (turn==white? a1:a8), rt = (turn==white? d1:d8);
        FLIP_BIT(piece_occ_bb[turn][Rook], rf);
        FLIP_BIT(piece_occ_bb[turn][Rook], rt);
        mailbox[turn][rf] = Rook;
        mailbox[turn][rt] = Empty;
      }
      else {
        FLIP_BIT(piece_occ_bb[turn][King], from_sq);
        FLIP_BIT(piece_occ_bb[turn][King], to_sq);
        mailbox[turn][from_sq] = King;
        mailbox[turn][to_sq]   = Empty;
      }
    }
    else {
      if (cap != Empty) {
        FLIP_BIT(piece_occ_bb[turn][mp],  from_sq);
        FLIP_BIT(piece_occ_bb[turn][mp],  to_sq);
        FLIP_BIT(piece_occ_bb[opp][cap],  to_sq);
        mailbox[turn][from_sq] = mp;
        mailbox[turn][to_sq]   = Empty;
        mailbox[opp][to_sq]    = cap;
      } else {
        FLIP_BIT(piece_occ_bb[turn][mp], from_sq);
        FLIP_BIT(piece_occ_bb[turn][mp], to_sq);
        mailbox[turn][from_sq] = mp;
        mailbox[turn][to_sq]   = Empty;
      }
    }

    // rebuild occupancy
    std::memset(player_occ_bb, 0, 24);
    for (int pt = Pawn; pt <= King; ++pt) {
      player_occ_bb[white] |= piece_occ_bb[white][pt];
      player_occ_bb[black] |= piece_occ_bb[black][pt];
    }
    player_occ_bb[all_color] = player_occ_bb[white] | player_occ_bb[black];
}




//-----------------------------------------------------------------------------
// A little helper to hold exactly the fields we clobber when we null‐move.
//-----------------------------------------------------------------------------
struct NullUndo {
  int           old_ep;
  int           old_castle;
  uint64_t      old_hash;
  bool          old_turn;
};

//-----------------------------------------------------------------------------
// do_null_move()
//    — applies a “pass” (flip side, clear EP, update Zobrist), and
//      returns a NullUndo you must pass to undo_null_move()
//-----------------------------------------------------------------------------
static inline __attribute__((always_inline))
NullUndo do_null_move()
{
    // snapshot
    NullUndo nu {
        .old_ep      = en_passant,
        .old_castle  = castle_right,
        .old_hash    = position_hash,
        .old_turn    = turn
    };

    // remove old EP from hash if any
    if (nu.old_ep != no_sqr)
    {
        int f = nu.old_ep & 7;  // extract file 0=a … 7=h
        Move_Gen::position_hash ^= Tables::zobrist_aux[ZOB_EP_FILE_A + f];
    }
    Move_Gen::en_passant = no_sqr;

    // flip side‐to‐move in both state + hash
    Move_Gen::turn = !nu.old_turn;
    Move_Gen::position_hash ^= Tables::zobrist_aux[ZOB_SIDE_TO_MOVE];

    return nu;
}

//-----------------------------------------------------------------------------
// undo_null_move()
//    — restores everything do_null_move clobbered
//-----------------------------------------------------------------------------
static inline __attribute__((always_inline))
void undo_null_move(const NullUndo &nu)
{
    turn         = nu.old_turn;
    en_passant   = nu.old_ep;
    castle_right = nu.old_castle;
    position_hash= nu.old_hash;
}




}   // end Move_Gen namepsace
}   // end RedStone namespace