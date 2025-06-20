#include"encoder.h"


//const char tricky_position[] = "8/3ppP2/8/4P3/4P3/8/3PP3/8 w KQkq - 0 1";

// Board array:
// int board[128] = {
//     // rank 8
//     r, n, b, q, k, b, n, r,   o, o, o, o, o, o, o, o,
//     // rank 7
//     p, p, p, p, p, p, p, p,   o, o, o, o, o, o, o, o,
//     // rank 6
//     e, e, e, e, e, e, e, e,   o, o, o, o, o, o, o, o,
//     // rank 5
//     e, e, e, e, e, e, e, e,   o, o, o, o, o, o, o, o,
//     // rank 4
//     e, e, e, e, e, e, e, e,   o, o, o, o, o, o, o, o,
//     // rank 3
//     e, e, e, e, e, e, e, e,   o, o, o, o, o, o, o, o,
//     // rank 2
//     P, P, P, P, P, P, P, P,   o, o, o, o, o, o, o, o,
//     // rank 1
//     R, N, B, Q, K, B, N, R,   o, o, o, o, o, o, o, o
// };




// Move offsets arrays
const int knight_step[8] = { -31, -14, +18, +33, +31, +14, -18, -33 };
const int bishop_step[4] = { -15, +17, +15, -17 };
const int rook_step[4]   = { -16, +1, +16, -1 };
const int queen_step[8]  = { -16, -15, +1, +17, +16, +15, -1, -17 };
const int king_step[8]   = { -16, -15, +1, +17, +16, +15, -1, -17 };
const int pawn_step[2]          = { -16, +16 };
const int pawn_double_step[2]   = { -32, +32 };
const int pawn_capture_white[2] = { -15, -17 };
const int pawn_capture_black[2] = { +17, +15 };