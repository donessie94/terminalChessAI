#include"misc.h"

namespace RedStone{

namespace Utils{

void print_bb(Bitboard bb)
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

void parse_fen_str(const char fen[], Bitboard player_occ_bb[3], Bitboard piece_occ_bb[2][6], bool &turn, int &castle_right, int &en_passant, int king_pos[])
{
    // 1) Zero out all occupancy bitboards:
    // Zero all bitboards in one shot:
    memset(player_occ_bb, 0, 3 * sizeof(Bitboard));
    memset(piece_occ_bb, 0, 2 * 6 * sizeof(Bitboard));

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
                        Move_Gen::mailbox[white][idx] = Empty;
                        Move_Gen::mailbox[black][idx] = Empty;
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
                        if(fen[letter] == 'k' || fen[letter]  == 'K' )
                        {
                                king_pos[ (fen[letter] == 'K') ? Color::white : Color::black ] = idx;
                        }
                        int pc_rpt = Encoder::char_to_piece[fen[letter]];
                        int p_type = Encoder::char_to_piece_type[fen[letter]];
                        bool piece_color = (pc_rpt < 6) ? Color::white : Color::black;
                        //
                        SET_SQUARE_SIMPLE(piece_color, p_type, idx);

                        Piece_Type piece;
                        if(p_type == P | p_type == p)
                            piece=Pawn;
                        else if(p_type == N | p_type == n)
                            piece=Knight;
                        else if(p_type == B | p_type == b)
                            piece=Bishop;
                        else if(p_type == R | p_type == r)
                            piece=Rook;
                        else if(p_type == Q | p_type == q)
                            piece=Queen;
                        else if(p_type == K | p_type == k)
                            piece=King;

                        if(piece_color == Color::white)
                        {
                            Move_Gen::mailbox[white][idx] = piece;
                            Move_Gen::mailbox[black][idx] = Empty;
                        }
                        else
                        {
                            Move_Gen::mailbox[white][idx] = Empty;
                            Move_Gen::mailbox[black][idx] = piece;
                        }
                    }
                }
                // here know "spaces" is active so we position empty spaces in the board
                // which translate to zero out this bit for all bitboards we have
                // i made a macro cuz ill have to do this many times i am sure
                else
                {
                    CLEAR_ALL_SQUARE(Color::white, idx);
                    CLEAR_ALL_SQUARE(Color::black, idx);
                    Move_Gen::mailbox[white][idx] = Empty;
                    Move_Gen::mailbox[black][idx] = Empty;
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

    // Zobrist Hashing initial set up =========================================================================================

    // PIECES
    for(int sq=0; sq<64; sq++)
    {
        Piece_Type w_piece = Move_Gen::mailbox[white][sq];
        Piece_Type b_piece = Move_Gen::mailbox[black][sq];

        // if this square is not empty (in the mailbox)
        if(w_piece != Empty)
            Move_Gen::position_hash ^= Tables::zobrist_piece[w_piece][sq];

        // black
        else if(b_piece != Empty)
            Move_Gen::position_hash ^= Tables::zobrist_piece[b_piece][sq];
    }

    // AUXILIARS
    // side to move
    if (Move_Gen::turn == white)                            Move_Gen::position_hash ^= Tables::zobrist_aux[ZOB_SIDE_TO_MOVE];
    // castle rights
    if (Move_Gen::castle_right & Castle_Right::KC)          Move_Gen::position_hash ^= Tables::zobrist_aux[ZOB_CASTLE_WK];
    if (Move_Gen::castle_right & Castle_Right::QC)          Move_Gen::position_hash ^= Tables::zobrist_aux[ZOB_CASTLE_WQ];
    if (Move_Gen::castle_right & Castle_Right::kc)          Move_Gen::position_hash ^= Tables::zobrist_aux[ZOB_CASTLE_BK];
    if (Move_Gen::castle_right & Castle_Right::qc)          Move_Gen::position_hash ^= Tables::zobrist_aux[ZOB_CASTLE_BQ];
    // enpassant
    if(en_passant != no_sqr)
    {
        // infer file 0..7 from the square index
        int ep_file = en_passant & 7;  // since a1=0,…,h1=7,a2=8…h8=63
        // XOR in the corresponding ZobristAux slot
        Move_Gen::position_hash ^= Tables::zobrist_aux[ZOB_EP_FILE_A + ep_file];
    }

    // ========================================================================================================================
}

void print_mini_board(Bitboard player_occ_bb[3], Bitboard piece_occ_bb[2][6], bool turn, int castle_right, int en_passant)
{
    printf("\nBoard State:\n\n");
    for(int rank = 0; rank<8; rank++)
    {
        printf(" %d  ", 8-rank);
        for(int file = 0; file<8; file++)
        {
            // take the 1D array to 2D formula
            int idx = (rank << 3) + file;

            // if this Bitboard is set in the all player Bitboard
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
                            printf("%s ", Encoder::unicode_pieces[piece]);
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
                            printf("%s ", Encoder::unicode_pieces[piece+6]);
                        }
                    }
                }
            }
            // if the bit is not set then this is an empty square so print and continue
            else
            {
                printf("%s ", Encoder::unicode_pieces[Piece::e]);
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

    printf("En-Passant: %s\n", (en_passant==64) ? "-" : Encoder::square_to_coord[en_passant]);
}

void print_attack_map(Bitboard player_occ_bb[3], Bitboard piece_occ_bb[2][6], const bool turn)
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

} // end Utils namespace
} // end RedStone namespace