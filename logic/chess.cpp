#include "chess.h"

void CHESS::changeTurn(){
    currentPlayer = (currentPlayer == COLOR::WHITE) ? COLOR::BLACK : COLOR::WHITE;
    turnCount++;
    computeNewValidMoves(); // Recompute valid moves for the new player
}

CHESS::CHESS() :  board(64), wKingPosition(4), bKingPosition(60), check(false), checkMate(false) {
    currentPlayer = COLOR::WHITE;
    turnCount     = 0;

    auto place = [&](int f, int r, COLOR c, PIECE_TYPE t){
        int idx = r*8 + f;
        POSITION pos(idx);
        switch(t){
          case PIECE_TYPE::ROOK:   board[idx] = std::make_unique<ROOK>(pos,c);    break;
          case PIECE_TYPE::KNIGHT: board[idx] = std::make_unique<KNIGHT>(pos,c);  break;
          case PIECE_TYPE::BISHOP: board[idx] = std::make_unique<BISHOP>(pos,c);  break;
          case PIECE_TYPE::QUEEN:  board[idx] = std::make_unique<QUEEN>(pos,c);   break;
          case PIECE_TYPE::KING:   board[idx] = std::make_unique<KING>(pos,c);    break;
          case PIECE_TYPE::PAWN:   board[idx] = std::make_unique<PAWN>(pos,c);    break;
          case PIECE_TYPE::EMPTY:                                                 break;
          default: /* leave nullptr */                                            break;
        }
    };

    // white pawns on rank 1, black on rank 6
    for(int f=0;f<8;++f){
      place(f,1,COLOR::WHITE,PIECE_TYPE::PAWN);
      place(f,6,COLOR::BLACK,PIECE_TYPE::PAWN);
    }
    // rooks
    place(0,0,COLOR::WHITE,PIECE_TYPE::ROOK);
    place(7,0,COLOR::WHITE,PIECE_TYPE::ROOK);
    place(0,7,COLOR::BLACK,PIECE_TYPE::ROOK);
    place(7,7,COLOR::BLACK,PIECE_TYPE::ROOK);
    // knights
    place(1,0,COLOR::WHITE,PIECE_TYPE::KNIGHT);
    place(6,0,COLOR::WHITE,PIECE_TYPE::KNIGHT);
    place(1,7,COLOR::BLACK,PIECE_TYPE::KNIGHT);
    place(6,7,COLOR::BLACK,PIECE_TYPE::KNIGHT);
    // bishops
    place(2,0,COLOR::WHITE,PIECE_TYPE::BISHOP);
    place(5,0,COLOR::WHITE,PIECE_TYPE::BISHOP);
    place(2,7,COLOR::BLACK,PIECE_TYPE::BISHOP);
    place(5,7,COLOR::BLACK,PIECE_TYPE::BISHOP);
    // queen & king
    place(3,0,COLOR::WHITE,PIECE_TYPE::QUEEN);
    place(4,0,COLOR::WHITE,PIECE_TYPE::KING);
    place(3,7,COLOR::BLACK,PIECE_TYPE::QUEEN);
    place(4,7,COLOR::BLACK,PIECE_TYPE::KING);
}

void CHESS::movePiece(MOVE move)
{
}

void CHESS::computeNewValidMoves()
{

}
