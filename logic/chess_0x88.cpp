#include "chess_0x88.h"

Chess_0x88::Chess_0x88() : turn(0), castle_right(0), en_passant(Square::no_sqr)
{
    zero_board(board);
}

Chess_0x88::Chess_0x88(const char fen[]) : turn(0), castle_right(0), en_passant(Square::no_sqr)
{
    parse_fen_str(fen, board, turn, castle_right, en_passant);
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
                            {
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

                                        // if not in the starting position cant perform 2 step move
                                        if(rank != 6) break;
                                        //else check if 2 step is possible
                                        search+=(-16);
                                    }
                                }
                            }
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
                            {
                                int search = idx + 16; bool promotion = false;
                                for(int i=0; i<2; i++)
                                {
                                    if( ( (search) & 0x88 ) == 0 )
                                    {
                                        if(board[search] > 0) break;
                                        if(rank == 6) promotion = true;
                                        printf("\nvalid pawn move: %s -> %s %s", square_to_coord[idx], square_to_coord[search], (promotion) ? "(P)" : " ");
                                        if(rank != 1) break;
                                        search+=16;
                                    }
                                }
                            }
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
