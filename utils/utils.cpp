#include "utils.h"

//
bitboard pawn_attack_bb[2][64];  // e.g. side 0=white, 1=black
bitboard knight_attack_bb[64];
bitboard king_attack_bb[64];
bitboard bishop_attack_bb[64];

//
bitboard bishop_relevant_sqrs_bb[64];
bitboard rook_relevant_sqrs_bb[64];
bitboard queen_relevant_sqrs_bb[64];

// ALL COMBINATIONS OF SIZE N ======================================================================================

// example:
// 2^3 ==> 1000 => 8 it directly maps all subsets of a set of 3 elements
// RECALL we start counting from 0
// so the correct bit representation will happend for N=3 in 2^n-1 = 7
// so the bits look like this binary 111 => decimal 7 (but from 0 so 8 different unique combinations)
// the different combinations of 0s and 1s exactly matches how many ways you can say yes or no 3 times in a row
//
// so lets construct all subsets of a set of size N = 3
// list all elements (3) in a row, then say yes or not to include from left to right
//
// obviously there is a one to one correspondence between number of subsets 2^n and
// binary representation of 2^n (since all representations are unique) (example n=3)
// mapping fucntion (one to one)
// when first  bit is set ----> represents that element 1 in the set is included
// when second bit is set ----> represents that element 2 in the set is included
// when third  bit is set ----> represents that element 3 in the set is included
// Thus we can use this fact to find all possible combinations

// CODE EXAMPLE
//for each possible binary combination
//  for(int bin_representation=0; bin_representation<8; bin_representation++)
//  {
//     // holds the representation (different unquie subset)
//     bitboard attack = 0ULL;

//     // check if the flag idx of the set element is set or not
//     for (int set_element_idx = 0; set_eleemnt_idx<set.size(); set_element_idx++)
//     {
//         // the bit to check is basically (1 << set_element_idx)
//         // if this is true we include that element, else we dont
//         // to check if bit is set we AND it with the bin_representation
//         if(bin_representation & (1 << set_element_idx))
//         {
//             // if bit is set include the element "set_element_idx" of the original set
//             // else keep this as zero (dont include the element)
//             attack |= (1 << SET[set_element_idx])
//         }
//     }

//     // after each iteration we have an unique "attack" the representation of the different/unique subset
//  }

// ==============================================================================================================

// uint64_t relevant_mask = /* your ULL with 5 bits set, e.g. bits for c2,d3,e4,f5,g6 */;
// std::vector<int> relevant_indices;
// {
//     uint64_t bb = relevant_mask;
//     while (bb) {
//         // isolate least significant bit
//         int sq = __builtin_ctzll(bb);    // index 0..63 of LSB
//         relevant_indices.push_back(sq);
//         bb &= bb - 1;                     // clear that bit
//     }
// }
// // Now relevant_indices.size() == 5, containing the 5 square indices in some order.
// int N = (int)relevant_indices.size();  // should be 5

//int total = 1 << N;  // 2^5 = 32
// for (int mask = 0; mask < total; mask++) {
//     // Build the occupancy subset bitboard for this combination:
//     uint64_t occ_subset = 0ULL;
//     for (int i = 0; i < N; i++) {
//         if (mask & (1 << i)) {
//             int sq = relevant_indices[i];
//             occ_subset |= (1ULL << sq);
//         }
//     }
//     // Now occ_subset has bits set exactly on those relevant squares chosen by mask.
//     // Next: compute the attack mask for this occ_subset by ray-walk, etc.
//     uint64_t attack_bb = compute_sliding_attacks(origin_sq, occ_subset);
//     // Store attack_bb in your table, e.g. table[mask] = attack_bb (or after magic compression).
// }

//
void compute_leapers_attacks_bb()
{
    bitboard bb = 0ULL;
    bitboard attacks_w = 0ULL;
    bitboard attacks_b = 0ULL;
    for(int rank=0; rank<8; rank++)
    {
        for(int file=0; file<8; file++)
        {
            // << 3 is * 2^3 = 8
            int idx = (rank << 3) + file;

            // set bit to current square on the board (idx)
            set_bit(bb, idx);

            // ====================== PAWN GENERATION =========================================================================
            // only way to shift 7 bits to the right and end up in the 'a' file is to start at 'h' file
            // thus if this is the case the expression will return 0 (false)
            // is this piece (pawn) starting in the column 'h'? (based on pawn movements shift 7) => if so this returns false
            // shift 7 right gives us the / attack for pawn (diagonal step to the right - up)
            if ((bb >> 7) & not_a_file)
                attacks_w |= (bb >> 7);   // if pawn did not started on column h then we can mark the attack bitboard in this (bitboard >> 7) position using OR

            // only way to shift 9 bits to the right and end up in 'h' file is by starting at 'a' file
            // this is '\' step (diagonal up-left)
            // NOTE RECALL BOARD IS INVERTED SO RIGHT IS COUNTING TO THE LEFT IN THE BOARD ABOVE <- this way from position
            if ((bb >> 9) & not_h_file)
                attacks_w |= (bb >> 9);

            pawn_attack_bb[Color::white][idx] = attacks_w;

            // for black basically the same inverted
            if ((bb << 7) & not_h_file)
                attacks_b |= (bb << 7);
            if ((bb << 9) & not_a_file)
                attacks_b |= (bb << 9);

            pawn_attack_bb[Color::black][idx] = attacks_b;

            // clear bit for next iteration
            bb = 0ULL;
            attacks_w = 0ULL;
            attacks_b = 0ULL;

            // =================================================================================================================

            // ====================== KNIGHT GENERATION ========================================================================
            // For each shift, mask first to avoid wrap‐around, then shift:
            // Shifts for knight moves:
            //  - 17: up 2, right 1  (<< 17 from lower square; but for continuous generation we consider both directions)
            //  - 15: up 2, left 1
            //  - 10: up 1, right 2
            //  - 6:  up 1, left 2
            // And the mirrored shifts downward:
            //  +17: down 2, left 1  (>> 17)
            //  +15: down 2, right 1 (>> 15)
            //  +10: down 1, left 2  (>> 10)
            //  +6:  down 1, right 2 (>> 6)
            //
            // But easier is to apply both >> and << variants with appropriate masks:

            // 1) Shift right 17 (i.e., bb >> 17): from rank ≥3 down to rank ≤5 positions
            //    This corresponds to “north‐north‐east” or “south‐south‐west” depending on orientation,
            //    but as long as masking is correct it yields valid target bits.

            set_bit(bb, idx);

            if ((bb >> 17) & not_h_file) attacks_w |= (bb >> 17);

            // 2) Shift right 15 (bb >> 15)
            if ((bb >> 15) & not_a_file) attacks_w |= (bb >> 15);

            // 3) Shift right 10 (bb >> 10)
            if ((bb >> 10) & not_hg_file) attacks_w |= (bb >> 10);

            // 4) Shift right 6 (bb >> 6)
            if ((bb >> 6) & not_ab_file) attacks_w |= (bb >> 6);

            // 5) Shift left 17 (bb << 17)
            if ((bb << 17) & not_a_file) attacks_w |= (bb << 17);

            // 6) Shift left 15 (bb << 15)
            if ((bb << 15) & not_h_file) attacks_w |= (bb << 15);

            // 7) Shift left 10 (bb << 10)
            if ((bb << 10) & not_ab_file) attacks_w |= (bb << 10);

            // 8) Shift left 6 (bb << 6)
            if ((bb << 6) & not_hg_file) attacks_w |= (bb << 6);

            knight_attack_bb[idx] = attacks_w;

            // clear bit for next iteration
            bb = 0ULL;
            attacks_w = 0ULL;
            attacks_b = 0ULL;

            // =================================================================================================================

            // ====================== KNIGHT GENERATION ========================================================================
            // This if statement are basically zeroing the bitboard in the exact position we want to avoid missbehavior (as the ones above)

            // set bit to current square on the board (idx)
            set_bit(bb, idx);

            // no checks here no need it will zero out the attack bit (which means no valid destination from this position)
            // North: shift << 8 (up one rank). No file restriction needed for vertical shift.
            attacks_b |= (bb << 8);

            // South: shift >> 8 (down one rank).
            attacks_b |= (bb >> 8);

            // East: shift << 1, but mask out those originally on file H (to avoid wraparound).
            attacks_b |= ( (bb & not_h_file) << 1 );

            // West: shift >> 1, but mask out those originally on file A.
            attacks_b |= ( (bb & not_a_file) >> 1 );

            // NE: shift << 9 = north + east; mask original on file H first.
            attacks_b |= ( (bb & not_h_file) << 9 );

            // NW: shift << 7 = north + west; mask original on file A first.
            attacks_b |= ( (bb & not_a_file) << 7 );

            // SE: shift >> 7 = south + east; mask original on file H first.
            attacks_b |= ( (bb & not_h_file) >> 7 );

            // SW: shift >> 9 = south + west; mask original on file A first.
            attacks_b |= ( (bb & not_a_file) >> 9 );

            king_attack_bb[idx] = attacks_b;

            // clear bit for next iteration
            bb = 0ULL;
            attacks_w = 0ULL;
            attacks_b = 0ULL;

            // =================================================================================================================
        }
    }
}

void compute_bishop_relevant_occupancy_bb()
{
    // Directions for bishop rays: NE, NW, SE, SW
    const int dirs[4][2] =
    {
        {+1, +1},  // NE
        {-1, +1},  // NW
        {+1, -1},  // SE
        {-1, -1}   // SW
    };

    // Loop over every square on the board
    for (int sq = 0; sq < 64; ++sq)
    {
        // Compute file (0..7 for a..h) and rank (0..7 for 1..8)
        int file0 = sq & 7;     // same as (sq % 8)
        int rank0 = sq >> 3;    // same as (sq / 8)
        // Here I'm initializing the mask for this square
        bitboard mask = 0ULL;

        // For each diagonal direction, collect ray squares then exclude the terminal edge
        for (auto &d : dirs)
        {
            int df = d[0], dr = d[1];
            int f = file0, r = rank0;
            // I’ll collect all squares on the ray in a temporary vector
            std::vector<int> ray_squares;
            while (true)
            {
                f += df;
                r += dr;
                // If I stepped off-board, stop collecting
                if (f < 0 || f > 7 || r < 0 || r > 7)
                    break;

                int sq2 = r * 8 + f;
                ray_squares.push_back(sq2);
            }
            // Now ray_squares holds all squares from one step out to the edge.
            // To exclude the terminal edge square, I pop the last element if non-empty.
            if (!ray_squares.empty())
                ray_squares.pop_back();

            // Now I set bits for the remaining squares in the mask
            for (int sq2 : ray_squares)
                set_bit(mask, sq2);

            // Done with this direction; move on to next diagonal
        }
        // Store the computed relevant-occupancy mask
        bishop_relevant_sqrs_bb[sq] = mask;
    }
}

void compute_rook_relevant_occupancy_bb()
{
    // Directions for rook rays: East, West, North, South
    const int dirs[4][2] =
    {
        {+1,  0},  // East
        {-1,  0},  // West
        { 0, +1},  // North
        { 0, -1}   // South
    };

    // Loop over every square on the board
    for (int sq = 0; sq < 64; ++sq)
    {
        // Compute file (0..7 for a..h) and rank (0..7 for 1..8)
        int file0 = sq & 7;      // here I'm extracting the file via bitwise AND
        int rank0 = sq >> 3;     // here I'm extracting the rank via shift (divide by 8)
        bitboard mask = 0ULL;    // initializing the relevant mask for this rook square

        // For each orthogonal direction, collect ray squares then exclude the terminal edge
        for (auto &d : dirs)
        {
            int df = d[0], dr = d[1];
            int f = file0, r = rank0;
            // I’ll collect all squares along this ray in a temporary vector
            std::vector<int> ray_squares;
            while (true)
            {
                f += df;
                r += dr;
                // If I step off-board, I stop collecting for this direction
                if (f < 0 || f > 7 || r < 0 || r > 7)
                    break;

                int sq2 = r * 8 + f;
                ray_squares.push_back(sq2);
            }
            // Now ray_squares holds all squares from one step out to the edge.
            // To exclude the terminal edge square, I remove the last element if non-empty.
            if (!ray_squares.empty())
                ray_squares.pop_back();

            // Now I OR each remaining square’s bit into the mask
            for (int sq2 : ray_squares)
                set_bit(mask, sq2);

            // Done processing this direction; move to the next orthogonal ray
        }
        // Store the computed relevant-occupancy mask for the rook on square sq
        rook_relevant_sqrs_bb[sq] = mask;
    }
}

void compute_queen_relevant_occupancy_bb()
{
    // Directions for queen rays: combination of rook and bishop directions
    const int dirs[8][2] =
    {
        {+1,  0},  // East
        {-1,  0},  // West
        { 0, +1},  // North
        { 0, -1},  // South
        {+1, +1},  // NE
        {-1, +1},  // NW
        {+1, -1},  // SE
        {-1, -1}   // SW
    };

    // Loop over every square on the board
    for (int sq = 0; sq < 64; ++sq)
    {
        // Compute file (0..7 for a..h) and rank (0..7 for 1..8)
        int file0 = sq & 7;    // here I extract the file via bitwise AND
        int rank0 = sq >> 3;   // here I extract the rank via shift (divide by 8)
        // I initialize the mask for this queen square
        bitboard mask = 0ULL;

        // For each of the eight directions, collect ray squares then exclude the terminal edge
        for (auto &d : dirs)
        {
            int df = d[0], dr = d[1];
            int f = file0, r = rank0;
            // I’ll collect all squares along this ray in a temporary vector
            std::vector<int> ray_squares;
            while (true)
            {
                f += df;
                r += dr;
                // If I step off-board, I stop collecting for this direction
                if (f < 0 || f > 7 || r < 0 || r > 7)
                    break;

                int sq2 = r * 8 + f;
                ray_squares.push_back(sq2);
            }
            // Now ray_squares holds all squares from one step out to the edge.
            // To exclude the terminal edge square, I remove the last element if non-empty.
            if (!ray_squares.empty())
                ray_squares.pop_back();

            // Now I OR each remaining square’s bit into the mask
            for (int sq2 : ray_squares)
                set_bit(mask, sq2);

            // Done processing this direction; move on to the next ray
        }

        // Store the computed relevant-occupancy mask for the queen on square sq
        queen_relevant_sqrs_bb[sq] = mask;
    }
}

bitboard compute_bishop_attack_bb(bitboard relevant_occupancy_bb, int sq)
{
    // Directions for bishop rays: NE, NW, SE, SW
    const int dirs[4][2] =
    {
        {+1, +1},  // NE
        {-1, +1},  // NW
        {+1, -1},  // SE
        {-1, -1}   // SW
    };

    // Compute file (0..7 for a..h) and rank (0..7 for 1..8)
    int file0 = sq & 7;     // same as (sq % 8)
    int rank0 = sq >> 3;    // same as (sq / 8)
    // Here I'm initializing the mask for this square
    bitboard mask = 0ULL;

    // For each diagonal direction
    for (auto &d : dirs)
    {
        int df = d[0], dr = d[1];
        int f = file0, r = rank0;

        while (true)
        {
            f += df;
            r += dr;
            // If I stepped off-board, stop searching that direction
            if (f < 0 || f > 7 || r < 0 || r > 7)
                break;

            // calculate the idx coordinates of the bit for setting in the mask
            int sq2 = r * 8 + f;

            // if blocking piece found we check the next direction but first set the bit
            if ( get_bit(relevant_occupancy_bb, sq2) != 0 )
            {
                // calculate the idx of the bit and then set it on the mask
                set_bit(mask, sq2);
                break;
            }
            set_bit(mask, sq2);
        }
        // Now ray_squares holds all attacked squares that relevant_occupancy_bb allows (including destination)
    }
    // Store the computed relevant-occupancy mask
    return mask;
}

bitboard compute_rook_attack_bb(bitboard relevant_occupancy_bb, int sq)
{
    // Directions for rook rays: East, West, North, South
    const int dirs[4][2] =
    {
        {+1,  0},  // East
        {-1,  0},  // West
        { 0, +1},  // North
        { 0, -1}   // South
    };

    // Compute file (0..7 for a..h) and rank (0..7 for 1..8)
    int file0 = sq & 7;     // same as (sq % 8)
    int rank0 = sq >> 3;    // same as (sq / 8)
    // Here I'm initializing the attack mask for this square
    bitboard mask = 0ULL;

    // For each orthogonal direction
    for (auto &d : dirs)
    {
        int df = d[0], dr = d[1];
        int f = file0, r = rank0;
        // I step square by square along the ray
        while (true)
        {
            f += df;
            r += dr;
            // If I stepped off-board, stop this direction
            if (f < 0 || f > 7 || r < 0 || r > 7)
                break;

            // Compute the destination square index
            int sq2 = r * 8 + f;

            // I include this square in mask (it is reachable or a capture)
            set_bit(mask, sq2);

            // If there's a blocker here (in relevant_occupancy_bb), I stop the ray
            if (get_bit(relevant_occupancy_bb, sq2) != 0)
            {
                // I found a blocker, so I break and move to next direction
                break;
            }
            // Otherwise I continue stepping further along the ray
        }
    }
    // After processing all four directions, mask holds all attacked squares
    return mask;
}

bitboard compute_queen_attack_bb(bitboard relevant_occupancy_bb, int sq) {
    // Directions for queen rays: combine rook + bishop directions
    const int dirs[8][2] =
    {
        {+1,  0},  // East
        {-1,  0},  // West
        { 0, +1},  // North
        { 0, -1},  // South
        {+1, +1},  // NE
        {-1, +1},  // NW
        {+1, -1},  // SE
        {-1, -1}   // SW
    };

    // Compute file (0..7 for a..h) and rank (0..7 for 1..8)
    int file0 = sq & 7;     // here I recover file via bitwise AND
    int rank0 = sq >> 3;    // here I recover rank via shift (divide by 8)
    // I initialize the attack mask for this queen square
    bitboard mask = 0ULL;

    // For each of the eight directions
    for (auto &d : dirs)
    {
        int df = d[0], dr = d[1];
        int f = file0, r = rank0;
        // I step square by square along this ray
        while (true)
        {
            f += df;
            r += dr;
            // If I stepped off-board, I stop this direction
            if (f < 0 || f > 7 || r < 0 || r > 7)
                break;

            // Compute the destination square index
            int sq2 = r * 8 + f;

            // I include this square in mask (reachable move or capture)
            set_bit(mask, sq2);

            // If there's a blocker here in relevant_occupancy_bb, I stop the ray
            if (get_bit(relevant_occupancy_bb, sq2) != 0)
            {
                // I found a blocker, so I break out to the next direction
                break;
            }
            // Otherwise, I continue stepping further along this ray
        }
        // Done processing one direction; moving to the next
    }
    // After all directions, mask holds all attacked squares
    return mask;
}

void print_bb(bitboard bb)
{
    printf("\nBoard State:\n\n");
    for(int rank=0; rank<8; rank++)
    {
        printf("%d  ", 8-rank);
        for(int file=0; file<8; file++)
        {
            // << 3 is * 2^3 = 8
            int idx = (rank << 3) + file;
            printf("%d ", get_bit(bb, idx) ? 1 : 0);
        }
        printf("\n");
    }
    printf("\n   ");
    for(int file = 97; file < 105; file++)
        printf("%c ", file);
    printf("\n\n");

    printf("Bitboard: %llud\n\n", bb);
    // const char* toMove = (turn) ? "Black" : "White";
    // printf("Moves: %s\n", toMove);

    // // AND bitwise operator to ask is this flag up (recall > 0 is true)
    // printf("Castle: %c%c%c%c\n",    (castle_right & Castle_Right::KC) ? 'K' : '-',
    //                                 (castle_right & Castle_Right::QC) ? 'Q' : '-',
    //                                 (castle_right & Castle_Right::kc) ? 'k' : '-',
    //                                 (castle_right & Castle_Right::qc) ? 'q' : '-' );

    // printf("En-Passant: %s\n", (en_passant==120) ? "-" : square_to_coord[en_passant]);
}

// bitboard king_attacks_from(bitboard bb) {
//     bitboard attacks = 0ULL;


// }