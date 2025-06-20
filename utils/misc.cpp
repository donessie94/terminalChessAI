#include"misc.h"

// Define the FEN start string:
const char start_position[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
//const char tricky_position[] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
const char tricky_position[] = "8/8/8/8/8/8/PPPPPPPP/8 w Kq e4 0 1";

// Define ASCII/Unicode:
const char ascii_pieces[] = {
    'P', 'N', 'B', 'R', 'Q', 'K',
    'p', 'n', 'b', 'r', 'q', 'k', '.'
};
const char *unicode_pieces[] = {
    "♟", "♞", "♝", "♜", "♛", "♚",
    "♙", "♘", "♗", "♖", "♕", "♔", ".",
};

// Define coordinate lookup:
const char *square_to_coord[128] = {
    "a8","b8","c8","d8","e8","f8","g8","h8",
    "a7","b7","c7","d7","e7","f7","g7","h7",
    "a6","b6","c6","d6","e6","f6","g6","h6",
    "a5","b5","c5","d5","e5","f5","g5","h5",
    "a4","b4","c4","d4","e4","f4","g4","h4",
    "a3","b3","c3","d3","e3","f3","g3","h3",
    "a2","b2","c2","d2","e2","f2","g2","h2",
    "a1","b1","c1","d1","e1","f1","g1","h1","no_sqr"
};


// Define char_to_piece:
const int char_to_piece[] = {
    ['P'] = Piece::P, ['p'] = Piece::p,
    ['N'] = Piece::N, ['n'] = Piece::n,
    ['B'] = Piece::B, ['b'] = Piece::b,
    ['R'] = Piece::R, ['r'] = Piece::r,
    ['Q'] = Piece::Q, ['q'] = Piece::q,
    ['K'] = Piece::K, ['k'] = Piece::k,
    ['e'] = Piece::e
};

// Define char_to_type:
const int char_to_piece_type[] = {
    ['P'] = Piece_Type::Pawn,   ['p'] = Piece_Type::Pawn,
    ['N'] = Piece_Type::Knight, ['n'] = Piece_Type::Knight,
    ['B'] = Piece_Type::Bishop, ['b'] = Piece_Type::Bishop,
    ['R'] = Piece_Type::Rook,   ['r'] = Piece_Type::Rook,
    ['Q'] = Piece_Type::Queen,  ['q'] = Piece_Type::Queen,
    ['K'] = Piece_Type::King,   ['k'] = Piece_Type::King,
    ['e'] = Piece_Type::Empty
};

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
            printf("%d ", GET_BIT(bb, idx) ? 1 : 0);
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

void parse_fen_str(const char fen[], bitboard player_occ_bb[3], bitboard piece_occ_bb[2][6], bool &turn, int &castle_right, int &en_passant, int king_pos[])
{
    // 1) Zero out all occupancy bitboards:
    // Zero all bitboards in one shot:
    memset(player_occ_bb, 0, 3 * sizeof(bitboard));
    memset(piece_occ_bb, 0, 2 * 6 * sizeof(bitboard));

    // 3) Initialize side to move to a default
    turn = 0;

    // 4) Clear castling rights:
    castle_right = 0;

    // 5) Clear en-passant square:
    en_passant = Square::no_sqr;

    // 6) Clear king positions:
    king_pos[Color::white] = Square::no_sqr;
    king_pos[Color::black] = Square::no_sqr;

    // Now we parse the string and fill up the values
    int letter = 0;
    int spaces = 0;
    bool spaceFound = false;
    for(int rank = 0; rank < 8; rank++)
    {
        for(int file = 0; file < 8; file++)
        {
            // take the 1D array to 2D formula
            int idx = (rank << 3) + file;

            //
            if(spaceFound)
                continue;
            else
            {
                if(!(spaces > 0))
                {
                    if(fen[letter] == '/') letter++;
                    if(fen[letter] == ' ') { spaceFound = true; continue; }

                    // if it is a number
                    if(fen[letter] - '0' < 9 && fen[letter] - '0' > 0)
                    {
                        spaces = fen[letter] - '0' - 1;
                        CLEAR_ALL_SQUARE(Color::white, idx);
                        CLEAR_ALL_SQUARE(Color::black, idx);
                    }
                    // if its a letter (piece represenation)
                    else {
                        // if(fen[letter] == 'k' || fen[letter]  == 'K' )
                        // {
                        //     king_pos[ (fen[letter] == 'K') ? Color::white : Color::black ] = idx;
                        // }
                        // board[idx] = char_to_piece[fen[letter]];

                        // first leets figure the color out (if char_to_piece[letter] is from 0 to 5 is White) (piece representation)
                        //
                        // RECAL: enum Piece (representation) { P, N, B, R, Q, K, p, n, b, r, q, k, e };
                        //
                        // note caeful dont confuse with: (different)
                        //
                        // RECAL: enum Piece_Type { Pawn, Knight, Bishop, Rook, Queen, King, Empty };
                        //
                        int pc_rpt = char_to_piece[fen[letter]];
                        int p_type = char_to_piece_type[fen[letter]];
                        bool piece_color = (pc_rpt < 6) ? Color::white : Color::black;
                        //
                        SET_SQUARE_SIMPLE(piece_color, p_type, idx);
                    }
                }
                // here know "spaces" is active so we position empty spaces in the board
                // which translate to zero out this bit for all bitboards we have
                // i made a macro cuz ill have to do this many times i am sure
                else
                {
                    CLEAR_ALL_SQUARE(Color::white, idx);
                    CLEAR_ALL_SQUARE(Color::black, idx);
                    spaces--;
                    continue;
                }
            }
            letter++;
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
    int idx = (rankRow << 3) + fileIndex;    // index

    //printf("En-Passant: %s\n", square_to_coord[idx]);
    en_passant = idx;
}

void print_mini_board(bitboard player_occ_bb[3], bitboard piece_occ_bb[2][6], bool turn, int castle_right, int en_passant)
{
    printf("\nBoard State:\n\n");
    for(int rank = 0; rank<8; rank++)
    {
        printf(" %d  ", 8-rank);
        for(int file = 0; file<8; file++)
        {
            // take the 1D array to 2D formula
            int idx = (rank << 3) + file;

            // if this bitboard is set in the all player bitboard
            if(GET_BIT(player_occ_bb[Color::all_color], idx) )
            {
                bool piece_color = ( GET_BIT(player_occ_bb[Color::white], idx) ) ? Color::white : Color::black;

                // if color of piece is white
                if(piece_color == 0)
                {
                    // basically we loop trhough all white pieces bitboards to see wich one has this bit set, and wherever has it,
                    // thats the piece we need to represent
                    for(int piece=0; piece<6; piece++)
                    {
                        // we are looking for the set bit
                        if( GET_BIT(piece_occ_bb[Color::white][piece], idx) )
                        {
                            printf("%s ", unicode_pieces[piece]);
                        }
                    }

                }
                else
                {
                    // same for black
                    for(int piece=0; piece<6; piece++)
                    {
                        // we are looking for the set bit
                        if( GET_BIT(piece_occ_bb[Color::black][piece], idx) )
                        {
                            // if black now we need the PIECE REPRESENTATION so we must add + 6 for the offset
                            // RECALL
                            // enum Piece { P, N, B, R, Q, K, p, n, b, r, q, k, e };
                            // so for the representation + 6 -> Piece
                            // for the index no changes but -> Piece_Type
                            printf("%s ", unicode_pieces[piece+6]);
                        }
                    }
                }
            }
            // if the bit is not set then this is an empty square so print and continue
            else
            {
                printf("%s ", unicode_pieces[Piece::e]);
            }
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

    printf("En-Passant: %s\n", (en_passant==64) ? "-" : square_to_coord[en_passant]);
}

void print_attack_map(bitboard player_occ_bb[3], bitboard piece_occ_bb[2][6], const bool turn)
{
    printf("\nAttack Map:\n\n");
    for(int rank = 0; rank<8; rank++)
    {
        printf(" %d  ", 8-rank);
        for(int file = 0; file<8; file++)
        {
            // take the 1D array to 2D formula
            int idx = (rank << 3) + file;
            printf( "%s", ( IS_SQUARE_ATTACKED(turn, player_occ_bb[Color::all_color], idx) ) ? "x ": ". " );
        }
        printf("\n");
    }
    printf("\n    ");
    for(int file = 97; file < 105; file++)
        printf("%c ", file);
    printf("\n\n");

    const char* toMove = (turn) ? "Black" : "White";
    printf("Moves: %s\n", toMove);

}
