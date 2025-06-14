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
void KNIGHT::computeValidMovesInCheck(const POSITION& from, const CHESS& state, const std::vector<POSITION>& attackers) {
    movesCheck.clear();
    movesCapture.clear();
    movesDevelopment.clear();
    movesQuiet.clear();

    // if more than 1 attacker attacking the king it is impossible to block or capture a single piece that cancel the check
    if(attackers.size() > 1)
        return;

    int pieceIdx = PIECE::pieceTypeToIndex(type);
    int fromIdx  = from.index;
    const auto &rawMoves = PIECE::rawMoveTable[pieceIdx][fromIdx];

}