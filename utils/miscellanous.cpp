#include "miscellanous.h"

void print_move_info_extended(unsigned int mv)
{
    unsigned src   = get_move_source(mv);
    unsigned dst   = get_move_destination(mv);
    unsigned promo = get_promotion_flag(mv);
    bool cap       = get_capture_flag(mv);
    bool dbl       = get_double_pawn_flag(mv);
    bool ep        = get_en_passant_flag(mv);
    bool cast      = get_castle_flag(mv);

    printf("Move encoding: unsigned: %u, hex: 0x%X\n", mv, mv);
    // Source
    printf("  Source index: %u (%s) ", src, square_to_coord[src]);
    printf("  Destination index: %u (%s)\n", dst, square_to_coord[dst]);

    // Promotion
    printf("  Promotion piece: %c\n", ascii_pieces[promo]);

    // Flags
    printf("  Capture flag:     %s\n", cap  ? "yes" : "no ");
    printf("  Double-pawn push: %s\n", dbl  ? "yes" : "no ");
    printf("  En-passant flag:  %s\n", ep   ? "yes" : "no ");
    printf("  Castling flag:    %s\n", cast ? "yes" : "no ");
}

void print_move_info(unsigned int mv)
{
    unsigned src   = get_move_source(mv);
    unsigned dst   = get_move_destination(mv);
    bool cap       = get_capture_flag(mv);
    unsigned promo = get_promotion_flag(mv);
    bool dbl       = get_double_pawn_flag(mv);
    bool ep        = get_en_passant_flag(mv);
    bool cast      = get_castle_flag(mv);

    printf("  %s -> ", square_to_coord[src]);
    printf("%s", square_to_coord[dst]);
    printf(" %s", cap  ? "yes" : "no ");
    printf(" %c", ascii_pieces[promo]);
    printf(" 2? %s", dbl  ? "yes" : "no ");
    printf(" ep: %s", ep   ? "yes" : "no ");
    printf(" c: %s\n", cast ? "yes" : "no ");
}

void print_mini_board(const int board[], bool turn, int castle_right, int en_passant)
{
    printf("\nBoard State:\n\n");
    for(int rank = 0; rank<8; rank++)
    {
        printf(" %d  ", 8-rank);
        for(int file = 0; file<16; file++)
        {
            // take the 1D array to 2D formula
            int idx = (rank << 4) + file;
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
            int idx = (rank << 4) + file;
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

void parse_fen_str(const char fen[], int board[], bool &turn, int &castle_right, int &en_passant, int king_pos[])
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
            int idx = (rank << 4) + file;
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
                        // if its a letter (piece represenation)
                        else {
                            if(fen[letter] == 'k' || fen[letter]  == 'K' )
                            {
                                king_pos[ (fen[letter] == 'K') ? Turn::white : Turn::black ] = idx;
                            }
                            board[idx] = char_to_piece[fen[letter]];
                        }
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
    int idx = (rankRow << 4) + fileIndex;    // 0x88 index

    //printf("En-Passant: %s\n", square_to_coord[idx]);
    en_passant = idx;
}
