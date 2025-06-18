#include "miscellanous.h"

// Define the FEN start string:
const char start_position[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
const char tricky_position[] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
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

// Define coordinate lookup:
const char *square_to_coord[128] = {
    "a8","b8","c8","d8","e8","f8","g8","h8","i8","j8","k8","l8","m8","n8","o8","p8",
    "a7","b7","c7","d7","e7","f7","g7","h7","i7","j7","k7","l7","m7","n7","o7","p7",
    "a6","b6","c6","d6","e6","f6","g6","h6","i6","j6","k6","l6","m6","n6","o6","p6",
    "a5","b5","c5","d5","e5","f5","g5","h5","i5","j5","k5","l5","m5","n5","o5","p5",
    "a4","b4","c4","d4","e4","f4","g4","h4","i4","j4","k4","l4","m4","n4","o4","p4",
    "a3","b3","c3","d3","e3","f3","g3","h3","i3","j3","k3","l3","m3","n3","o3","p3",
    "a2","b2","c2","d2","e2","f2","g2","h2","i2","j2","k2","l2","m2","n2","o2","p2",
    "a1","b1","c1","d1","e1","f1","g1","h1","i1","j1","k1","l1","m1","n1","o1","p1"
};

// Define ASCII/Unicode:
const char ascii_pieces[] = {
    '.', 'P', 'N', 'B', 'R', 'Q', 'K',
    'p', 'n', 'b', 'r', 'q', 'k', 'o'
};
const char *unicode_pieces[] = {
    ".", "♟", "♞", "♝", "♜", "♛", "♚",
    "♙", "♘", "♗", "♖", "♕", "♔", "o"
};

// Define char_to_piece:
const int char_to_piece[] = {
    ['P'] = Piece::P, ['p'] = Piece::p,
    ['N'] = Piece::N, ['n'] = Piece::n,
    ['B'] = Piece::B, ['b'] = Piece::b,
    ['R'] = Piece::R, ['r'] = Piece::r,
    ['Q'] = Piece::Q, ['q'] = Piece::q,
    ['K'] = Piece::K, ['k'] = Piece::k,
    ['o'] = Piece::o, ['e'] = Piece::e
};

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

void print_mini_board(const int board[], bool turn, int castle_right, int en_passant)
{
    printf("\nBoard State:\n\n");
    for(int rank = 0; rank<8; rank++)
    {
        printf(" %d  ", 8-rank);
        for(int file = 0; file<16; file++)
        {
            // take the 1D array to 2D formula
            int idx = rank * 16 + file;
            if( (idx & 0x88) == 0 )
                printf("%s ", unicode_pieces[board[idx]]);
        }
        printf("\n");
    }
    printf("\n    ");
    for(int file = 97; file < 105; file++)
        printf("%c ", file);
    printf("\n\n");

    const char* toMove = (turn) ? "Black" : "White";
    printf("Moves: %s\n", toMove);

    // AND bitwise operator to ask is this flag up (recall > 0 is true)
    printf("Castle: %c%c%c%c\n",    (castle_right & Castle_Right::KC) ? 'K' : '-',
                                    (castle_right & Castle_Right::QC) ? 'Q' : '-',
                                    (castle_right & Castle_Right::kc) ? 'k' : '-',
                                    (castle_right & Castle_Right::qc) ? 'q' : '-' );

    printf("En-Passant: %s\n", (en_passant==120) ? "-" : square_to_coord[en_passant]);
}

// note the moves must be "inverted" here since we checking from the attacked square to the piece (backwards)
// this compute attacked squares for the player passed (turn boolean)
bool is_square_attacked(const int *board, const bool turn, const int idx)
{
    // we already know this square is on the board from the function call it (print_attack_map())
    // white pieces
    if(turn == 0)
    {
        // if starting square is occupied by ally piece we cant attack this piece
        // white piece enum are from 1-6, black pieces are from 7-12 (0 is empty and 13 is offboard)
        if(board[idx] < 7  && board[idx] != 0) return false;

        for(auto &step : pawn_capture_white)
        {
            int search = idx + (-step);
            // if destination step is also on the board and there is a valid white piece attacking there (thus attacking the square) then yes
            if( ( ( search & 0x88 ) == 0 ) && ( board[search] == Piece::P ) ) return true;
        }
        // for each attacking step of knight
        for(auto &step : knight_step)
        {
            int search = idx + (-step);
            if( ( ( search & 0x88 ) == 0 ) && ( board[search] == Piece::N ) ) return true;
        }
        for(auto &step : bishop_step)
        {
            // while the steps in any particular direction stays on the board (search all the way til offboard)
            int search = idx + (-step);
            while(( ( search & 0x88 ) == 0 ))
            {
                if (( board[search] == Piece::B )) return true;
                // if we find a piece that is not the white bishop first then vector ray is broken so we cant attack
                // the initial square we are checking because the path is blocked
                // thus we break to get out of inner loop and continue with next step check
                else if(board[search] != Piece::e) break;
                search += (-step);
            }
        }
        for(auto &step : rook_step)
        {
            int search = idx + (-step);
            while(( ( search & 0x88 ) == 0 ))
            {
                if (( board[search] == Piece::R )) return true;
                else if(board[search] != Piece::e) break;
                search += (-step);
            }
        }
        for(auto &step : queen_step)
        {
            int search = idx + (-step);
            while(( ( search & 0x88 ) == 0 ))
            {
                if (( board[search] == Piece::Q )) return true;
                else if(board[search] != Piece::e) break;
                search += (-step);
            }
        }
        for(auto &step : king_step)
        {
            int search = idx + (-step);
            if( ( ( search & 0x88 ) == 0 ) && ( board[search] == Piece::K ) ) return true;
        }
    }
    // black
    else
    {
        if(board[idx] > 6 && board[idx] != 13) return false;
        for(auto &step : pawn_capture_black)
        {
            int search = idx + (-step);
            if( ( ( search & 0x88 ) == 0 ) && ( board[search] == Piece::p ) ) return true;
        }
        for(auto &step : knight_step)
        {
            int search = idx + (-step);
            if( ( ( search & 0x88 ) == 0 ) && ( board[search] == Piece::n ) ) return true;
        }
        for(auto &step : bishop_step)
        {
            int search = idx + (-step);
            while(( ( search & 0x88 ) == 0 ))
            {
                if (( board[search] == Piece::b )) return true;
                else if(board[search] != Piece::e) break;
                search += (-step);
            }
        }
        for(auto &step : rook_step)
        {
            int search = idx + (-step);
            while(( ( search & 0x88 ) == 0 ))
            {
                if (( board[search] == Piece::r )) return true;
                else if(board[search] != Piece::e) break;
                search += (-step);
            }
        }
        for(auto &step : queen_step)
        {
            int search = idx + (-step);
            while(( ( search & 0x88 ) == 0 ))
            {
                if (( board[search] == Piece::q )) return true;
                else if(board[search] != Piece::e) break;
                search += (-step);
            }
        }
        for(auto &step : king_step)
        {
            int search = idx + (-step);
            if( ( ( search & 0x88 ) == 0 ) && ( board[search] == Piece::k ) ) return true;
        }
    }
    return false;
}

void print_attack_map(const int board[], const bool turn)
{
    printf("\nAttacking Map: %s\n\n", turn ? "Black" : "White");
    for(int rank = 0; rank<8; rank++)
    {
        printf(" %d  ", 8-rank);
        for(int file = 0; file<16; file++)
        {
            // take the 1D array to 2D formula
            int idx = rank * 16 + file;
            if( (idx & 0x88) == 0 )
                printf("%s", is_square_attacked(board, turn, idx) ? "x " : ". ");
        }
        printf("\n");
    }
    printf("\n    ");
    for(int file = 97; file < 105; file++)
        printf("%c ", file);
    printf("\n\n");
}

void zero_board(int board[])
{
    for(int rank = 0; rank<8; rank++)
    {
        for(int file = 0; file<16; file++)
        {
            // take the 1D array to 2D formula
            int idx = rank * 16 + file;
            if( (idx & 0x88) == 0 )
                board[idx] = Piece::e;
            else
                board[idx] = Piece::o;
        }
    }
}

void parse_fen_str(const char fen[], int board[], bool &turn, int &castle_right, int &en_passant)
{
    zero_board(board);
    int letter = 0;
    int spaces = 0;
    bool spaceFound = false;
    for(int rank = 0; rank < 8; rank++)
    {
        for(int file = 0; file < 16; file++)
        {
            // take the 1D array to 2D formula
            int idx = rank * 16 + file;
            // if in the board space
            if( (idx & 0x88) == 0 )
            {
                if(spaceFound)
                {
                    continue;
                }
                else
                {
                    if(!(spaces > 0))
                    {
                        if(fen[letter] == '/') letter++;
                        if(fen[letter] == ' ') { spaceFound = true; continue; }

                        if(fen[letter] - '0' < 9 && fen[letter] - '0' > 0)
                        {
                            spaces = fen[letter] - '0' - 1;
                            board[idx] = Piece::e;
                        }
                        else { board[idx] = char_to_piece[fen[letter]]; }
                    }
                    // here know "spaces" is active so we position empty spaces in the board
                    else
                    {
                        board[idx] = Piece::e;
                        spaces--;
                        continue;
                    }
                }
                letter++;
            }
            else { board[idx] = Piece::o; }
        }
    }

    // parsing turn
    turn = (fen[++letter] == 'w') ? 0 : 1;
    letter+=2; //ignoring white space

    // parsing castle rights
    while (fen[letter] != ' ')
    {
        switch (fen[letter])
        {
            // OR bitwise operator, OR with 1, 2, 4, 8 basically putting the bits there (since we start at 'castle_right' = 0)
            case 'K':   castle_right = castle_right | Castle_Right::KC; break;
            case 'k':   castle_right |= Castle_Right::kc;               break;
            case 'Q':   castle_right |= Castle_Right::QC;               break;
            case 'q':   castle_right |= Castle_Right::qc;               break;
            case '-':                                                   break;
        }
        letter++;
    }

    // skip white space
    letter++;
    // if letter is '-' it means no enpassant possible
    if(fen[letter]=='-') { en_passant = Square::no_sqr; return; }

    char fileChar = fen[letter];
    char rankChar = fen[letter + 1];
    //printf("En-Passant: %c%c\n", fileChar, rankChar);

    int fileIndex = fileChar - 'a';        // 0..7
    int rankDigit = rankChar - '0';        // 1..8
    int rankRow = 8 - rankDigit;           // 0..7 for 0x88
    int idx = rankRow * 16 + fileIndex;    // 0x88 index

    //printf("En-Passant: %s\n", square_to_coord[idx]);
    en_passant = idx;
}
