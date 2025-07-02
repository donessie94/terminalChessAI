#include "tables.h"

namespace RedStone{

namespace Tables{

Bitboard KING_ZONE[64];
//
Bitboard pawn_attack_bb[2][64];  // e.g. side 0=white, 1=black
Bitboard knight_attack_bb[64];
Bitboard king_attack_bb[64];

// max number of possible combination of relevant squares is 2^12 (rooks) and 2^9 (bishop)
Bitboard bishop_attack_bb[64][(1u<<9)];
Bitboard rook_attack_bb[64][(1u<<12)];

//
Slider_Magic rook_magic[64];
Slider_Magic bishop_magic[64];

// Pin checking helpers precomputed tables
Bitboard diagonal_ray_mask[64];
Bitboard orthogonal_ray_mask[64];
//
Bitboard between_squares_mask[64][64];

void initialize_precomputed_tables()
{
    //
    compute_leapers_attacks_bb();
    compute_bishop_relevant_occupancy_bb();
    compute_rook_relevant_occupancy_bb();
    compute_diagonal_ray_mask();
    compute_orthogonal_ray_mask();
    compute_between_squares_mask();
    //
    compute_bishop_attack_table();
    compute_rook_attack_table();
    build_king_zones();
}

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
// when first  bit is set ----> represents that element 1 in the set is included (if not set then this element is not included duh)
// when second bit is set ----> represents that element 2 in the set is included
// when third  bit is set ----> represents that element 3 in the set is included
// Thus we can use this fact to find all possible combinations

// CODE EXAMPLE
//for each possible binary combination (each integer representation in binary in that range basically)
//  for(int bin_representation=0; bin_representation<8; bin_representation++)
//  {
//     // holds the representation (different unquie subset)
//     Bitboard attack = 0ULL;

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

//
void compute_leapers_attacks_bb()
{
    Bitboard bb = 0ULL;
    Bitboard attacks_w = 0ULL;
    Bitboard attacks_b = 0ULL;

    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            // << 3 is * 2^3 = 8
            int idx = (rank << 3) + file;

            // set bit to current square on the board (idx)
            SET_BIT(bb, idx);

            // ====================== PAWN GENERATION =========================================================================
            // only way to shift 7 bits to the right and end up in the 'a' file is to start at 'h' file
            // thus if this is the case the expression will return 0 (false)
            // is this piece (pawn) starting in the column 'h'? (based on pawn movements shift 7) => if so this returns false
            // shift 7 right gives us the / attack for pawn (diagonal step to the right - up)
            if ((bb >> 7) & not_a_file)
                attacks_w |= (bb >> 7);   // if pawn did not started on column h then we can mark the attack Bitboard in this (Bitboard >> 7) position using OR

            // only way to shift 9 bits to the right and end up in 'h' file is by starting at 'a' file
            // this is '\' step (diagonal up-left)
            // NOTE RECALL BOARD IS INVERTED SO RIGHT IS COUNTING TO THE LEFT IN THE BOARD ABOVE <- this way from position
            if ((bb >> 9) & not_h_file)
                attacks_w |= (bb >> 9);

            // 0 is Color::white basically
            pawn_attack_bb[0][idx] = attacks_w;

            // for black basically the same inverted
            if ((bb << 7) & not_h_file)
                attacks_b |= (bb << 7);
            if ((bb << 9) & not_a_file)
                attacks_b |= (bb << 9);

            // 1 is Color::black
            pawn_attack_bb[1][idx] = attacks_b;

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

            SET_BIT(bb, idx);

            if ((bb >> 17) & not_h_file)
                attacks_w |= (bb >> 17);

            // 2) Shift right 15 (bb >> 15)
            if ((bb >> 15) & not_a_file)
                attacks_w |= (bb >> 15);

            // 3) Shift right 10 (bb >> 10)
            if ((bb >> 10) & not_hg_file)
                attacks_w |= (bb >> 10);

            // 4) Shift right 6 (bb >> 6)
            if ((bb >> 6) & not_ab_file)
                attacks_w |= (bb >> 6);

            // 5) Shift left 17 (bb << 17)
            if ((bb << 17) & not_a_file)
                attacks_w |= (bb << 17);

            // 6) Shift left 15 (bb << 15)
            if ((bb << 15) & not_h_file)
                attacks_w |= (bb << 15);

            // 7) Shift left 10 (bb << 10)
            if ((bb << 10) & not_ab_file)
                attacks_w |= (bb << 10);

            // 8) Shift left 6 (bb << 6)
            if ((bb << 6) & not_hg_file)
                attacks_w |= (bb << 6);

            knight_attack_bb[idx] = attacks_w;

            // clear bit for next iteration
            bb = 0ULL;
            attacks_w = 0ULL;
            attacks_b = 0ULL;

            // =================================================================================================================

            // ====================== KING GENERATION =========================================================================
            // This if statement are basically zeroing the Bitboard in the exact position we want to avoid missbehavior (as the ones above)

            // set bit to current square on the board (idx)
            SET_BIT(bb, idx);

            // no checks here no need it will zero out the attack bit (which means no valid destination from this position)
            // North: shift << 8 (up one rank). No file restriction needed for vertical shift.
            attacks_b |= (bb << 8);

            // South: shift >> 8 (down one rank).
            attacks_b |= (bb >> 8);

            // East: shift << 1, but mask out those originally on file H (to avoid wraparound).
            attacks_b |= ((bb & not_h_file) << 1);

            // West: shift >> 1, but mask out those originally on file A.
            attacks_b |= ((bb & not_a_file) >> 1);

            // NE: shift << 9 = north + east; mask original on file H first.
            attacks_b |= ((bb & not_h_file) << 9);

            // NW: shift << 7 = north + west; mask original on file A first.
            attacks_b |= ((bb & not_a_file) << 7);

            // SE: shift >> 7 = south + east; mask original on file H first.
            attacks_b |= ((bb & not_h_file) >> 7);

            // SW: shift >> 9 = south + west; mask original on file A first.
            attacks_b |= ((bb & not_a_file) >> 9);

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
    const int dirs[4][2] = {
        {+1, -1},  // NE (file+1, rank-1)
        {-1, -1},  // NW (file-1, rank-1)
        {+1, +1},  // SE (file+1, rank+1)
        {-1, +1},  // SW (file-1, rank+1)
    };

    // Loop over every square on the board
    for (int sq = 0; sq < 64; ++sq)
    {
        // Compute file (0..7 for a..h) and rank (0..7 for 1..8)
        int file0 = sq & 7;     // same as (sq % 8)
        int rank0 = sq >> 3;    // same as (sq / 8)
        // Here I'm initializing the mask for this square
        Bitboard mask = 0ULL;

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
                SET_BIT(mask, sq2);

            // Done with this direction; move on to next diagonal
        }
        // Store the computed relevant-occupancy mask
        bishop_magic[sq].relevant_sqrs_bb = mask;
    }
}

void compute_rook_relevant_occupancy_bb()
{
    // Orthogonal directions under mapping: rank0=0 is rank8, rank0=7 is rank1.
    const int dirs[4][2] = {
        {+1,  0},  // East:  file+1, rank unchanged
        {-1,  0},  // West:  file-1, rank unchanged
        { 0, -1},  // North: rank-1, file unchanged
        { 0, +1},  // South: rank+1, file unchanged
    };

    for (int sq = 0; sq < 64; ++sq)
    {
        int file0 = sq & 7;    // 0..7 for a..h
        int rank0 = sq >> 3;   // 0..7 for ranks 8..1
        Bitboard mask = 0ULL;

        for (auto &d : dirs)
        {
            int df = d[0], dr = d[1];
            int f = file0, r = rank0;
            std::vector<int> ray_squares;
            while (true)
            {
                f += df;
                r += dr;
                // stop if off-board
                if (f < 0 || f > 7 || r < 0 || r > 7)
                    break;
                int sq2 = r * 8 + f;
                ray_squares.push_back(sq2);
            }
            // exclude the terminal edge square:
            if (!ray_squares.empty())
                ray_squares.pop_back();
            // OR the remaining squares into mask
            for (int sq2 : ray_squares)
                mask |= (1ULL << sq2);
        }

        rook_magic[sq].relevant_sqrs_bb = mask;
    }
}

Bitboard compute_bishop_attack_bb(Bitboard relevant_occupancy_bb, int sq)
{
    // Directions for bishop rays: NE, NW, SE, SW
    const int dirs[4][2] = {
        {+1, -1},  // NE (file+1, rank-1)
        {-1, -1},  // NW (file-1, rank-1)
        {+1, +1},  // SE (file+1, rank+1)
        {-1, +1},  // SW (file-1, rank+1)
    };

    // Compute file (0..7 for a..h) and rank (0..7 for 1..8)
    int file0 = sq & 7;     // same as (sq % 8)
    int rank0 = sq >> 3;    // same as (sq / 8)
    // Here I'm initializing the mask for this square
    Bitboard mask = 0ULL;

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
            if ( GET_BIT(relevant_occupancy_bb, sq2) != 0 )
            {
                // calculate the idx of the bit and then set it on the mask
                SET_BIT(mask, sq2);
                break;
            }
            SET_BIT(mask, sq2);
        }
        // Now ray_squares holds all attacked squares that relevant_occupancy_bb allows (including destination)
    }
    // Store the computed relevant-occupancy mask
    return mask;
}

Bitboard compute_rook_attack_bb(Bitboard relevant_occupancy_bb, int sq)
{
    // Directions for rook rays: East, West, North, South
    const int dirs[4][2] = {
        {+1,  0},  // East:  file+1, rank unchanged
        {-1,  0},  // West:  file-1, rank unchanged
        { 0, -1},  // North: rank-1, file unchanged
        { 0, +1},  // South: rank+1, file unchanged
    };

    // Compute file (0..7 for a..h) and rank (0..7 for 1..8)
    int file0 = sq & 7;     // same as (sq % 8)
    int rank0 = sq >> 3;    // same as (sq / 8)
    // Here I'm initializing the attack mask for this square
    Bitboard mask = 0ULL;

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
            SET_BIT(mask, sq2);

            // If there's a blocker here (in relevant_occupancy_bb), I stop the ray
            if (GET_BIT(relevant_occupancy_bb, sq2) != 0)
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

// READ ABOVE I EXPLAIN IN DETAIL THIS FUNCTION AND HOW IT WORKS (ROOK AND QUEEN ARE SAME THING)
// basically we use a mapping function from 2^n number of configurations of the bits
// to binary representation of the number 2^n (note both are unquie so this exist an it is one to one)
void compute_bishop_attack_table()
{
    // we will do all 64 relevant square indeces here
    // t_idx = relevant square table index (which represents the indexes of the board)
    // thus if we talking about t_idx = 0 we are basically computing from a8 all possible attacks (given any combination of blockers)
    for(int t_idx=0; t_idx<64; t_idx++)
    {
        // it will store the relevant indices for any given relevant_occupancy Bitboard
        std::vector<int> relevant_indices_set;

        // our relevant_occupancy Bitboard being processed in this iteration
        Bitboard bb = bishop_magic[t_idx].relevant_sqrs_bb;

        // while "bb" still has a bit set we keep going
        // until we extract all the bits indeces in "bb"
        while (bb) {
            // count the trailing zeros to figure out index of the LS1B
            int sq = LS1B_IDX(bb);    // index 0..63 of LSB
            relevant_indices_set.push_back(sq);
            bb &= bb - 1;               // clear that bit (LS1B bit) so in next iteration we process the other bit after (wherever it is)
        }

        // same thing these 2
        // int N = relevant_indices_set.size() = count_bits(bishop_relevant_sqrs_bb[t_idx])
        int twoToN = 1ULL << COUNT_BITS(bishop_magic[t_idx].relevant_sqrs_bb) ;
        //int N = count_bits(bishop_magic[t_idx].relevant_sqrs_bb);
        int N = relevant_indices_set.size();

        // MAGIC NUMBER (BITBOARD) ====================================================================================================================
        // here we save the "magic_shift" needed after the magic Bitboard multiplication by this combination,
        // this is basically how many bits are in this possible combination (N bits), we need any combination of these bits after the
        // multiplication step to land on the top N bits of the resulting Bitboard ( or number, same)
        // we must find a constant M that basically make the top N bits unique (wich is the same as saying that contains this
        // unique arrangment of combination on the top) ex: max is 12 (for rooks) but think you have 2^64-12 = 2^52 numbers to land
        // ex 99999999999999999999999999999999 to 9999999999999999999999999999999 (one less 9) you multiply by something and landing there
        // is easy (thats a lot of numbers), recall also how multiplication works we can just overflow and start from 0 again (so no if the mask
        // has a 1 at the very end (huge number) it does not matter cuz we "go around")
        //
        // so once we have such constand M that wehn multiplied by the subset_bb on this combination yields different (2^N different patterns) on top
        // then we just shift right for wherever numbers of bits we had to find an unquie hash mapping that is perfect (one per each entrance) and fast
        // note: because is 1 per entrance we can simply use a static array and index yeeey (fast)
        //
        // quick note, top N bits cuz when sfhiting right will zero out all the garbage, at the start bit for example you will have to zero
        // out yourself the garbage and is not as fast as a simple shift operation
        bishop_magic[t_idx].shift = 64 - N; // CAREFULL the shift is 64-N not N itself

        // temporary dta that we need to fill the final slider_attack_table and its magics bitboards
        // basically saving here all occupancy subsets and all attack masks for each one of them
        std::vector<Bitboard> occ_subsets(twoToN);
        std::vector<Bitboard> attack_masks(twoToN);

        // since shifting by 64 in a 64 bits is undefined behavior we have to make sure we handle this special case when N = 0
        // even tho (1<<0) = 2^0 = 1 (thus 1 subset, the empty subset is correct) the problem is when generating the magic Bitboard
        // we will try to shift by 64 - N ( which is 0) so 64 - 0 = 64 PUM undefined
        if (N == 0) {
            bishop_magic[t_idx].magic_bb = 0ULL;
            bishop_magic[t_idx].shift = 0; // so (0 * magic) >> 0 == 0
            bishop_attack_bb[t_idx][0] = 0ULL;
            printf("ERROR: N is equal zero: %d\n", N);
            continue;
        }

        // shift N (x << N) basically multiply x by 2^N
        // for each different binary representation of the number 2^N where N=number of bits in the relevant_sqr Bitboard
        // br_idx = binary representation unique configuration
        for(int br_idx=0; br_idx < twoToN; br_idx++)
        {
            // our mapping function representation basically (holds an unique subset of
            // all unque possible configurations of the bits in the relevant_sqr Bitboard)
            Bitboard subset_bb = 0ULL;

            // check if the flag position that represents Yes or No (for including or no) a particular element of the Set is set
            for (int set_element_idx = 0; set_element_idx < N; set_element_idx++)
            {
                // in the binary representation from [0 - 2^N-1] (which maps one to one to the number of subsets of a set with N elements (2^N subsets))
                // if the specific position that represents Yes or No (include/exclude) for this element on the set is set then
                // we include that element of the set
                if( br_idx & (1ULL << set_element_idx) )
                {
                    //  since the bit is set we include the element of the set
                    subset_bb |= (1ULL << relevant_indices_set[set_element_idx]);
                }
            }
            // after each iteration we have an unique different configuration representation of the different/unique subset
            // basically a unquie different combination of possible pieces in the relevant_sqr Bitboard

            // save the particular combination (subset of the current relevant_sqrs_bb)
            occ_subsets[br_idx] = subset_bb;

            // Next: compute the attack mask for this (and eventually all once the loop is over) specific unique combination
            // for this specific square on the board
            Bitboard attack_bb = compute_bishop_attack_bb(subset_bb, t_idx);
            attack_masks[br_idx] = attack_bb;
        }

        // MAGIC NUMBER GENERATION ====================================================================================================================
        //
        //     	Initialize a random-number generator (e.g., std::mt19937_64 rng(seed)), where seed can be deterministic per square or from random device.
        // •	Repeat:
        // 1.	Generate a candidate M, e.g. uint64_t M = rng() | 1ULL; (ensuring odd is common, though not strictly required; some people also mask or combine random bits to get sparser patterns).
        // 2.	Allocate a temporary "used" array of size total, initialized to -1. This tracks which subset index mapped to each hash index.
        // 3.	For each subset i in [0..total-1]:
        // •	Compute idx = (occ_subsets[i] * M) >> shift;
        // •	If idx >= total, or if used[idx] != -1 (collision with a previous subset), reject this M immediately and break.
        // •	Otherwise set used[idx] = i and continue.
        // 4.	If the loop finishes without collision, M is valid. Record magic[sq] = M and magic_shift[sq] = shift, then break out of the search.
        // •	Empirically, because the space of 64-bit constants is huge relative to the small number of subsets (e.g., 2^12 = 4096), valid magic constants are plentiful and typically found after a modest number of tries.

        // if (t_idx % 8 == 7) {
        //     // last file in this rank: print value then newline
        //     printf("%d\n", N);
        // } else {
        //     // not last: print value and comma+space
        //     printf("%d, ", N);
        // }

        // 3. Search magic
        int shift = 64 - N;
        std::vector<int> used(twoToN, -1);
        // we provide a seed for reproducibility  0x924345B97F12fBBCULL
        std::mt19937_64 rng(t_idx ^ 0x022511947F12fBBCULL);
        std::uniform_int_distribution<Bitboard> dist;
        Bitboard magic = 0ULL;

        int attempts = 0;
        while (true) {
            attempts++;
            // if ((attempts % 1000000) == 0) {
            //     //printf("Searching magic for square %d: attempt %lld\n", t_idx, (long long)attempts);
            // }
            uint64_t M = (dist(rng) & dist(rng) & dist(rng)) | 1ULL; // sparser
            bool collision = false;
            std::fill(used.begin(), used.end(), -1);
            for (int i = 0; i < twoToN; i++) {
                uint64_t idx = (occ_subsets[i] * M) >> shift;
                if (idx >= (uint64_t)twoToN) {
                    //printf("  idx >= total: idx=%llu total=%d shift=%d\n", (unsigned long long)idx, twoToN, shift);
                    collision = true;
                    break;
                }
                if (used[idx] != -1) {
                    int prev = used[idx];
                    //printf("  Collision M=0x%016llx: subset %d and %d → idx=%llu\n",
                    //    (unsigned long long)M, prev, i, (unsigned long long)idx);
                    // Optionally dump their bitboards if helpful:
                    // print_bb(occ_subsets[prev]); print_bb(occ_subsets[i]);
                    collision = true;
                    break;
                }
                used[idx] = i;
            }
            if (!collision) {
                //printf("0x%016llxULL,\n", (unsigned long long)M);
                magic = M;
                break;
            }
            // Reseed if too many attempts: // 0xCAFEEDBABULL GOD LIKE SEED
            if (attempts % 1000000 == 0) {
                uint64_t newSeed = ((uint64_t)t_idx << 32) ^ (uint64_t)attempts ^ 0xCAFEEDBABULL;
                rng.seed(newSeed);
                //printf("  Reseeded RNG for square %d at attempt %lld\n", t_idx, (long long)attempts);
            }
        }
        //
        bishop_magic[t_idx].magic_bb = magic;

        // now we good so we store everything on our array for use
        // BE CAREFUL, OUR HASH IS PERFECT BUT THE ORDER IS "RANDOM" we only ensure one too one not that 0 is the first 1 is second and so on..
        // so wherever that order is we must keep here (this may give you the worst heache ever)
        for (int i = 0; i < twoToN; i++)
        {
            uint64_t idx = (occ_subsets[i] * magic) >> shift;
            bishop_attack_bb[t_idx][idx] = attack_masks[i];
        }

        // DEBUGING PURPOSES =================================================================================================================
        //
        // {
        //     int sq = t_idx;
        //     // Sanity: ensure stored shift matches
        //     if (bishop_magic[sq].shift != shift) {
        //         printf("Error: stored shift (%d) != localShift (%d) at sq %d\n",
        //             bishop_magic[sq].shift, shift, sq);
        //         exit(1);
        //     }
        //     // Loop all subsets
        //     for (int i = 0; i < twoToN; i++) {
        //         Bitboard subset = occ_subsets[i];
        //         // 1) naive attack
        //         Bitboard naive = compute_bishop_attack_bb(subset, sq);
        //         // 2) compare to the precomputed attack_masks[i]
        //         Bitboard stored = attack_masks[i];
        //         if (naive != stored) {
        //             printf("Validation ERROR: precomputed attack mismatch at sq %d subset %d\n", sq, i);
        //             printf("Subset Bitboard:\n"); print_bb(subset);
        //             printf("Naive attack:\n");    print_bb(naive);
        //             printf("attack_masks[%d]:\n", i); print_bb(stored);
        //             exit(1);
        //         }
        //         // 3) compute magic index
        //         uint64_t idx = (subset * magic) >> shift;
        //         if (idx >= (uint64_t)twoToN) {
        //             printf("Validation ERROR: magic idx out of range at sq %d subset %d idx=%llu (twoToN=%d)\n",
        //                 sq, i, (unsigned long long)idx, twoToN);
        //             printf("Subset Bitboard:\n"); print_bb(subset);
        //             exit(1);
        //         }
        //         // 4) lookup via the TABLE
        //         Bitboard lookup = bishop_attack_bb[sq][idx];
        //         if (lookup != naive) {
        //             printf("Validation ERROR: lookup via magic mismatch at sq %d subset %d idx=%llu\n",
        //                 sq, i, (unsigned long long)idx);
        //             printf("Subset Bitboard:\n"); print_bb(subset);
        //             printf("Naive attack:\n");    print_bb(naive);
        //             printf("Table entry [idx=%llu]:\n", (unsigned long long)idx); print_bb(lookup);
        //             exit(1);
        //         }
        //     }
        //     // If we reach here, all subsets for this square passed validation
        //     // printf("Square %d: all %d subsets validated OK\n", sq, twoToN);
        // }
        //
        // ================================================================================================================================

    }
    //printf("\n");
}

void compute_rook_attack_table()
{
    for(int t_idx=0; t_idx<64; t_idx++)
    {
        std::vector<int> relevant_indices_set;
        Bitboard bb = rook_magic[t_idx].relevant_sqrs_bb;
        while (bb) {
            int sq = LS1B_IDX(bb);
            relevant_indices_set.push_back(sq);
            bb &= bb - 1;
        }
        int twoToN = 1ULL << COUNT_BITS(rook_magic[t_idx].relevant_sqrs_bb) ;
        int N = COUNT_BITS(rook_magic[t_idx].relevant_sqrs_bb);
        rook_magic[t_idx].shift = 64 - N;
        std::vector<Bitboard> occ_subsets(twoToN);
        std::vector<Bitboard> attack_masks(twoToN);
        if (N == 0) {
            rook_magic[t_idx].magic_bb = 0ULL;
            rook_magic[t_idx].shift = 0; // so (0 * magic) >> 0 == 0
            rook_attack_bb[t_idx][0] = 0ULL;
            continue;
        }
        for(int br_idx=0; br_idx < twoToN; br_idx++)
        {
            Bitboard subset_bb = 0ULL;
            for (int set_element_idx = 0; set_element_idx < N; set_element_idx++)
            {
                if( br_idx & (1ULL << set_element_idx) )
                    subset_bb |= (1ULL << relevant_indices_set[set_element_idx]);
            }
            occ_subsets[br_idx] = subset_bb;
            Bitboard attack_bb = compute_rook_attack_bb(subset_bb, t_idx);
            attack_masks[br_idx] = attack_bb;
        }
        int shift = 64 - N;
        std::vector<int> used(twoToN, -1);
        //std::mt19937_64 rng(t_idx ^ 0x924345B97FAF7C15ULL);
        std::mt19937_64 rng(t_idx ^ 0x4DC7B62FCA65190ULL);
        std::uniform_int_distribution<Bitboard> dist;
        Bitboard magic = 0ULL;
        int attempts =0;

        // if (t_idx % 8 == 7) {
        //     // last file in this rank: print value then newline
        //     printf("%d\n", N);
        // } else {
        //     // not last: print value and comma+space
        //     printf("%d, ", N);
        // }
        while (true) {
            attempts++;
            if ((attempts % 10000000) == 0) {
                //printf("Searching magic for square %d: attempt %lld\n", t_idx, (long long)attempts);
            }
            uint64_t M = (dist(rng) & dist(rng) & dist(rng));// | 1ULL; // sparser
            bool collision = false;
            std::fill(used.begin(), used.end(), -1);
            for (int i = 0; i < twoToN; i++) {
                uint64_t idx = (occ_subsets[i] * M) >> shift;
                if (idx >= (uint64_t)twoToN) {
                    //printf("  idx >= total: idx=%llu total=%d shift=%d\n", (unsigned long long)idx, twoToN, shift);
                    collision = true;
                    break;
                }
                if (used[idx] != -1) {
                    int prev = used[idx];
                    //printf("  Collision M=0x%016llx: subset %d and %d → idx=%llu\n",
                    //    (unsigned long long)M, prev, i, (unsigned long long)idx);
                    // Optionally dump their bitboards if helpful:
                    //print_bb(occ_subsets[prev]); print_bb(occ_subsets[i]);
                    collision = true;
                    break;
                }
                used[idx] = i;
            }
            if (!collision) {
                //printf("0x%016llxULL,\n", (unsigned long long)M);
                magic = M;
                break;
            }
            // Reseed if too many attempts:
            if (attempts % 1000000 == 0) {
                uint64_t newSeed = ((uint64_t)t_idx << 32) ^ (uint64_t)attempts ^ 0x022511947F12fBBCULL;
                rng.seed(newSeed);
                //printf("  Reseeded RNG for square %d at attempt %lld\n", t_idx, (long long)attempts);
            }
        }
        rook_magic[t_idx].magic_bb = magic;
        for (int i = 0; i < twoToN; i++)
        {
            uint64_t idx = (occ_subsets[i] * magic) >> shift;
            rook_attack_bb[t_idx][idx] = attack_masks[i];
        }
    }
}

void compute_diagonal_ray_mask()
{
    const int dirs[4][2] =
    {
        {+1, -1},  // NE (file+1, rank-1)
        {-1, -1},  // NW (file-1, rank-1)
        {+1, +1},  // SE (file+1, rank+1)
        {-1, +1},  // SW (file-1, rank+1)
    };

    // Loop over every square on the board
    for (int sq = 0; sq < 64; ++sq)
    {
        // Compute file (0..7 for a..h) and rank (0..7 for 1..8)
        int file0 = sq & 7;     // same as (sq % 8)
        int rank0 = sq >> 3;    // same as (sq / 8)
        // Here I'm initializing the mask for this square
        Bitboard mask = 0ULL;

        // For each diagonal direction, collect ray squares including the terminal edge
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

            // Now I set bits for the remaining squares in the mask
            for (int sq2 : ray_squares)
                SET_BIT(mask, sq2);

            // Done with this direction; move on to next diagonal
        }
        // Store the computed relevant-occupancy mask
        diagonal_ray_mask[sq] = mask;
    }
}

void compute_orthogonal_ray_mask()
{
    // Orthogonal directions under mapping: rank0=0 is rank8, rank0=7 is rank1.
    const int dirs[4][2] =
    {
        {+1,  0},  // East:  file+1, rank unchanged
        {-1,  0},  // West:  file-1, rank unchanged
        { 0, -1},  // North: rank-1, file unchanged
        { 0, +1},  // South: rank+1, file unchanged
    };

    for (int sq = 0; sq < 64; ++sq)
    {
        int file0 = sq & 7;    // 0..7 for a..h
        int rank0 = sq >> 3;   // 0..7 for ranks 8..1
        Bitboard mask = 0ULL;

        for (auto &d : dirs)
        {
            int df = d[0], dr = d[1];
            int f = file0, r = rank0;
            std::vector<int> ray_squares;
            while (true)
            {
                f += df;
                r += dr;
                // stop if off-board
                if (f < 0 || f > 7 || r < 0 || r > 7)
                    break;
                int sq2 = r * 8 + f;
                ray_squares.push_back(sq2);
            }
            // OR the remaining squares into mask
            for (int sq2 : ray_squares)
                mask |= (1ULL << sq2);
        }
        orthogonal_ray_mask[sq] = mask;
    }
}

void compute_between_squares_mask()
{
    for (int sq = 0; sq < 64; ++sq)
    {
        int file0 = sq & 7;
        int rank0 = sq >> 3;
        for (int sq2 = 0; sq2 < 64; ++sq2)
        {
            if (sq2 == sq)
            {
                between_squares_mask[sq][sq2] = 0ULL;
                continue;
            }
            int file1 = sq2 & 7;
            int rank1 = sq2 >> 3;
            int df = file1 - file0;
            int dr = rank1 - rank0;

            // Check alignment: same file, same rank, or same diagonal
            if (file0 == file1)
            {
                // same file (vertical)
                int step_f = 0;
                int step_r = sign(dr);
                Bitboard m = 0ULL;
                int r = rank0 + step_r;
                // walk until we reach rank1, but exclude endpoint:
                while (r != rank1)
                {
                    int sq_mid = (r << 3) + file0;
                    m |= (1ULL << sq_mid);
                    r += step_r;
                }
                between_squares_mask[sq][sq2] = m;
            }
            else if (rank0 == rank1)
            {
                // same rank (horizontal)
                int step_f = sign(df);
                int step_r = 0;
                Bitboard m = 0ULL;
                int f = file0 + step_f;
                while (f != file1)
                {
                    int sq_mid = (rank0 << 3) + f;
                    m |= (1ULL << sq_mid);
                    f += step_f;
                }
                between_squares_mask[sq][sq2] = m;
            }
            else if (abs(df) == abs(dr))
            {
                // same diagonal
                int step_f = sign(df);
                int step_r = sign(dr);
                Bitboard m = 0ULL;
                int f = file0 + step_f;
                int r = rank0 + step_r;
                while (!(f == file1 && r == rank1))
                {
                    int sq_mid = (r << 3) + f;
                    m |= (1ULL << sq_mid);
                    f += step_f;
                    r += step_r;
                }
                between_squares_mask[sq][sq2] = m;
            }
            else
            {
                // not aligned: no squares strictly between on a straight ray
                between_squares_mask[sq][sq2] = 0ULL;
            }
        }
    }
}

} // end Tables namespace
} // end RedStone namespace