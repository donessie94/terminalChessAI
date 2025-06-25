#pragma once
#include"../utils/misc.h"

namespace RedStone{

// Helpers Macros ===================================================================================================================================================

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

namespace Move_Gen{

// Globals====================================================================================================================================================
extern int king_position[2];
extern int castle_right;
extern int en_passant;
extern bool turn;

// big enough so all possible move in any position fit here
extern Move valid_moves[218];
// tracks how many moves were generated on this turn
extern int move_count;

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

// clear past pin_mask and pin_ray_mask and generate the new ones for the current position
void inline generate_pin_mask()
{
    /* reset pin_mask first */
    pin_mask = 0ULL;
    std::memset(pin_ray_mask, 0, sizeof(pin_ray_mask));

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
    Bitboard potential_orth_attackers = Tables::diagonal_ray_mask[king_sq] & opp_queen_rook_occ;

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

void inline generate_discovered_mask()
{
    /* reset discovered masks first */
    discovered_mask = 0ULL;
    std::memset(discovered_ray_mask, 0, sizeof(discovered_ray_mask));

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

void inline generate_check_mask()
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
    attackers |= ( Tables::pawn_attack_bb[opp][ally_king_sq] & opp_pawn_occ );
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

void inline generate_non_capture_pawn_moves()
{
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
        Bitboard ally_pwn_occ_bb = 1ULL << ally_pwn_sq;

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
                    valid_moves[move_count] = Encoder::construct_move(ally_pwn_sq, to_sq, Pawn, Empty, Empty, move_flags);
                    move_count++;
                }
                else
                    continue;   //  if the move was not in the right direction to keep blocking the check we skip to next pawn (this pawn push leaves king in check)
            }
            // if square was not on pinned pieces then save the move
            else
            {
                // construct the move and save it
                valid_moves[move_count] = Encoder::construct_move(ally_pwn_sq, to_sq, Pawn, Empty, Empty, move_flags);
                move_count++;
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
                    valid_moves[move_count] = Encoder::construct_move(ally_pwn_sq, to_sq2, Pawn, Empty, Empty, move_flags);
                    move_count++;
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

        Bitboard ally_pwn_occ_bb = 1ULL << ally_pwn_sq;

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
                    valid_moves[move_count] = Encoder::construct_move(ally_pwn_sq, to_sq, Pawn, Empty, Queen, 0u);
                    move_count++;
                    valid_moves[move_count] = Encoder::construct_move(ally_pwn_sq, to_sq, Pawn, Empty, Rook, 0u);
                    move_count++;
                    valid_moves[move_count] = Encoder::construct_move(ally_pwn_sq, to_sq, Pawn, Empty, Bishop, 0u);
                    move_count++;
                    valid_moves[move_count] = Encoder::construct_move(ally_pwn_sq, to_sq, Pawn, Empty, Knight, 0u);
                    move_count++;
                }
                else
                    continue;   //  if the move was not in the right direction to keep blocking the check we skip to next pawn (this pawn push leaves king in check)
            }
            // if square was not on pinned pieces then save the move
            else
            {
                // construct the move and save it
                valid_moves[move_count] = Encoder::construct_move(ally_pwn_sq, to_sq, Pawn, Empty, Queen, 0u);
                move_count++;
                valid_moves[move_count] = Encoder::construct_move(ally_pwn_sq, to_sq, Pawn, Empty, Rook, 0u);
                move_count++;
                valid_moves[move_count] = Encoder::construct_move(ally_pwn_sq, to_sq, Pawn, Empty, Bishop, 0u);
                move_count++;
                valid_moves[move_count] = Encoder::construct_move(ally_pwn_sq, to_sq, Pawn, Empty, Knight, 0u);
                move_count++;
            }
        }
    }
}

void inline generate_capture_pawn_moves()
{
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
                    valid_moves[move_count] = Encoder::construct_move(ally_pwn_sq, move_idx, Pawn, captured, Empty, 0u);
                    move_count++;
                }
            }
            // piece is not pinned so move is valid
            else
            {
                // lets figure out what piece this move is capturing
                Piece_Type captured = mailbox[!turn][move_idx];
                valid_moves[move_count] = Encoder::construct_move(ally_pwn_sq, move_idx, Pawn, captured, Empty, 0u);
                move_count++;
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

            // we know the move captures an enemy piece so far (pseudo valid)
            // so we have to make sure it does not leave our king in check
            if(pin_mask & ally_pwn_occ_bb)
            {
                // if piece is pinned but the move still blocks (along the same pinned ray) then move is valid
                if(pin_ray_mask[ally_pwn_sq] & move_bb)
                {
                    // lets figure out what piece this move is capturing (using our mailbox representation)
                    Piece_Type captured = mailbox[!turn][move_idx];

                    valid_moves[move_count] = Encoder::construct_move(ally_pwn_sq, move_idx, Pawn, captured, Queen, 0u);
                    move_count++;
                    valid_moves[move_count] = Encoder::construct_move(ally_pwn_sq, move_idx, Pawn, captured, Rook, 0u);
                    move_count++;
                    valid_moves[move_count] = Encoder::construct_move(ally_pwn_sq, move_idx, Pawn, captured, Bishop, 0u);
                    move_count++;
                    valid_moves[move_count] = Encoder::construct_move(ally_pwn_sq, move_idx, Pawn, captured, Knight, 0u);
                    move_count++;
                }
            }
            // piece is not pinned so move is valid
            else
            {
                // lets figure out what piece this move is capturing
                Piece_Type captured = mailbox[!turn][move_idx];
                valid_moves[move_count] = Encoder::construct_move(ally_pwn_sq, move_idx, Pawn, captured, Queen, 0u);
                move_count++;
                valid_moves[move_count] = Encoder::construct_move(ally_pwn_sq, move_idx, Pawn, captured, Rook, 0u);
                move_count++;
                valid_moves[move_count] = Encoder::construct_move(ally_pwn_sq, move_idx, Pawn, captured, Bishop, 0u);
                move_count++;
                valid_moves[move_count] = Encoder::construct_move(ally_pwn_sq, move_idx, Pawn, captured, Knight, 0u);
                move_count++;
            }
        }
    }
}

void inline generate_king_moves()
{
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
        // if king ends in check move is not valid
        if(IS_SQUARE_ATTACKED(!turn, occ_after ,move_idx))
            continue;

        // if enemy king is too close move is not valid
        if(GET_KING_ATTACK(move_idx) & opp_king_occ)
            continue;

        // lets figure out what piece this move is capturing
        Piece_Type captured = mailbox[!turn][move_idx];
        valid_moves[move_count] = Encoder::construct_move(king_sq, move_idx, King, captured, Empty, 0u);
        move_count++;
    }

    // non capture moves
    while (non_capture_moves)
    {
        int move_idx = LS1B_IDX(non_capture_moves);
        non_capture_moves &= non_capture_moves - 1;

        Bitboard occ_after = (all_occ ^ (1ULL<<king_sq)) | (1ULL<<move_idx);
        // if king ends in check move is not valid
        if(IS_SQUARE_ATTACKED(!turn, occ_after ,move_idx))
            continue;

        // if enemy king is too close move is not valid
        if(GET_KING_ATTACK(move_idx) & opp_king_occ)
            continue;

        valid_moves[move_count] = Encoder::construct_move(king_sq, move_idx, King, Empty, Empty, 0u);
        move_count++;
    }
}

void inline generate_castle_moves()
{

}

void inline generate_knight_moves()
{
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
            if(pin_mask & this_knight_occ)
                continue;
            // piece is not pinned so move is valid
            else
            {
                // lets figure out what piece this move is capturing
                Piece_Type captured = mailbox[!turn][move_idx];
                valid_moves[move_count] = Encoder::construct_move(this_knight_sq, move_idx, Knight, captured, Empty, 0u);
                move_count++;
            }
        }

        // non capture moves
        while (non_capture_moves)
        {
            int move_idx = LS1B_IDX(non_capture_moves);
            non_capture_moves &= non_capture_moves - 1;
            if(pin_mask & this_knight_occ)
                continue;
            // piece is not pinned so move is valid
            else
            {
                valid_moves[move_count] = Encoder::construct_move(this_knight_sq, move_idx, Knight, Empty, Empty, 0u);
                move_count++;
            }
        }
    }
}

void inline generate_bishop_moves()
{
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
            if(pin_mask & this_bishop_occ)
            {
                // if piece is pinned but the move still blocks (along the same pinned ray) then move is valid
                if(pin_ray_mask[this_bishop_sq] & move_bb)
                {
                    // lets figure out what piece this move is capturing (using our mailbox representation)
                    Piece_Type captured = mailbox[!turn][move_idx];
                    valid_moves[move_count] = Encoder::construct_move(this_bishop_sq, move_idx, Bishop, captured, Empty, 0u);
                    move_count++;
                }
            }
            // piece is not pinned so move is valid
            else
            {
                // lets figure out what piece this move is capturing
                Piece_Type captured = mailbox[!turn][move_idx];
                valid_moves[move_count] = Encoder::construct_move(this_bishop_sq, move_idx, Bishop, captured, Empty, 0u);
                move_count++;
            }
        }
        // non capture moves
        while (non_capture_moves)
        {
            int move_idx = LS1B_IDX(non_capture_moves);
            non_capture_moves &= non_capture_moves - 1;
            Bitboard move_bb = 1ULL << move_idx;
            if(pin_mask & this_bishop_occ)
            {
                if(pin_ray_mask[this_bishop_sq] & move_bb)
                {
                    valid_moves[move_count] = Encoder::construct_move(this_bishop_sq, move_idx, Bishop, Empty, Empty, 0u);
                    move_count++;
                }
            }
            else
            {
                valid_moves[move_count] = Encoder::construct_move(this_bishop_sq, move_idx, Bishop, Empty, Empty, 0u);
                move_count++;
            }
        }
    }
}

void inline generate_rook_moves()
{
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

            if (pin_mask & this_rook_occ)
            {
                if (pin_ray_mask[this_rook_sq] & move_bb)
                {
                    Piece_Type captured = mailbox[!turn][move_idx];
                    valid_moves[move_count] = Encoder::construct_move(this_rook_sq, move_idx, Rook, captured, Empty, 0u);
                    move_count++;
                }
            }
            else
            {
                Piece_Type captured = mailbox[!turn][move_idx];
                valid_moves[move_count] = Encoder::construct_move(this_rook_sq, move_idx, Rook, captured, Empty, 0u);
                move_count++;
            }
        }

        while (non_capture_moves)
        {
            int move_idx = LS1B_IDX(non_capture_moves);
            non_capture_moves &= non_capture_moves - 1;
            Bitboard move_bb = 1ULL << move_idx;

            if (pin_mask & this_rook_occ)
            {
                if (pin_ray_mask[this_rook_sq] & move_bb)
                {
                    valid_moves[move_count] = Encoder::construct_move(this_rook_sq, move_idx, Rook, Empty, Empty, 0u);
                    move_count++;
                }
            }
            else
            {
                valid_moves[move_count] = Encoder::construct_move(this_rook_sq, move_idx, Rook, Empty, Empty, 0u);
                move_count++;
            }
        }
    }
}

void inline generate_queen_moves()
{
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

            if (pin_mask & this_queen_occ)
            {
                if (pin_ray_mask[this_queen_sq] & move_bb)
                {
                    Piece_Type captured = mailbox[!turn][move_idx];
                    valid_moves[move_count] = Encoder::construct_move(this_queen_sq, move_idx, Queen, captured, Empty, 0u);
                    move_count++;
                }
            }
            else
            {
                Piece_Type captured = mailbox[!turn][move_idx];
                valid_moves[move_count] = Encoder::construct_move(this_queen_sq, move_idx, Queen, captured, Empty, 0u);
                move_count++;
            }
        }

        while (non_capture_moves)
        {
            int move_idx = LS1B_IDX(non_capture_moves);
            non_capture_moves &= non_capture_moves - 1;
            Bitboard move_bb = 1ULL << move_idx;

            if (pin_mask & this_queen_occ)
            {
                if (pin_ray_mask[this_queen_sq] & move_bb)
                {
                    valid_moves[move_count] = Encoder::construct_move(this_queen_sq, move_idx, Queen, Empty, Empty, 0u);
                    move_count++;
                }
            }
            else
            {
                valid_moves[move_count] = Encoder::construct_move(this_queen_sq, move_idx, Queen, Empty, Empty, 0u);
                move_count++;
            }
        }
    }
}

void inline generate_moves()
{
    // zero the move count
    move_count = 0;

    //
    generate_check_mask();

    // unblckable check
    if(num_attackers > 1)
    {

    }
    // blockable check
    else if(num_attackers == 1)
    {
        generate_pin_mask();
        generate_king_moves();
    }
    // no check
    else
    {
        generate_pin_mask();
        generate_non_capture_pawn_moves();
        generate_capture_pawn_moves();
        generate_castle_moves();
        generate_king_moves();
        generate_knight_moves();
        generate_bishop_moves();
        generate_rook_moves();
        generate_queen_moves();
    }
}


}   // end Move_Gen namepsace
}   // end RedStone namespace