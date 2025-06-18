#include "chess_0x88.h"

// encode_move signature:
// constexpr unsigned int encode_move(unsigned source,
//                                unsigned target,
//                                unsigned promotedPiece,
//                                bool isCapture,
//                                bool isDoublePawn,
//                                bool isEnpassant,
//                                bool isCastling)
//
//move_.push_back(encode_move(idx, search, 0, 0, 0, 0, 0));

Chess_0x88::Chess_0x88() : turn(0), castle_right(0), en_passant(Square::no_sqr)
{
    zero_board(board);
    move_.reserve(256);
    king_pos[0] = Square::e1;
    king_pos[1] = Square::e8;
}

Chess_0x88::Chess_0x88(const char fen[]) : turn(0), castle_right(0), en_passant(Square::no_sqr)
{
    parse_fen_str(fen, board, turn, castle_right, en_passant);
    move_.reserve(256);
    king_pos[0] = Square::e1;
    king_pos[1] = Square::e8;
}

void Chess_0x88::pawn_generation(Piece type, int idx, int rank)
{
    // white to move
    if (type == Piece::P)
    {
        // non capture
        int search = idx - 16;
        for(int i=0; i<2; i++)
        {
            // if step not off-board
            if( ( (search) & 0x88 ) == 0 )
            {
                // pawn cant capture unless diagonal move
                if(board[search] > 0) break;

                //printf("\nvalid pawn move: %s -> %s %s", square_to_coord[idx], square_to_coord[search], (promotion) ? "(P)" : " ");

                // IF WE PEREFORM A 2 STEP MOVE (easy en passant check)
                // // if i == 1 here that means a 2 square move is valid so en_passant flag should be set to the prior step
                //en_passant = idx - 16;
                if(i==1)
                    move_.push_back(encode_move(idx, search, 0, 0, 1, 0, 0));
                // else is a single step move
                else
                {
                    // check if move is promotion (here move is valid so it must be a promotion if correct rank)
                    if(rank == 1)
                    {
                        move_.push_back(encode_move(idx, search, Piece::Q, 0, 0, 0, 0));
                        move_.push_back(encode_move(idx, search, Piece::R, 0, 0, 0, 0));
                        move_.push_back(encode_move(idx, search, Piece::B, 0, 0, 0, 0));
                        move_.push_back(encode_move(idx, search, Piece::N, 0, 0, 0, 0));
                    }
                    // not promotion single step
                    else
                        move_.push_back(encode_move(idx, search, 0, 0, 0, 0, 0));
                }

                // if not in the starting position cant perform 2 step move
                if(rank != 6) break;
                //else check if 2 step is possible
                search+=(-16);
            }
        }
        // capture move
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
                    if(rank == 1)
                    {
                        move_.push_back(encode_move(idx, search, Piece::Q, 1, 0, 0, 0));
                        move_.push_back(encode_move(idx, search, Piece::R, 1, 0, 0, 0));
                        move_.push_back(encode_move(idx, search, Piece::B, 1, 0, 0, 0));
                        move_.push_back(encode_move(idx, search, Piece::N, 1, 0, 0, 0));
                    }
                    else
                        move_.push_back(encode_move(idx, search, 0, 1, 0, 0, 0));
                    //printf("\nvalid pawn capture: %s -> %s %s", square_to_coord[idx], square_to_coord[search], (promotion) ? "(P)" : " ");

                }
                // if empty and enpassant rule flag allows it (no need to check for empty if enpassant rule says the square is an en passant square we trust it)
                else if (en_passant == search)
                {
                    //printf("\nvalid pawn capture: %s -> %s %s", square_to_coord[idx], square_to_coord[search], (en_passant) ? "(enPass)" : " ");
                    move_.push_back(encode_move(idx, search, 0, 1, 0, 1, 0));
                }
            }
        }
    }
    // black to move
    else if (type == Piece::p)
    {
        int search = idx + 16;
        for(int i=0; i<2; i++)
        {
            if( ( (search) & 0x88 ) == 0 )
            {
                if(board[search] > 0) break;
                if(i==1)
                    move_.push_back(encode_move(idx, search, 0, 0, 1, 0, 0));
                else
                {
                    if(rank == 6)
                    {
                        move_.push_back(encode_move(idx, search, Piece::q, 0, 0, 0, 0));
                        move_.push_back(encode_move(idx, search, Piece::r, 0, 0, 0, 0));
                        move_.push_back(encode_move(idx, search, Piece::b, 0, 0, 0, 0));
                        move_.push_back(encode_move(idx, search, Piece::n, 0, 0, 0, 0));
                    }
                    else
                        move_.push_back(encode_move(idx, search, 0, 0, 0, 0, 0));
                }
                if(rank != 1) break;
                search+=16;
            }
        }
        for(auto &step : pawn_capture_black)
        {
            int search = idx + step;
            if( ( (search) & 0x88 ) == 0 )
            {
                if(board[search] > 6 && board[search] != 13)
                {
                    if(rank == 6)
                    {
                        move_.push_back(encode_move(idx, search, Piece::q, 1, 0, 0, 0));
                        move_.push_back(encode_move(idx, search, Piece::r, 1, 0, 0, 0));
                        move_.push_back(encode_move(idx, search, Piece::b, 1, 0, 0, 0));
                        move_.push_back(encode_move(idx, search, Piece::n, 1, 0, 0, 0));
                    }
                    else
                        move_.push_back(encode_move(idx, search, 0, 1, 0, 0, 0));
                }
                else if (en_passant == search)
                    move_.push_back(encode_move(idx, search, 0, 1, 0, 1, 0));
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
                    //printf("\nvalid king side castle: %s -> %s", square_to_coord[idx], square_to_coord[Square::g1]);
                    move_.push_back(encode_move(idx, Square::g1, 0, 0, 0, 0, 1));
                }
            }
        }
        if(castle_right & Castle_Right::QC)
        {
            if(board[Square::d1] == 0 && board[Square::c1] == 0 && board[Square::b1] == 0)
            {
                if( !is_square_attacked(board, !turn, idx) && !is_square_attacked(board, !turn, Square::d1) )
                {
                    //printf("\nvalid queen side castle: %s -> %s", square_to_coord[idx], square_to_coord[Square::c1]);
                    move_.push_back(encode_move(idx, Square::c1, 0, 0, 0, 0, 1));
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
                    //printf("\nvalid king side castle: %s -> %s", square_to_coord[idx], square_to_coord[Square::g8]);
                    move_.push_back(encode_move(idx, Square::g8, 0, 0, 0, 0, 1));
                }
            }
        }
        if(castle_right & Castle_Right::qc)
        {
            if(board[Square::d8] == 0 && board[Square::c8] == 0 && board[Square::b8] == 0)
            {
                if( !is_square_attacked(board, !turn, idx) && !is_square_attacked(board, !turn, Square::d8) )
                {
                    //printf("\nvalid queen side castle: %s -> %s", square_to_coord[idx], square_to_coord[Square::c8]);
                    move_.push_back(encode_move(idx, Square::c8, 0, 0, 0, 0, 1));
                }
            }
        }
    }
}

// here we are not enforcing checks rules (knight and king moves)
void Chess_0x88::leaper_generation(Piece type, int idx)
{
    // lets loop trhough all possible king/knight steps
    for(auto &step : (type == Piece::K || type == Piece::k) ? king_step : knight_step)
    {
        // compute possible target square
        int search = idx + step;
        // if step not off-board
        if( ( (search) & 0x88 ) == 0 )
        {
            // if target destination is empty (quiet move)
            if(board[search] == 0)
            {
                //printf("\nvalid %s move: %s -> %s", ( (type == Piece::K || type == Piece::k) ? "king" : "knight" ) ,square_to_coord[idx], square_to_coord[search]);
                move_.push_back(encode_move(idx, search, 0, 0, 0, 0, 0));
            }
            // else if type == K or == N (white king/knight to move)
            // and if (board[search] > 6 && board[search] != 13) (target square holds a black piece, so white capture move for king/knight)
            // 2nd case when to move is black then we check if target destination holds a white piece (black capture)
            else if( ( type == Piece::K || type == Piece::N ) ? ( board[search] > 6 && board[search] != 13 ) : ( board[search] < 7  && board[search] != 0 ) )
            {
                //printf("\nvalid %s capture: %s -> %s", ( (type == Piece::K || type == Piece::k) ? "king" : "knight" ) ,square_to_coord[idx], square_to_coord[search]);
                move_.push_back(encode_move(idx, search, 0, 1, 0, 0, 0));
            }
        }
    }
}

// (rook and bishop moves)
void Chess_0x88::slide_generation(Piece type, int idx)
{
    // QUEEN
    if(type == Piece::Q || type == Piece::q)
    {
        for(auto &step : queen_step)
        {
            int search = idx + step;
            while ( ( (search) & 0x88 ) == 0 )
            {
                if(board[search] == 0)
                {
                    //printf("\nvalid queen move: %s -> %s", square_to_coord[idx], square_to_coord[search]);
                    move_.push_back(encode_move(idx, search, 0, 0, 0, 0, 0));
                    search += step;
                    continue;
                }
                else if( ( type == Piece::Q ) ? (board[search] > 6 && board[search] != 13) : (board[search] < 7  && board[search] != 0) )
                {
                    //printf("\nvalid queen capture: %s -> %s", square_to_coord[idx], square_to_coord[search]);
                    move_.push_back(encode_move(idx, search, 0, 1, 0, 0, 0));
                    break;
                }
                break;
            }
        }
        // if it is queen then no need to continue after calculating queen moves
        return;
    }

    // BISHOP AND ROOKS
    // lets loop trhough all possible bishop or rook steps (depending the piece was passed here)
    for(auto &step : (type == Piece::B || type == Piece::b) ? bishop_step : rook_step)
    {
        // compute possible target square
        int search = idx + step;
        // while the steps is not off-board
        while ( ( (search) & 0x88 ) == 0 )
        {
            // if target destination is empty (quiet move)
            if(board[search] == 0)
            {
                //printf("\nvalid %s move: %s -> %s", ( (type == Piece::B || type == Piece::b) ? "bishop" : "rook" ) ,square_to_coord[idx], square_to_coord[search]);
                move_.push_back(encode_move(idx, search, 0, 0, 0, 0, 0));
                // lets check the next step in this direction because we know this is a valid move (empty square)
                // and the next step may be valid since this square does not block next one
                search += step;
                continue;
            }
            // else if type == B or == R (white bishop/rook to move)
            // and if (board[search] > 6 && board[search] != 13) (target square holds a black piece, so white capture move for bishop/rook)
            // 2nd case when to move is black then we check if target destination holds a white piece (black capture)
            else if( ( type == Piece::B || type == Piece::R ) ? (board[search] > 6 && board[search] != 13) : (board[search] < 7  && board[search] != 0) )
            {
                //printf("\nvalid %s capture: %s -> %s", ( (type == Piece::B || type == Piece::b) ? "bishop" : "rook" ) ,square_to_coord[idx], square_to_coord[search]);
                move_.push_back(encode_move(idx, search, 0, 1, 0, 0, 0));
                // if we found a capture move this is the last possible move since this piece blocks continuing the attack vector
                break;
            }
            // this means this square is not empty nor ocuppied by an enemy piece (thus occupied by ally so we break this ray path has been blocked)
            break;
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
            int idx = (rank << 4) + file;
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
                            case Piece::P:  pawn_generation(Piece::P, idx, rank);   break;
                            case Piece::K:  leaper_generation(Piece::K, idx);
                                            castle_generation(Piece::K, idx);       break;
                            case Piece::N:  leaper_generation(Piece::N, idx);       break;
                            case Piece::B:  slide_generation(Piece::B, idx);        break;
                            case Piece::R:  slide_generation(Piece::R, idx);        break;
                            case Piece::Q:  slide_generation(Piece::Q, idx);        break;
                            default: break;
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
                            case Piece::p:  pawn_generation(Piece::p, idx, rank);   break;
                            case Piece::k:  leaper_generation(Piece::k, idx);
                                            castle_generation(Piece::k, idx);       break;
                            case Piece::n:  leaper_generation(Piece::n, idx);       break;
                            case Piece::b:  slide_generation(Piece::b, idx);        break;
                            case Piece::r:  slide_generation(Piece::r, idx);        break;
                            case Piece::q:  slide_generation(Piece::q, idx);        break;
                            default: break;
                        }
                    }
                }
            }
        }
    }
}
