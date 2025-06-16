#include "pawn.h"
#include "../logic/chess.h"

PAWN::PAWN(POSITION pos, COLOR col) : PIECE(PIECE_TYPE::PAWN, col, pos) {}

void PAWN::computeValidMoves(const POSITION &from, const CHESS &state)
{
    movesCheck.clear();
    movesCapture.clear();
    movesDevelopment.clear();
    movesQuiet.clear();
    directAttackInfo.clear();
    discoveredAttackInfo.clear();

    int pieceIdx = PIECE::pieceTypeToIndex(type);
    int fromIdx  = from.index;
    const auto &rawMoves = PIECE::rawMoveTable[pieceIdx][fromIdx];

    // Enemy king index for direct check test:
    int enemyKingIdx = (color == COLOR::WHITE ? state.bKingPosition.index
                                              : state.wKingPosition.index);

    for (const MOVE &m : rawMoves) {
        int toIdx = m.to.index;

        // Pawn can’t move “backwards”
        // compute rank delta: white should go to higher rank, black to lower
        int rankDiff = m.to.rank - from.rank;
        if (color == COLOR::WHITE) {
            if (rankDiff <= 0) {
                // here I’m skipping any white pawn move that doesn’t advance
                continue;
            }
        } else {
            if (rankDiff >= 0) {
                // here I’m skipping any black pawn move that doesn’t retreat
                continue;
            }
        }

        // General rules: allied‐occupancy + pin‐check
        if (!generalRulesAllow(m, state)) {
            continue;
        }
        // Is path clear?
        if (!isPathClearPawn(m, state)) {
            continue;
        }

        // capture detection:
        const auto &destPtr = state.board[toIdx];
        bool isCapture     = (destPtr && destPtr->color != color);

        // diagonal must be capture; straight must NOT be capture
        if ( (m.to.file != from.file)   // diagonal?
            ? !isCapture              //   then reject if NOT a capture
            :  isCapture ) {          // straight? then reject if it IS a capture
            // here I'm skipping any pawn move that's invalid for its direction
            continue;
        }

        // Pawn direct‐attack: after moving to m.to, does this pawn attack the enemy king?
        bool directCheck = false;
        {
            int toIdxLocal = m.to.index;
            // Use the rawMoveTable for the pawn type from the destination square:
            const auto &destRaw = PIECE::rawMoveTable[pieceTypeToIndex(type)][toIdxLocal];
            // Traverse all pawn “raw” moves from that square:
            for (const MOVE &rm : destRaw) {
                // If any raw move lands on the king’s index…
                if (rm.to.index == enemyKingIdx) {
                    // …and it’s diagonal (file changed) → pawn is delivering check
                    if (rm.to.file != m.to.file) {
                        directCheck = true;
                        break;
                    }
                }
            }
        }

        // Discovered attack: does moving uncover a slider attack on enemy king?
        bool discCheck = discoveredAttack(m, state);

        bool givesCheck = (directCheck || discCheck);

        bool isDev = false;
        // pawn development: central pawns (c–f files) making a two-square advance
        // here I'm checking for a double‐step push
        bool isDoublePush = (std::abs(m.to.rank - from.rank) == 2);
        // here I'm considering files c, d, e, or f as “central”
        bool onCenterFile = (from.file == 'c' ||
                            from.file == 'd' ||
                            from.file == 'e' ||
                            from.file == 'f');
        // here I'm ensuring it's from the back rank (2 for White, 7 for Black)
        bool fromBackRank = (color == COLOR::WHITE ? from.rank == 2
                                                    : from.rank == 7);
        if (isDoublePush && onCenterFile && fromBackRank) {
            // here I'm marking this central pawn double‐step as development
            isDev = true;
        }

        // Categorize
        if (givesCheck) {
            if(directCheck){
                MOVE moveInfo(POSITION(m.from.index), POSITION(m.to.index), state.board[m.from.index]->type);
                POSITION attack(m.to.index);
                directAttackInfo.emplace_back(moveInfo, attack);
            }
            movesCheck.push_back(m);
        }
        else if (isCapture) {
            movesCapture.push_back(m);
        }
        else if (isDev) {
            movesDevelopment.push_back(m);
        }
        else {
            movesQuiet.push_back(m);
        }
    }
}

void PAWN::computeValidMovesInCheck(const POSITION& from, const CHESS& state, const std::vector<POSITION>& attackers) {
    movesCheck.clear();
    movesCapture.clear();
    movesDevelopment.clear();
    movesQuiet.clear();
    directAttackInfo.clear();
    discoveredAttackInfo.clear();

    // If more than 1 attacker, rook cannot block or capture both; no valid rook moves to resolve check.
    if (attackers.size() != 1) {
        return;
    }
    // Exactly one attacker
    POSITION attackerPos = attackers[0];
    int attackerIdx = attackerPos.index;
    // Identify attacker piece type
    const auto &attackerPtr = state.board[attackerIdx];
    if (!attackerPtr) {
        // Should not happen: attacker square must have a piece
        return;
    }
    PIECE_TYPE attackerType = attackerPtr->type;

    // Our king position
    const POSITION &kingPos = (color == COLOR::WHITE ? state.wKingPosition : state.bKingPosition);
    int kingIdx = kingPos.index;

    // Determine if unblockable by interposition
    bool unblockable = false;
    // If attacker is knight or pawn, check is always adjacent → unblockable
    if (attackerType == PIECE_TYPE::KNIGHT || attackerType == PIECE_TYPE::PAWN) {
        unblockable = true;
    } else {
        // For sliding piece (rook, bishop, queen), check adjacency:
        // If attacker is adjacent to king (no squares between), can't block.
        int df = std::abs(attackerPos.file - kingPos.file);
        int dr = std::abs(attackerPos.rank - kingPos.rank);
        if (std::max(df, dr) == 1) {
            // adjacent along rank/file/diag
            unblockable = true;
        }
    }

    int pieceIdx = PIECE::pieceTypeToIndex(type); // should be PAWN index
    int fromIdx  = from.index;
    const auto &rawMoves = PIECE::rawMoveTable[pieceIdx][fromIdx];

    // Precompute enemy king index
    int enemyKingIdx = (color == COLOR::WHITE
                        ? state.bKingPosition.index
                        : state.wKingPosition.index);

    // If not unblockable, compute blocking squares between attacker and king
    std::vector<int> blockingSquares;
    if (!unblockable) {
        // Ensure attacker and king are aligned along file, rank, or diagonal; they must be for a sliding check
        bool sameFile = (attackerPos.file == kingPos.file);
        bool sameRank = (attackerPos.rank == kingPos.rank);
        bool sameDiag = (attackerPos.diagonal == kingPos.diagonal);
        bool sameAnti = (attackerPos.antiDiagonal == kingPos.antiDiagonal);
        if (sameFile || sameRank || sameDiag || sameAnti) {
            int step = 0;
            // Determine step from attacker toward king
            if (sameFile) {
                // file aligned: compare rank
                step = (attackerPos.rank < kingPos.rank ? +8 : -8);
            }
            else if (sameRank) {
                // rank aligned: compare file
                step = (attackerPos.file < kingPos.file ? +1 : -1);
            }
            else if (sameDiag) {
                // "\" diagonal: index step ±9
                step = (attackerPos.rank < kingPos.rank ? +9 : -9);
            }
            else { // sameAnti
                // "/" diagonal: index step ±7
                step = (attackerPos.rank < kingPos.rank ? +7 : -7);
            }
            // Collect squares strictly between attacker and king
            int scan = attackerIdx + step;
            while (scan >= 0 && scan < 64 && scan != kingIdx) {
                blockingSquares.push_back(scan);
                scan += step;
            }
            // Note: if scan goes out of board before reaching kingIdx, alignment was faulty or bug,
            // but since we checked alignment via POSITION, this loop should reach kingIdx eventually.
        }
        // else: theoretically if not aligned, sliding piece couldn't be giving check; but input should guarantee alignment.
    }

    // Iterate raw PAWN moves
    for (const MOVE &m : rawMoves) {
        int toIdx = m.to.index;

        // Pawn can’t move “backwards”
        // compute rank delta: white should go to higher rank, black to lower
        int rankDiff = m.to.rank - from.rank;
        if (color == COLOR::WHITE) {
            if (rankDiff <= 0) {
                // here I’m skipping any white pawn move that doesn’t advance
                continue;
            }
        } else {
            if (rankDiff >= 0) {
                // here I’m skipping any black pawn move that doesn’t retreat
                continue;
            }
        }

        // General rules: allied‐occupancy + pin‐check
        if (!generalRulesAllow(m, state)) {
            continue;
        }
        // Is path clear?
        if (!isPathClearPawn(m, state)) {
            continue;
        }

        // capture detection:
        const auto &destPtr = state.board[toIdx];
        bool isCapture     = (destPtr && destPtr->color != color);

        // diagonal must be capture; straight must NOT be capture
        if ( (m.to.file != from.file)   // diagonal?
            ? !isCapture              //   then reject if NOT a capture
            :  isCapture ) {          // straight? then reject if it IS a capture
            // here I'm skipping any pawn move that's invalid for its direction
            continue;
        }

        // If capturing the attacker: (at this point this move is 100% valid so if this is the case then the move is a valid capture)
        if (toIdx == attackerIdx) {
            // This move captures the checking piece. It resolves the check, but might also give check to opponent.
            // Pawn direct‐attack: after moving to m.to, does this pawn attack the enemy king?
            bool directCheck = false;
            {
                int toIdxLocal = m.to.index;
                // Use the rawMoveTable for the pawn type from the destination square:
                const auto &destRaw = PIECE::rawMoveTable[pieceTypeToIndex(type)][toIdxLocal];
                // Traverse all pawn “raw” moves from that square:
                for (const MOVE &rm : destRaw) {
                    // If any raw move lands on the king’s index…
                    if (rm.to.index == enemyKingIdx) {
                        // …and it’s diagonal (file changed) → pawn is delivering check
                        if (rm.to.file != m.to.file) {
                            directCheck = true;
                            break;
                        }
                    }
                }
            }
            //  - Discovered: does moving this pawn uncover a sliding attack on the enemy king?
            bool discCheck = discoveredAttack(m, state);
            if (directCheck || discCheck) {
                if(directCheck){
                    MOVE moveInfo(POSITION(m.from.index), POSITION(m.to.index), state.board[m.from.index]->type);
                    POSITION attack(m.to.index);
                    directAttackInfo.emplace_back(moveInfo, attack);
                }
                movesCheck.push_back(m);
            } else {
                movesCapture.push_back(m);
            }
            continue;
        }

        // If blocking is possible (only when not unblockable)
        if (!unblockable) {
            bool isBlockSquare = false;
            for (int bidx : blockingSquares) {
                if (toIdx == bidx) {
                    isBlockSquare = true;
                    break;
                }
            }
            if (isBlockSquare) {
                // This pawn move interposes between attacker and our king, resolving the check.
                // But it might give a check to the opponent afterward.
                // Pawn direct‐attack: after moving to m.to, does this pawn attack the enemy king?
                bool directCheck = false;
                {
                    int toIdxLocal = m.to.index;
                    // Use the rawMoveTable for the pawn type from the destination square:
                    const auto &destRaw = PIECE::rawMoveTable[pieceTypeToIndex(type)][toIdxLocal];
                    // Traverse all pawn “raw” moves from that square:
                    for (const MOVE &rm : destRaw) {
                        // If any raw move lands on the king’s index…
                        if (rm.to.index == enemyKingIdx) {
                            // …and it’s diagonal (file changed) → pawn is delivering check
                            if (rm.to.file != m.to.file) {
                                directCheck = true;
                                break;
                            }
                        }
                    }
                }
                bool discCheck = discoveredAttack(m, state);
                if (directCheck || discCheck) {
                    if(directCheck){
                        MOVE moveInfo(POSITION(m.from.index), POSITION(m.to.index), state.board[m.from.index]->type);
                        POSITION attack(m.to.index);
                        directAttackInfo.emplace_back(moveInfo, attack);
                    }
                    movesCheck.push_back(m);
                } else {
                    movesQuiet.push_back(m);
                }
                continue;
            }
        }

        // Otherwise: this knight move neither captures attacker nor blocks check → illegal while in check
    }

    // Done. movesCapture and movesQuiet now contain the knight moves that resolve check.
}
