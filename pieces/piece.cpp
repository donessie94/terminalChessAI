#include "piece.h"
#include "../logic/chess.h"


PIECE::PIECE(PIECE_TYPE t, COLOR c, POSITION pos)
    : type(t), color(c), position(pos) {}

std::string PIECE::getTypeAsString() const
{
    switch (type) {
            case PIECE_TYPE::PAWN:   return "PAWN";
            case PIECE_TYPE::ROOK:   return "ROOK";
            case PIECE_TYPE::KNIGHT: return "KNIGHT";
            case PIECE_TYPE::BISHOP: return "BISHOP";
            case PIECE_TYPE::QUEEN:  return "QUEEN";
            case PIECE_TYPE::KING:   return "KING";
            case PIECE_TYPE::EMPTY:  return "EMPTY";
            default:                 return "UNKNOWN";
        }
}

std::string PIECE::getColorAsString() const { return (color == COLOR::WHITE) ? "WHITE" : "BLACK"; }

std::string PIECE::getPositionAsString() const { return position.toAlgebraicNotation(); }

int PIECE::pieceTypeToIndex(PIECE_TYPE pt) {
    switch (pt) {
        case PIECE_TYPE::PAWN:   return 0;
        case PIECE_TYPE::KNIGHT: return 1;
        case PIECE_TYPE::BISHOP: return 2;
        case PIECE_TYPE::ROOK:   return 3;
        case PIECE_TYPE::QUEEN:  return 4;
        case PIECE_TYPE::KING:   return 5;
    }
    // handle error
    return -1;
}

void PIECE::buildRawMoveTable()
{
    // 1) Clear any existing data
    for (int pt = 0; pt < 6; ++pt) {
        for (int sq = 0; sq < 64; ++sq) {
            rawMoveTable[pt][sq].clear();
        }
    }

    // 2) For each square 0..63...
    for (int fromIdx = 0; fromIdx < 64; ++fromIdx) {
        int file = fromIdx % 8;
        int rank = fromIdx / 8;
        POSITION from(fromIdx);

        // --- PAWN raw moves ---
        {
            auto &out = rawMoveTable[pieceTypeToIndex(PIECE_TYPE::PAWN)][fromIdx];
            // white-forward one
            if (rank + 1 < 8) {
                out.emplace_back(from, POSITION((rank+1)*8 + file), PIECE_TYPE::PAWN);
            }
            // white-forward two from rank 1
            if (rank == 1 && rank + 2 < 8) {
                out.emplace_back(from, POSITION((rank+2)*8 + file), PIECE_TYPE::PAWN);
            }
            // black-backward one
            if (rank - 1 >= 0) {
                out.emplace_back(from, POSITION((rank-1)*8 + file), PIECE_TYPE::PAWN);
            }
            // black-backward two from rank 6
            if (rank == 6 && rank - 2 >= 0) {
                out.emplace_back(from, POSITION((rank-2)*8 + file), PIECE_TYPE::PAWN);
            }
            // capture diagonals (both colors raw)
            if (rank + 1 < 8 && file + 1 < 8)
                out.emplace_back(from, POSITION((rank+1)*8 + (file+1)), PIECE_TYPE::PAWN);
            if (rank + 1 < 8 && file - 1 >= 0)
                out.emplace_back(from, POSITION((rank+1)*8 + (file-1)), PIECE_TYPE::PAWN);
            if (rank - 1 >= 0 && file + 1 < 8)
                out.emplace_back(from, POSITION((rank-1)*8 + (file+1)), PIECE_TYPE::PAWN);
            if (rank - 1 >= 0 && file - 1 >= 0)
                out.emplace_back(from, POSITION((rank-1)*8 + (file-1)), PIECE_TYPE::PAWN);
        }

        // --- KNIGHT raw moves ---
        {
            static constexpr int KDX[8] = { +1, +2, +2, +1, -1, -2, -2, -1 };
            static constexpr int KDY[8] = { +2, +1, -1, -2, -2, -1, +1, +2 };
            auto &out = rawMoveTable[pieceTypeToIndex(PIECE_TYPE::KNIGHT)][fromIdx];
            for (int i = 0; i < 8; ++i) {
                int nf = file + KDX[i];
                int nr = rank + KDY[i];
                if (nf >= 0 && nf < 8 && nr >= 0 && nr < 8) {
                    out.emplace_back(from, POSITION(nr*8 + nf), PIECE_TYPE::KNIGHT);
                }
            }
        }

        // --- BISHOP raw moves (diagonals) ---
        {
            auto &out = rawMoveTable[pieceTypeToIndex(PIECE_TYPE::BISHOP)][fromIdx];
            for (int dx : {+1, -1}) {
                for (int dy : {+1, -1}) {
                    int nf = file + dx;
                    int nr = rank + dy;
                    while (nf >= 0 && nf < 8 && nr >= 0 && nr < 8) {
                        out.emplace_back(from, POSITION(nr*8 + nf), PIECE_TYPE::BISHOP);
                        nf += dx;
                        nr += dy;
                    }
                }
            }
        }

        // --- ROOK raw moves (orthogonals) ---
        {
            auto &out = rawMoveTable[pieceTypeToIndex(PIECE_TYPE::ROOK)][fromIdx];
            for (auto [dx, dy] : { std::pair{+1,0}, std::pair{-1,0},
                                   std::pair{0,+1}, std::pair{0,-1} }) {
                int nf = file + dx;
                int nr = rank + dy;
                while (nf >= 0 && nf < 8 && nr >= 0 && nr < 8) {
                    out.emplace_back(from, POSITION(nr*8 + nf), PIECE_TYPE::ROOK);
                    nf += dx;
                    nr += dy;
                }
            }
        }

        // --- QUEEN raw moves (bishop + rook) ---
        {
            auto &out = rawMoveTable[pieceTypeToIndex(PIECE_TYPE::QUEEN)][fromIdx];
            // diagonals
            for (int dx : {+1, -1}) {
                for (int dy : {+1, -1}) {
                    int nf = file + dx;
                    int nr = rank + dy;
                    while (nf >= 0 && nf < 8 && nr >= 0 && nr < 8) {
                        out.emplace_back(from, POSITION(nr*8 + nf), PIECE_TYPE::QUEEN);
                        nf += dx;
                        nr += dy;
                    }
                }
            }
            // orthogonals
            for (auto [dx, dy] : { std::pair{+1,0}, std::pair{-1,0},
                                   std::pair{0,+1}, std::pair{0,-1} }) {
                int nf = file + dx;
                int nr = rank + dy;
                while (nf >= 0 && nf < 8 && nr >= 0 && nr < 8) {
                    out.emplace_back(from, POSITION(nr*8 + nf), PIECE_TYPE::QUEEN);
                    nf += dx;
                    nr += dy;
                }
            }
        }

        // --- KING raw moves (one‐step + castling) ---
        {
            static constexpr int KINX[8] = { +1, +1,  0, -1, -1, -1,  0, +1 };
            static constexpr int KINY[8] = {  0, +1, +1, +1,  0, -1, -1, -1 };
            auto &out = rawMoveTable[pieceTypeToIndex(PIECE_TYPE::KING)][fromIdx];

            // one‐step
            for (int i = 0; i < 8; ++i) {
                int nf = file + KINX[i];
                int nr = rank + KINY[i];
                if (nf >= 0 && nf < 8 && nr >= 0 && nr < 8) {
                    out.emplace_back(from, POSITION(nr*8 + nf), PIECE_TYPE::KING);
                }
            }

            // raw castling (no legality checks here)
            // white king on e1 (4) → g1 (6) and c1 (2)
            if (fromIdx == 4) {
                out.emplace_back(from, POSITION(6), PIECE_TYPE::KING);
                out.emplace_back(from, POSITION(2), PIECE_TYPE::KING);
            }
            // black king on e8 (60) → g8 (62) and c8 (58)
            if (fromIdx == 60) {
                out.emplace_back(from, POSITION(62), PIECE_TYPE::KING);
                out.emplace_back(from, POSITION(58), PIECE_TYPE::KING);
            }
        }
    }
}

bool PIECE::generalRulesAllow(const MOVE& m, const CHESS& state) const {
    int fromIdx = m.from.index;
    int toIdx   = m.to.index;

    // 1) Allied‐occupancy check
    const auto &destPtr = state.board[toIdx];
    if (destPtr && destPtr->color == this->color) {
        // Can't land on your own piece
        return false;
    }

    // 2) Pin‐check: if moving this piece (not the king) uncovers a sliding attack on our king, disallow.
    // Determine our king's position:
    const POSITION &kingPos = (this->color == COLOR::WHITE
                               ? state.wKingPosition
                               : state.bKingPosition);
    int kingIdx = kingPos.index;

    // If this piece is the king itself, skip pin‐check here.
    if (fromIdx == kingIdx) {
        return true;
    }

    // 2a) Check alignment: same file, same rank, same “\” diagonal, or same “/” anti‐diagonal?
    bool aligned = false;
    int step = 0;
    enum Alignment { FILE, RANK, DIAG, ANTIDIAG } alignType = FILE;

    // same file?
    if (m.from.file == kingPos.file) {
        aligned = true;
        alignType = FILE;
        // file vertical: index difference by multiples of 8.
        // Choose step so scanning goes away from king beyond 'from'.
        step = (fromIdx < kingIdx ? -8 : +8);
    }
    // same rank?
    else if (m.from.rank == kingPos.rank) {
        aligned = true;
        alignType = RANK;
        // scanning left/right: ±1
        step = (fromIdx < kingIdx ? -1 : +1);
    }
    // same "\" diagonal?
    else if (m.from.diagonal == kingPos.diagonal) {
        aligned = true;
        alignType = DIAG;
        // "\" diagonal stepping ±9
        step = (fromIdx < kingIdx ? -9 : +9);
    }
    // same "/" anti‐diagonal?
    else if (m.from.antiDiagonal == kingPos.antiDiagonal) {
        aligned = true;
        alignType = ANTIDIAG;
        // "/" diagonal stepping ±7
        step = (fromIdx < kingIdx ? -7 : +7);
    }

    if (!aligned) {
        // Not aligned with king → cannot uncover sliding attack along that line
        return true;
    }

    // 2b) Scan from the square beyond fromIdx along 'step'
    int scanIdx = fromIdx + step;
    while (scanIdx >= 0 && scanIdx < 64) {
        const auto &pPtr = state.board[scanIdx];
        if (pPtr) {
            // First piece encountered along that ray
            if (pPtr->color == this->color) {
                // Allied piece blocks safely
                break;
            }
            // Enemy piece: is it a sliding attacker along this line?
            PIECE_TYPE t = pPtr->type;
            bool isAttacker = false;
            if (alignType == FILE || alignType == RANK) {
                // straight line: rook or queen attack
                if (t == PIECE_TYPE::ROOK || t == PIECE_TYPE::QUEEN) {
                    isAttacker = true;
                }
            } else {
                // diagonal: bishop or queen attack
                if (t == PIECE_TYPE::BISHOP || t == PIECE_TYPE::QUEEN) {
                    isAttacker = true;
                }
            }
            if (isAttacker) {
                // Moving 'from' piece would expose king to this sliding attack: illegal
                return false;
            }
            // Otherwise, some other enemy piece blocks but is not attacker → safe
            break;
        }
        // empty square: continue scanning
        scanIdx += step;
    }

    return true;
}

bool PIECE::discoveredAttack(const MOVE& m, const CHESS& state) const {
    // This checks if moving this piece from m.from to m.to uncovers
    // an allied sliding piece attacking the enemy king.
    // 1) Identify enemy king position:
    const POSITION &enemyKingPos = (this->color == COLOR::WHITE
                                    ? state.bKingPosition
                                    : state.wKingPosition);
    int kingIdx = enemyKingPos.index;

    // 2) Starting square:
    int fromIdx = m.from.index;
    const POSITION &fromPos = m.from;

    // 3) Quick check: is fromPos aligned with enemy king?
    bool sameFile = (fromPos.file == enemyKingPos.file);
    bool sameRank = (fromPos.rank == enemyKingPos.rank);
    bool sameDiag = (fromPos.diagonal == enemyKingPos.diagonal);
    bool sameAnti = (fromPos.antiDiagonal == enemyKingPos.antiDiagonal);
    if (!(sameFile || sameRank || sameDiag || sameAnti)) {
        // Not aligned → cannot be a discovered attack on the king along a sliding line
        return false;
    }

    // 4) Determine scan direction (step) from fromIdx away from the king, to look for allied slider:
    int step = 0;
    if (sameFile) {
        // Compare ranks: POSITION.rank is 1..8
        if (fromPos.rank < enemyKingPos.rank) {
            // from is "below" king; slider must be further below: decreasing rank → index step -8
            step = -8;
        } else {
            // from above king; slider must be further above: increasing rank → +8
            step = +8;
        }
    }
    else if (sameRank) {
        // Compare files: file char 'a'..'h'
        if (fromPos.file < enemyKingPos.file) {
            // from is to left of king; slider must be further left: file decreasing → index -1
            step = -1;
        } else {
            // from to right of king; slider further right: +1
            step = +1;
        }
    }
    else if (sameDiag) {
        // "\" diagonal: index difference ±9
        // For "\" diag, moving NE increases index by +9, moving SW decreases by -9.
        // If from is "southwest" of king (i.e., fromIdx < kingIdx in diagonal sense?), better compare ranks:
        if (fromPos.rank < enemyKingPos.rank) {
            // from has smaller rank, king higher: from is SW of king; slider must be further SW: decreasing rank, decreasing file → step = -9
            step = -9;
        } else {
            // from is NE of king; slider further NE: step = +9
            step = +9;
        }
    }
    else { // sameAnti
        // "/" anti-diagonal: index difference ±7
        // For "/" diag, moving NW increases by +7, moving SE decreases by -7? Actually depends on indexing:
        // Check: index = rank*8 + file. For "/" diag: if from.rank < king.rank, from is SE of king? Actually easier: compare rank:
        if (fromPos.rank < enemyKingPos.rank) {
            // from rank smaller than king: from is below king; on "/" diag that means from is SE of king; slider must be further SE: step = -7
            step = -7;
        } else {
            // from is NW of king; slider further NW: step = +7
            step = +7;
        }
    }

    // 5) Scan from the square next to fromIdx along 'step', looking for an allied sliding piece
    int scanIdx = fromIdx + step;
    while (scanIdx >= 0 && scanIdx < 64) {
        const auto &pPtr = state.board[scanIdx];
        if (pPtr) {
            // Found first piece along that ray beyond fromIdx
            if (pPtr->color == this->color) {
                // Allied piece: could be sliding attacker
                PIECE_TYPE pt = pPtr->type;
                bool isSlider = false;
                // Determine if this ray is straight (file or rank) or diagonal:
                if (sameFile || sameRank) {
                    // straight line: rook or queen can attack
                    if (pt == PIECE_TYPE::ROOK || pt == PIECE_TYPE::QUEEN) {
                        isSlider = true;
                    }
                } else {
                    // diagonal: bishop or queen
                    if (pt == PIECE_TYPE::BISHOP || pt == PIECE_TYPE::QUEEN) {
                        isSlider = true;
                    }
                }
                if (isSlider) {
                    // check that between fromIdx and kingIdx there is no piece except the moving one.
                    int betweenIdx = fromIdx;
                    int stepToKing = 0;
                    // Determine step from fromIdx toward kingIdx (the opposite direction):
                    if (sameFile) {
                        stepToKing = (fromPos.rank < enemyKingPos.rank ? +8 : -8);
                    } else if (sameRank) {
                        stepToKing = (fromPos.file < enemyKingPos.file ? +1 : -1);
                    } else if (sameDiag) {
                        stepToKing = (fromPos.rank < enemyKingPos.rank ? +9 : -9);
                    } else { // sameAnti
                        stepToKing = (fromPos.rank < enemyKingPos.rank ? +7 : -7);
                    }
                    // Scan from fromIdx+stepToKing up to but not including kingIdx:
                    betweenIdx = fromIdx + stepToKing;
                    bool pathClear = true;
                    while (betweenIdx != kingIdx) {
                        // If there's any piece between fromIdx and kingIdx (other than at fromIdx which is the moving piece),
                        // then even after moving, that piece would block the sliding attack.
                        if (state.board[betweenIdx]) {
                            pathClear = false;
                            break;
                        }
                        betweenIdx += stepToKing;
                    }
                    if (!pathClear) {
                        // blocked between from and king → no discovered attack
                        return false;
                    }
                    // If pathClear, then moving piece uncovers slider attack
                    return true;
                }
            }
            // Either allied non-slider or enemy piece blocks the ray → no discovered attack
            break;
        }
        // Empty square: continue scanning
        scanIdx += step;
    }
    return false;
}

