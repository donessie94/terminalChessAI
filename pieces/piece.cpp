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
    // unpack positions & indices
    const POSITION &from    = m.from;
    const POSITION &to      = m.to;
    const int      fromIdx  = from.index;
    const int      toIdx    = to.index;

    // allied-occupancy at destination
    const auto &destPtr = state.board[toIdx];
    if (destPtr && destPtr->color == this->color) {
        // here I'm disallowing landing on my own piece
        return false;
    }

    // pin-check: locate my king
    const POSITION &kingPos = (this->color == COLOR::WHITE
                               ? state.wKingPosition
                               : state.bKingPosition);
    const int kingIdx = kingPos.index;

    // if I'm the king, skip pin logic
    if (fromIdx == kingIdx) {
        // here I'm letting the king move (self-check is handled elsewhere)
        return true;
    }

    // see if 'from' sits on the same ray as my king
    enum Alignment { FILE, RANK, DIAG, ANTIDIAG };
    bool aligned = false;
    Alignment alignType = FILE;

    // convert positions to 0..7 grid indices
    int fromRankIdx = from.rank - 1;        // 1..8 → 0..7
    int fromFileIdx = from.file - 'a';      // 'a'..'h' → 0..7
    int kingRankIdx = kingPos.rank - 1;
    int kingFileIdx = kingPos.file - 'a';

    // compute step in that grid to move away from the king
    int stepRank = 0, stepFile = 0;

    // file-aligned?
    if (from.file == kingPos.file) {
        if (to.file == kingPos.file) return true;  // still protecting
        aligned   = true;
        alignType = FILE;

        // vertical ray: file constant
        stepFile = 0;
        // if king above from, step down; else step up
        stepRank = (kingRankIdx > fromRankIdx ? -1 : +1);
    }
    // rank-aligned?
    else if (from.rank == kingPos.rank) {
        if (to.rank == kingPos.rank) return true;
        aligned   = true;
        alignType = RANK;

        // horizontal ray: rank constant
        stepRank = 0;
        // if king to right, step left; else step right
        stepFile = (kingFileIdx > fromFileIdx ? -1 : +1);
    }
    // "\" diagonal?
    else if (from.diagonal == kingPos.diagonal) {
        if (to.diagonal == kingPos.diagonal) return true;
        aligned   = true;
        alignType = DIAG;

        // "\" diagonal: both step same direction
        stepRank = (kingRankIdx > fromRankIdx ? -1 : +1);
        stepFile = (kingFileIdx > fromFileIdx ? -1 : +1);
    }
    // "/" anti-diagonal?
    else if (from.antiDiagonal == kingPos.antiDiagonal) {
        if (to.antiDiagonal == kingPos.antiDiagonal) return true;
        aligned   = true;
        alignType = ANTIDIAG;

        // "/" diagonal: rank & file step opposite directions
        stepRank = (kingRankIdx > fromRankIdx ? -1 : +1);
        stepFile = (kingFileIdx > fromFileIdx ? +1 : -1);
    }

    // if I'm not aligned, I can't uncover a slider
    if (!aligned) {
        // here I'm safe—no pin on a non-aligned ray
        return true;
    }

    // scan square-by-square in 0..7 bounds
    int scanRank = fromRankIdx + stepRank;
    int scanFile = fromFileIdx + stepFile;
    while (scanRank >= 0 && scanRank < 8 &&
           scanFile >= 0 && scanFile < 8) {
        // convert back to 0..63
        int scanIdx = scanRank * 8 + scanFile;
        const auto &pPtr = state.board[scanIdx];

        if (pPtr) {
            // here I'm at the first piece blocking the ray
            if (pPtr->color == this->color) {
                // allied piece blocks safely
                SDL_Log("move (%d, %d) available -> allied at %d blocks", m.from.index,m.to.index,scanIdx);
                break;
            }
            // enemy piece: only a true slider pins
            PIECE_TYPE t = pPtr->type;
            bool isAttacker = false;
            if (alignType == FILE || alignType == RANK) {
                // straight‐line slider?
                isAttacker = (t == PIECE_TYPE::ROOK || t == PIECE_TYPE::QUEEN);
            } else {
                // diagonal slider?
                isAttacker = (t == PIECE_TYPE::BISHOP || t == PIECE_TYPE::QUEEN);
            }
            if (isAttacker) {
                // here I'm uncovering a sliding attack on my king → pinned
                return false;
            }
            // otherwise non-slider enemy blocks safely
            break;
        }

        // empty: advance to next square
        scanRank += stepRank;
        scanFile += stepFile;
    }

    // here I'm done scanning with no pin detected → move is allowed
    return true;
}

bool PIECE::discoveredAttack(const MOVE& m, const CHESS& state) {
    // Enemy king position
    const POSITION &enemyKingPos = (this->color == COLOR::WHITE
                                    ? state.bKingPosition
                                    : state.wKingPosition);
    // Source square
    const POSITION &fromPos = m.from;

    // Quick alignment check
    bool sameFile = (fromPos.file         == enemyKingPos.file);
    bool sameRank = (fromPos.rank         == enemyKingPos.rank);
    bool sameDiag = (fromPos.diagonal     == enemyKingPos.diagonal);
    bool sameAnti = (fromPos.antiDiagonal == enemyKingPos.antiDiagonal);
    if (!(sameFile || sameRank || sameDiag || sameAnti)) {
        // here I'm not aligned → no possible discovered attack
        return false;
    }

    // Convert to 0..7 grid indices
    int fromR = fromPos.rank - 1;          // 1..8 → 0..7
    int fromF = fromPos.file - 'a';        // 'a'..'h' → 0..7
    int kingR = enemyKingPos.rank - 1;
    int kingF = enemyKingPos.file - 'a';

    // Determine grid‐step away from the king
    int stepR = 0, stepF = 0;
    if (sameFile) {
        // vertical ray
        stepF = 0;
        // if king above me (larger rankIdx), step down; else up
        stepR = (kingR > fromR ? -1 : +1);
    }
    else if (sameRank) {
        // horizontal ray
        stepR = 0;
        // if king to my right, step left; else right
        stepF = (kingF > fromF ? -1 : +1);
    }
    else if (sameDiag) {
        // "\" diagonal: both same direction
        stepR = (kingR > fromR ? -1 : +1);
        stepF = (kingF > fromF ? -1 : +1);
    }
    else { // sameAnti
        // "/" diagonal: rank & file opposite directions
        stepR = (kingR > fromR ? -1 : +1);
        stepF = (kingF > fromF ? +1 : -1);
    }

    // Scan outward from the square next to 'from'
    int scanR = fromR + stepR;
    int scanF = fromF + stepF;
    while (scanR >= 0 && scanR < 8 && scanF >= 0 && scanF < 8) {
        int scanIdx = scanR * 8 + scanF;
        const auto &pPtr = state.board[scanIdx];

        if (pPtr) {
            // here I'm at the first piece beyond 'from'
            if (pPtr->color == this->color) {
                // allied piece may slide to king
                PIECE_TYPE pt = pPtr->type;
                bool isSlider = false;
                if (sameFile || sameRank) {
                    // straight line: rook or queen
                    isSlider = (pt == PIECE_TYPE::ROOK || pt == PIECE_TYPE::QUEEN);
                } else {
                    // diagonal: bishop or queen
                    isSlider = (pt == PIECE_TYPE::BISHOP || pt == PIECE_TYPE::QUEEN);
                }

                if (isSlider) {
                    // here I'm checking path between 'from' and the king for blockers
                    // compute step from 'from' toward king (opposite of above)
                    int toKingR = (fromR < kingR ? +1 : -1);
                    int toKingF = (fromF < kingF ? +1 : -1);
                    if (sameFile)      { toKingF = 0; }
                    else if (sameRank) { toKingR = 0; }
                    // walk between from and king (exclusive)
                    int r = fromR + toKingR, f = fromF + toKingF;
                    bool clearPath = true;
                    while (!(r == kingR && f == kingF)) {
                        int idx = r * 8 + f;
                        if (state.board[idx]) {
                            // here I'm finding a blocker → no discovered attack
                            clearPath = false;
                            break;
                        }
                        r += toKingR;
                        f += toKingF;
                    }
                    if (clearPath) {
                        // here I'm uncovering an allied slider attack on the king
                        MOVE mv(m.from, m.to, this->type);
                        POSITION sliderPos(scanIdx);
                        discoveredAttackInfo.emplace_back(mv, sliderPos);
                        return true;
                    }
                }
            }
            // allied non-slider or enemy piece blocks this ray → stop scanning
            break;
        }

        // empty square → continue stepping
        scanR += stepR;
        scanF += stepF;
    }

    // here I'm done scanning with no discovered attack
    return false;
}

bool PIECE::isPathClearRook(const MOVE &m, const CHESS &state) {
    const POSITION &from = m.from;
    const POSITION &to   = m.to;

    // Here I'm determining step in index space:
    int step = 0;
    if (from.rank == to.rank) {
        // horizontal move
        if (to.file > from.file) {
            // here I'm moving right
            step = +1;
        } else {
            // here I'm moving left
            step = -1;
        }
    } else {
        // vertical move
        if (to.rank > from.rank) {
            // here I'm moving up (higher rank)
            step = +8;
        } else {
            // here I'm moving down (lower rank)
            step = -8;
        }
    }

    // Here I'm iterating squares strictly between from.index and to.index:
    int idx = from.index + step;
    while (idx != to.index) {
        // Here I'm checking if there's a piece blocking:
        if (state.board[idx]) {
            // here I'm seeing a blocker before reaching destination
            return false;
        }
        idx += step;
    }

    // No blockers found → path is clear.
    return true;
}

bool PIECE::isPathClearBishop(const MOVE &m, const CHESS &state) {
    const POSITION &from = m.from;
    const POSITION &to   = m.to;
    int dr = to.rank - from.rank;
    int df = to.file - from.file;

    // Here I'm determining step: ±9 or ±7
    int step = 0;
    if (dr > 0) {
        // moving up
        if (df > 0) {
            // here I'm moving up-right
            step = +9;
        } else {
            // here I'm moving up-left
            step = +7;
        }
    } else {
        // moving down
        if (df > 0) {
            // here I'm moving down-right
            step = -7;
        } else {
            // here I'm moving down-left
            step = -9;
        }
    }
    int idx = from.index + step;
    while (idx != to.index) {
        if (state.board[idx]) {
            // here I'm seeing a blocker in bishop path
            return false;
        }
        idx += step;
    }
    // No blockers → path is clear
    return true;
}

bool PIECE::isPathClearQueen(const MOVE &m, const CHESS &state) {
    const POSITION &from = m.from;
    const POSITION &to   = m.to;
    // Here I'm checking if it's straight or diagonal:
    if (from.rank == to.rank || from.file == to.file) {
        // here I'm delegating to rook logic
        return isPathClearRook(m, state);
    }
    int dr = to.rank - from.rank;
    int df = to.file - from.file;
    if (std::abs(dr) == std::abs(df)) {
        // here I'm delegating to bishop logic
        return isPathClearBishop(m, state);
    }
    // not a sliding move
    return false;
}