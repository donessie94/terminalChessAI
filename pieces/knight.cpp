#include "knight.h"
#include "../logic/chess.h"

KNIGHT::KNIGHT(POSITION pos, COLOR color) : PIECE(PIECE_TYPE::KNIGHT, color, pos) {}

// compute valid moves when player at turn is not in check, when in check another fucntion must be used
void KNIGHT::computeValidMoves(const POSITION& from, const CHESS& state) {
    movesCheck.clear();
    movesCapture.clear();
    movesDevelopment.clear();
    movesQuiet.clear();

    int pieceIdx = PIECE::pieceTypeToIndex(type);
    int fromIdx  = from.index;
    const auto &rawMoves = PIECE::rawMoveTable[pieceIdx][fromIdx];

    // int result = rawMoves.size();
    // SDL_Log("REPINGA result=%d", result);

    // Enemy king index for direct check test:
    int enemyKingIdx = (color == COLOR::WHITE ? state.bKingPosition.index
                                              : state.wKingPosition.index);

    for (const MOVE &m : rawMoves) {
        int toIdx = m.to.index;

        // 1) General rules: allied‐occupancy + pin‐check
        if (!generalRulesAllow(m, state)) {
            continue;
        }

        // 2) Is capture?
        const auto &destPtr = state.board[toIdx];
        bool isCapture = (destPtr && destPtr->color != color);

        // 3) Direct attack: after moving to m.to, does this piece itself attack enemy king?
        bool directCheck = false;
        {
            int toIdxLocal = m.to.index;
            // Use rawMoveTable for this piece type from destination square:
            const auto &destRaw = PIECE::rawMoveTable[pieceTypeToIndex(type)][toIdxLocal];
            // Check if any raw move lands on enemyKingIdx:
            for (const MOVE &rm : destRaw) {
                if (rm.to.index == enemyKingIdx) { // this cant be a check blocking move, those must be handled somewhere else
                    directCheck = true;
                    break;
                }
            }
        }

        // 4) Discovered attack: does moving uncover a slider attack on enemy king?
        bool discCheck = discoveredAttack(m, state);

        bool givesCheck = (directCheck || discCheck);

        // 5) isDevelopment heuristic
        bool isDev = false;
        if (!isCapture && !givesCheck) {
            if (color == COLOR::WHITE) {
                if (from.rank == 1 && m.to.rank > 1) isDev = true;
            } else {
                if (from.rank == 8 && m.to.rank < 8) isDev = true;
            }
        }

        // 6) Categorize
        if (givesCheck) {
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

// only moves possible by pieces that are not the king in this situation are blocking the check or capturing the attacking piece
// knight and pawns attack are unblockable, and if the piece is right next to the king attack is also unblockable
void KNIGHT::computeValidMovesInCheck(const POSITION& from, const CHESS& state, const std::vector<POSITION>& attackers) {
    movesCheck.clear();
    movesCapture.clear();
    movesDevelopment.clear();
    movesQuiet.clear();

    // 1) If more than 1 attacker, knight cannot block or capture both; no valid knight moves to resolve check.
    if (attackers.size() > 1) {
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

    // 2) Determine if unblockable by interposition
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

    int pieceIdx = PIECE::pieceTypeToIndex(type); // should be KNIGHT index
    int fromIdx  = from.index;
    const auto &rawMoves = PIECE::rawMoveTable[pieceIdx][fromIdx];

    // 3) If not unblockable, compute blocking squares between attacker and king
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

    // 4) Iterate raw knight moves
    for (const MOVE &m : rawMoves) {
        int toIdx = m.to.index;

        // 4a) General rules: allied‐occupancy + pin‐check
        if (!generalRulesAllow(m, state)) {
            continue;
        }

        // 4b) If capturing the attacker
        if (toIdx == attackerIdx) {
            // Capture attacker resolves check, if generalRulesAllow passed
            movesCapture.push_back(m);
            // Note: you could also check if capturing leaves king in check by some other piece,
            // but generalRulesAllow should already ensure your king isn't left in check by pin.
            continue;
        }

        // 4c) If blocking is possible (only when not unblockable)
        if (!unblockable) {
            // If this move lands on one of the blocking squares, it interposes between attacker and king
            // before the king; check that toIdx matches a blocking square:
            bool isBlockSquare = false;
            for (int bidx : blockingSquares) {
                if (toIdx == bidx) {
                    isBlockSquare = true;
                    break;
                }
            }
            if (isBlockSquare) {
                // This knight move blocks the slider’s check, and generalRulesAllow passed,
                // so it is a valid move to resolve check.
                movesQuiet.push_back(m);
                continue;
            }
        }

        // Otherwise: this knight move neither captures attacker nor blocks check → illegal while in check
    }

    // Done. movesCapture and movesQuiet now contain the knight moves that resolve check.
}