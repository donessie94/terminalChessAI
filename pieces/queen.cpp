#include "queen.h"
#include "../logic/chess.h"

QUEEN::QUEEN(POSITION pos, COLOR col) : PIECE(PIECE_TYPE::QUEEN, col, pos) {}

void QUEEN::computeValidMoves(const POSITION &from, const CHESS &state)
{
    movesCheck.clear();
    movesCapture.clear();
    movesDevelopment.clear();
    movesQuiet.clear();

    int pieceIdx = PIECE::pieceTypeToIndex(type);
    int fromIdx  = from.index;
    const auto &rawMoves = PIECE::rawMoveTable[pieceIdx][fromIdx];

    // Enemy king index for direct check test:
    int enemyKingIdx = (color == COLOR::WHITE ? state.bKingPosition.index
                                              : state.wKingPosition.index);

    for (const MOVE &m : rawMoves) {
        int toIdx = m.to.index;

        // General rules: allied‐occupancy + pin‐check
        if (!generalRulesAllow(m, state)) {
            continue;
        }
        // Is path clear?
        if (!isPathClearQueen(m, state)) {
            continue;
        }

        // Is capture?
        const auto &destPtr = state.board[toIdx];
        bool isCapture = (destPtr && destPtr->color != color);

        // Direct attack: after moving to m.to, does this piece itself attack enemy king?
        bool directCheck = false;
        {
            int toIdxLocal = m.to.index;
            // Use rawMoveTable for this piece type from destination square:
            const auto &destRaw = PIECE::rawMoveTable[pieceTypeToIndex(type)][toIdxLocal];
            // Check if any raw move lands on enemyKingIdx:
            for (const MOVE &rm : destRaw) {
                if (rm.to.index == enemyKingIdx) { // this cant be a check blocking move, those must be handled somewhere else
                    if(isPathClearQueen(rm, state)){
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
        // queen development
        if (color == COLOR::WHITE) {
            // here I'm checking if the white queen leaves the back rank (rank 1)
            if (from.rank == 1 && m.to.rank > 1) {
                // here I'm marking this queen move as development
                isDev = true;
            }
        } else {
            // here I'm checking if the black queen leaves the back rank (rank 8)
            if (from.rank == 8 && m.to.rank < 8) {
                // here I'm marking this queen move as development
                isDev = true;
            }
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

void QUEEN::computeValidMovesInCheck(const POSITION& from, const CHESS& state, const std::vector<POSITION>& attackers) {
    movesCheck.clear();
    movesCapture.clear();
    movesDevelopment.clear();
    movesQuiet.clear();

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

    int pieceIdx = PIECE::pieceTypeToIndex(type); // should be ROOK index
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

    // Iterate raw bishop moves
    for (const MOVE &m : rawMoves) {
        int toIdx = m.to.index;

        // General rules: allied‐occupancy + pin‐check
        if (!generalRulesAllow(m, state)) {
            continue;
        }
        // Is path clear?
        if (!isPathClearQueen(m, state)) {
            continue;
        }

        // If capturing the attacker:
        if (toIdx == attackerIdx) {
            // This move captures the checking piece. It resolves the check, but might also give check to opponent.
            bool directCheck = false;
            //  - Direct: from the new rook position, can it attack the enemy king?
            {
                const auto &destRaw = PIECE::rawMoveTable[pieceIdx][toIdx];
                for (const MOVE &rm : destRaw) {
                    if (rm.to.index == enemyKingIdx) {
                        if(isPathClearQueen(rm, state)){
                            directCheck = true;
                            break;
                        }
                    }
                }
            }
            //  - Discovered: does moving this rook uncover a sliding attack on the enemy king?
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
                // This rook move interposes between attacker and our king, resolving the check.
                // But it might give a check to the opponent afterward.
                bool directCheck = false;
                {
                    const auto &destRaw = PIECE::rawMoveTable[pieceIdx][toIdx];
                    for (const MOVE &rm : destRaw) {
                        if (rm.to.index == enemyKingIdx) {
                            if(isPathClearQueen(rm, state)){
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
