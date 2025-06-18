#include "chess_0x88.h"

Chess_0x88::Chess_0x88() : turn(0), castle_right(0), en_passant(Square::no_sqr)
{
    zero_board(board);
}

Chess_0x88::Chess_0x88(const char fen[]) : turn(0), castle_right(0), en_passant(Square::no_sqr)
{
    parse_fen_str(fen, board, turn, castle_right, en_passant);
}

void Chess_0x88::pawn_generation(Piece type, int idx, int rank)
{
    // white to move
    if (type == Piece::P)
    {
        // non capture
        int search = idx - 16; bool promotion = false;
        for(int i=0; i<2; i++)
        {
            // if step not off-board
            if( ( (search) & 0x88 ) == 0 )
            {
                // pawn cant capture unless diagonal move
                if(board[search] > 0) break;
                // check if move is promotion (here move is valid so it must be a promotion if correct rank)
                if(rank == 1) promotion = true;

                printf("\nvalid pawn move: %s -> %s %s", square_to_coord[idx], square_to_coord[search], (promotion) ? "(P)" : " ");

                // IF WE PEREFORM A 2 STEP MOVE (easy en passant check)
                // // if i == 1 here that means a 2 square move is valid so en_passant flag should be set to the prior step
                // if(i==1)
                //     en_passant = idx - 16;

                // if not in the starting position cant perform 2 step move
                if(rank != 6) break;
                //else check if 2 step is possible
                search+=(-16);
            }
        }
        // capture move
        promotion = false; bool enPass = false;
        for(auto &step : pawn_capture_white)
        {
            // calculating target square index
            int search = idx + step;
            // if target square still on the board
            if( ( (search) & 0x88 ) == 0 )
            {
                // printf("\nen_passant value: %d and search: %d", en_passant, search);
                // if square target holds a black piece
                if(board[search] > 6 && board[search] != 13)
                {
                    // if the capture promotes
                    if(rank == 1) promotion = true;
                    printf("\nvalid pawn capture: %s -> %s %s", square_to_coord[idx], square_to_coord[search], (promotion) ? "(P)" : " ");
                }
                // if empty and enpassant rule flag allows it (no need to check for empty if enpassant rule says the square is an en passant square we trust it)
                else if (en_passant == search)
                {
                    en_passant = true;
                    printf("\nvalid pawn capture: %s -> %s %s", square_to_coord[idx], square_to_coord[search], (en_passant) ? "(enPass)" : " ");
                }
            }
        }
    }
    // black to move
    else if (type == Piece::p)
    {
        int search = idx + 16; bool promotion = false;
        for(int i=0; i<2; i++)
        {
            if( ( (search) & 0x88 ) == 0 )
            {
                if(board[search] > 0) break;
                if(rank == 6) promotion = true;
                printf("\nvalid pawn move: %s -> %s %s", square_to_coord[idx], square_to_coord[search], (promotion) ? "(P)" : " ");
                // if(i==1)
                //     en_passant = idx + 16;
                if(rank != 1) break;
                search+=16;
            }
        }
        promotion = false; bool enPass = false;
        for(auto &step : pawn_capture_black)
        {
            int search = idx + step;
            if( ( (search) & 0x88 ) == 0 )
            {
                if(board[search] < 7  && board[search] != 0)
                {
                    if(rank == 6) promotion = true;
                    printf("\nvalid pawn capture: %s -> %s %s", square_to_coord[idx], square_to_coord[search], (promotion) ? "(P)" : " ");
                }
                else if (en_passant == search)
                {
                    en_passant = true;
                    printf("\nvalid pawn capture: %s -> %s %s", square_to_coord[idx], square_to_coord[search], (en_passant) ? "(enPass)" : " ");
                }
            }
        }
    }
}

void Chess_0x88::castle_generation(Piece type, int idx)
{
    // the reason i am doing it like this and not prettier (sipmply checking if this square is attacked here) is for efficency
    // when castle right are no more then is a simple castle right check we perform instead of an is_square_attacked check every time

    // if white king
    if(type == Piece::K)
    {
        // recall AND will give us if the flag is up or not, and if a single bit is up then != 0, so => true
        // thus if this returns not false (hence true) the castling right is valid to this side
        if(castle_right & Castle_Right::KC)
        {
            // there have to be a clear path between rook and king on this side, we know rook exist and has not move
            // because the castle rights so no worries there, so all we must check is if these squares are empty
            if(board[Square::f1] == 0 && board[Square::g1] == 0)
            {
                // we check if the in between square is not under attack (note !turn because we are looking if the enemy (!turn) attacks this square)
                if( !is_square_attacked(board, !turn, idx) && !is_square_attacked(board, !turn, Square::f1) )
                {
                    printf("\nvalid king side castle: %s -> %s", square_to_coord[idx], square_to_coord[Square::g1]);
                }
            }
        }
        if(castle_right & Castle_Right::QC)
        {
            if(board[Square::d1] == 0 && board[Square::c1] == 0 && board[Square::b1] == 0)
            {
                if( !is_square_attacked(board, !turn, idx) && !is_square_attacked(board, !turn, Square::d1) )
                {
                printf("\nvalid queen side castle: %s -> %s", square_to_coord[idx], square_to_coord[Square::c1]);
                }
            }
        }
    }
    // if black king
    else if(type == Piece::k)
    {
        if(castle_right & Castle_Right::kc)
        {
            if(board[Square::f8] == 0 && board[Square::g8] == 0)
            {
                if( !is_square_attacked(board, !turn, idx) && !is_square_attacked(board, !turn, Square::f8) )
                {
                    printf("\nvalid king side castle: %s -> %s", square_to_coord[idx], square_to_coord[Square::g8]);
                }
            }
        }
        if(castle_right & Castle_Right::qc)
        {
            if(board[Square::d8] == 0 && board[Square::c8] == 0 && board[Square::b8] == 0)
            {
                if( !is_square_attacked(board, !turn, idx) && !is_square_attacked(board, !turn, Square::d8) )
                {
                    printf("\nvalid queen side castle: %s -> %s", square_to_coord[idx], square_to_coord[Square::c8]);
                }
            }
        }
    }
}

void Chess_0x88::generate_moves()
{
    for(int rank = 0; rank<8; rank++)
    {
        for(int file = 0; file<16; file++)
        {
            // take the 1D array to 2D formula
            int idx = rank * 16 + file;
            // if square is not off-board
            if( (idx & 0x88) == 0 )
            {
                // if white to move
                if(turn == 0)
                {
                    // if the square contains a white piece
                    if(board[idx] < 7  && board[idx] != 0)
                    {
                        switch (board[idx])
                        {
                            case Piece::P:
                                pawn_generation(Piece::P, idx, rank);
                            break;
                            case Piece::K:
                                castle_generation(Piece::K, idx);
                            break;
                            default:
                            break;
                        }
                    }
                }
                // if black to move
                else
                {
                    // if the square contains a black piece
                    if(board[idx] > 6 && board[idx] != 13)
                    {
                        switch (board[idx])
                        {
                            case Piece::p:
                                pawn_generation(Piece::p, idx, rank);
                            break;
                            case Piece::k:
                                castle_generation(Piece::k, idx);
                            break;
                            default:
                            break;
                        }
                    }
                }
            }
        }
    }
}
