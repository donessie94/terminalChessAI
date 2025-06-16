#include "chess.h"

void CHESS::changeTurn(){
    currentPlayer = (currentPlayer == COLOR::WHITE) ? COLOR::BLACK : COLOR::WHITE;
    turnCount++;
    computeNewValidMoves(); // Recompute valid moves for the new player
}

CHESS::CHESS() :  board(64), wKingPosition(4), bKingPosition(60), check(false), checkMate(false), staleMate(false) {
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
    computeNewValidMoves();
}

void CHESS::updateMoveHistory(const MOVE& move)
{
    MOVE m(move.from, move.to, move.pieceType);


    moveHistory.push_back(m);
}

bool CHESS::movePiece(const MOVE &move)
{
    //SDL_Log("movePiece called: %d->%d for player %s", move.from.index, move.to.index,
    //        currentPlayer==COLOR::WHITE?"WHITE":"BLACK");

    int fromIdx = move.from.index;
    int toIdx   = move.to.index;

    // Find the move in one of the valid‐move lists:
    bool isValid = false;
    bool isInCheckList = false;
    // Check movesCheck:
    for (const auto &m : movesCheck) {
        if (m.from.index == fromIdx && m.to.index == toIdx) {
            isValid = true;
            isInCheckList = true;
            break;
        }
    }
    // If not found yet, check movesCapture:
    if (!isValid) {
        for (const auto &m : movesCapture) {
            if (m.from.index == fromIdx && m.to.index == toIdx) {
                isValid = true;
                break;
            }
        }
    }
    // If still not found, check movesDevelopment:
    if (!isValid) {
        for (const auto &m : movesDevelopment) {
            if (m.from.index == fromIdx && m.to.index == toIdx) {
                isValid = true;
                break;
            }
        }
    }
    // If still not found, check movesQuiet:
    if (!isValid) {
        for (const auto &m : movesQuiet) {
            if (m.from.index == fromIdx && m.to.index == toIdx) {
                isValid = true;
                break;
            }
        }
    }

    if (!isValid) {
        // Move not in any valid‐move list: reject
        SDL_Log("  move not in any valid-move list!");
        return false;
    }

    // There is a piece at 'fromIdx'? If not, error.
    auto &srcPtr = board[fromIdx];
    if (!srcPtr) {
        SDL_Log("  no piece at fromIdx!");
        return false;
    }
    // (Optional) check srcPtr->color matches currentPlayer:
    if (srcPtr->color != currentPlayer) {
        SDL_Log("  piece color mismatch: piece.color=%s but currentPlayer=%s",
                srcPtr->getColorAsString().c_str(),
                currentPlayer==COLOR::WHITE?"WHITE":"BLACK");
        return false;
    }

    // Handle capture: if an enemy piece sits at toIdx, destroy it.
    auto &dstPtr = board[toIdx];
    if (dstPtr) {
        // Capturing: ensure it's enemy (should be guaranteed by valid‐move lists).
        if (dstPtr->color == currentPlayer) {
            SDL_Log("  destination occupied by allied piece!");
            // Shouldn't happen because generalRulesAllow and move lists skip allied landings.
            return false;
        }
        // Destroy the captured piece:
        dstPtr.reset();
    }
    else {
        // here I'm in the “no piece at toIdx” case → may be en passant
        if (srcPtr->type == PIECE_TYPE::PAWN) {
            // grab our Pawn subclass
            PAWN *p = static_cast<PAWN*>(srcPtr.get());
            if (p->enPassant) {
                // determine the index of the pawn being captured
                // white captures downward (enemy pawn sits one rank below dest)
                // black captures upward   (enemy pawn sits one rank above dest)
                int capIdx = (p->color == COLOR::WHITE
                            ? toIdx - 8
                            : toIdx + 8);

                auto &capPtr = board[capIdx];
                if (capPtr && capPtr->type == PIECE_TYPE::PAWN && capPtr->color != currentPlayer) {
                    // here I'm removing the captured pawn en passant
                    capPtr.reset();
                } else {
                    SDL_Log("  en passant failed: no capturable pawn at %d", capIdx);
                    return false;
                }
            }
        }
    }

    // at this point the move is valid so if there was any check prior we are safe to remove it
    // (we will put the check flag on again if player at turn gives check tho the next player at turn tho)
    if(check){
        check=false;
        attackers.clear();
    }


    // Update check/checkMate/attackers info:
    if(isInCheckList){
        attackers.clear();
        for(auto &pair : board[move.from.index]->directAttackInfo){
            if(move.from.index == pair.first.from.index && move.to.index == pair.first.to.index){
                attackers.push_back(pair.second);
            }
        }

        for(auto &pair : board[move.from.index]->discoveredAttackInfo){
            if(move.from.index == pair.first.from.index && move.to.index == pair.first.to.index){
                attackers.push_back(pair.second);
            }
        }

        check = true;
    }

    // Move the piece pointer:
    dstPtr = std::move(srcPtr);
    // Now board[fromIdx] (srcPtr) is nullptr.

    // Update the PIECE’s internal position:
    dstPtr->position.setPosition(toIdx);

    // If moved piece is king, update king position:
    if (dstPtr->type == PIECE_TYPE::KING) {
        if (currentPlayer == COLOR::WHITE) {
            wKingPosition.setPosition(toIdx);
        } else {
            bKingPosition.setPosition(toIdx);
        }
    }
    // Otherwise, no immediate king‐position change here.



    updateMoveHistory(move);

    // Advance turn:
    changeTurn();

    return true;
}

void CHESS::computeNewValidMoves()
{
    // Clear aggregated move lists
    movesCheck.clear();
    movesCapture.clear();
    movesDevelopment.clear();
    movesQuiet.clear();

    bool inCheck = this->check;
    // attackers vector is now filled if inCheck is true

    // Iterate through all squares; for each piece of currentPlayer, compute its moves
    for (const auto& piecePtr : board) {
        if (!piecePtr) continue;
        if (piecePtr->color != currentPlayer) continue;

        // Compute valid moves for this piece
        if (inCheck) {
            // Only moves that can resolve check: capture attacker or block
            piecePtr->computeValidMovesInCheck(piecePtr->position, *this, attackers);
        }
        else {
            // Normal move generation
            piecePtr->computeValidMoves(piecePtr->position, *this);
        }

        // Append this piece’s moves into CHESS aggregated lists:
        if (!piecePtr->movesCheck.empty()) {
            movesCheck.insert(movesCheck.end(),
                              piecePtr->movesCheck.begin(),
                              piecePtr->movesCheck.end());
        }
        if (!piecePtr->movesCapture.empty()) {
            movesCapture.insert(movesCapture.end(),
                                piecePtr->movesCapture.begin(),
                                piecePtr->movesCapture.end());
        }
        if (!piecePtr->movesDevelopment.empty()) {
            movesDevelopment.insert(movesDevelopment.end(),
                                   piecePtr->movesDevelopment.begin(),
                                   piecePtr->movesDevelopment.end());
        }
        if (!piecePtr->movesQuiet.empty()) {
            movesQuiet.insert(movesQuiet.end(),
                              piecePtr->movesQuiet.begin(),
                              piecePtr->movesQuiet.end());
        }
    }

    bool noMoveExists = !movesCheck.empty()
                       || !movesCapture.empty()
                       || !movesDevelopment.empty()
                       || !movesQuiet.empty();

    if (!noMoveExists) {
        // No legal moves at all for side to move:
        if (inCheck) {
            // Side to move is in check and has no legal moves: checkmate.
            checkMate = true;
        } else {
            // Side to move is not in check but has no legal moves: stalemate.
            staleMate = true;
        }
    }
}