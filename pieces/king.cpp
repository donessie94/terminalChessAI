#include "king.h"
#include "../logic/chess.h"

KING::KING(POSITION pos, COLOR col) : PIECE(PIECE_TYPE::KING, col, pos) {}

bool KING::isKingProtected(const MOVE &m, const CHESS &state) const {
    // here I'm checking that the king moving to m.to would not be in check
    const POSITION &to    = m.to;
    int              toIdx = to.index;
    COLOR            enemy = (this->color == COLOR::WHITE
                              ? COLOR::BLACK
                              : COLOR::WHITE);

    // convert to 0..7 grid coords
    int tr = to.rank - 1;       // 1..8 → 0..7
    int tf = to.file - 'a';     // 'a'..'h' → 0..7

    // 1) Pawn attacks
    {
        int dr = (this->color == COLOR::WHITE ? +1 : -1);
        for (int df : {-1, +1}) {
            int r = tr + dr, f = tf + df;
            if (r >= 0 && r < 8 && f >= 0 && f < 8) {
                int idx = r*8 + f;
                auto &p = state.board[idx];
                if (p && p->color == enemy && p->type == PIECE_TYPE::PAWN) {
                    // here I'm detecting a pawn attack
                    return false;
                }
            }
        }
    }

    // 2) Knight jumps
    {
        static constexpr int KN[8][2] = {
            {+2,+1},{+1,+2},{-1,+2},{-2,+1},
            {-2,-1},{-1,-2},{+1,-2},{+2,-1}
        };
        for (auto &d : KN) {
            int r = tr + d[0], f = tf + d[1];
            if (r >= 0 && r < 8 && f >= 0 && f < 8) {
                int idx = r*8 + f;
                auto &p = state.board[idx];
                if (p && p->color == enemy && p->type == PIECE_TYPE::KNIGHT) {
                    // here I'm detecting a knight attack
                    return false;
                }
            }
        }
    }

    // 3) Sliding attacks (rook/queen on straights, bishop/queen on diagonals)
    struct Ray { int dr, df; };
    static constexpr Ray RAYS[8] = {
        {+1,  0}, {-1,  0}, { 0,+1}, { 0,-1},
        {+1,+1}, {+1,-1}, {-1,+1}, {-1,-1}
    };

    for (auto &ray : RAYS) {
        int r = tr + ray.dr;
        int f = tf + ray.df;
        while (r >= 0 && r < 8 && f >= 0 && f < 8) {
            int idx = r*8 + f;

            // here I'm skipping the king’s original square so it doesn't block detection
            if (idx == m.from.index) {
                r += ray.dr;
                f += ray.df;
                continue;
            }

            auto &p = state.board[idx];
            if (p) {
                if (p->color == enemy) {
                    bool isStraight = (ray.dr == 0 || ray.df == 0);
                    bool attacker   = isStraight
                        ? (p->type == PIECE_TYPE::ROOK  ||
                           p->type == PIECE_TYPE::QUEEN)
                        : (p->type == PIECE_TYPE::BISHOP||
                           p->type == PIECE_TYPE::QUEEN);
                    if (attacker) {
                        // here I'm detecting a sliding attack
                        return false;
                    }
                }
                // here I'm blocked by some other piece
                break;
            }
            r += ray.dr;
            f += ray.df;
        }
    }

    // 4) Adjacent enemy king
    for (int dr = -1; dr <= +1; ++dr) {
        for (int df = -1; df <= +1; ++df) {
            if (dr == 0 && df == 0) continue;
            int r = tr + dr, f = tf + df;
            if (r >= 0 && r < 8 && f >= 0 && f < 8) {
                int idx = r*8 + f;
                auto &p = state.board[idx];
                if (p && p->color == enemy && p->type == PIECE_TYPE::KING) {
                    // here I'm detecting the opposing king is too close
                    return false;
                }
            }
        }
    }

    // here I'm done checking — no attackers found
    return true;
}

void KING::computeValidMoves(const POSITION &from, const CHESS &state) {
    movesCapture.clear();
    movesDevelopment.clear();
    movesQuiet.clear();
    discoveredAttackInfo.clear();

    int pieceIdx    = PIECE::pieceTypeToIndex(type);
    int fromIdx     = from.index;
    const auto &raw = PIECE::rawMoveTable[pieceIdx][fromIdx];

    // Home‐square index for e1/e8
    int homeIdx = (color == COLOR::WHITE ? 4 : 60);

    // Enemy king index (used inside isKingProtected)
    int enemyKingIdx = (color == COLOR::WHITE
                       ? state.bKingPosition.index
                       : state.wKingPosition.index);

    for (const MOVE &m : raw) {
        int toIdx = m.to.index;

        // 1) Can't land on an allied piece
        if (state.board[toIdx] && state.board[toIdx]->color == color) {
            continue;
        }

        // 2) Can't move into check or adjacent to enemy king
        //    (this tests the *destination* square only)
        if (!isKingProtected(m, state)) {
            continue;
        }

        // 3) Basic attack and discovery flags
        bool isCapture = (state.board[toIdx] && state.board[toIdx]->color != color);
        bool discCheck = discoveredAttack(m, state);

        // 4) Castling detection (king moves two files from home)
        bool isDev = false;
        if (fromIdx == homeIdx) {
            int df = m.to.file - from.file;
            if (std::abs(df) == 2) {
                // determine which side we’re castling
                bool kingSide = (df > 0);

                // 4a) do we even have the right to castle this side?
                bool canCastle = false;
                if (color == COLOR::WHITE) {
                    canCastle = kingSide
                                ? state.whiteCanCastleKingSide
                                : state.whiteCanCastleQueenSide;
                } else {
                    canCastle = kingSide
                                ? state.blackCanCastleKingSide
                                : state.blackCanCastleQueenSide;
                }
                if (!canCastle)
                    continue;

                // 4b) figure out rook’s file and our rank
                int fromR    = from.rank - 1;      // 0..7
                int fromF    = from.file - 'a';    // 0..7
                int rookFile = kingSide ? 7 : 0;   // 'h' or 'a'

                // 4c) every square strictly between fromF and rookFile must be empty
                int lo = std::min(fromF, rookFile) + 1;
                int hi = std::max(fromF, rookFile) - 1;
                bool blocked = false;
                for (int f = lo; f <= hi; ++f) {
                    int idx = fromR*8 + f;
                    if (state.board[idx]) {
                        // here I'm seeing a piece between king and rook → no castle
                        blocked = true;
                        break;
                    }
                }
                if (blocked)
                    continue;

                // 4d) finally, ensure the king doesn’t pass through or land in check:
                //      we already know start is safe and end is safe (isKingProtected tested end),
                //      so only test the intermediate square (one step toward rook)
                char midFile = (from.file + m.to.file) / 2;
                int  midIdx  = (from.rank - 1) * 8 + (midFile - 'a');
                MOVE midMove(from, POSITION(midIdx), type);
                if (!isKingProtected(midMove, state))
                    continue;

                // here I'm marking this two‐square king move as castling development
                isDev = true;
            }
        }

        // 5) Categorize into the right vector
        if (discCheck) {
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

void KING::computeValidMovesInCheck(const POSITION& from, const CHESS& state, const std::vector<POSITION>& attackers) {
    movesCapture.clear();
    movesDevelopment.clear();
    movesQuiet.clear();
    discoveredAttackInfo.clear();

    int pieceIdx    = PIECE::pieceTypeToIndex(type);
    int fromIdx     = from.index;
    const auto &raw = PIECE::rawMoveTable[pieceIdx][fromIdx];

    // Home‐square index for e1/e8
    int homeIdx = (color == COLOR::WHITE ? 4 : 60);

    // Enemy king index (used inside isKingProtected)
    int enemyKingIdx = (color == COLOR::WHITE
                       ? state.bKingPosition.index
                       : state.wKingPosition.index);

    for (const MOVE &m : raw) {
        int toIdx = m.to.index;

        // 1) Can't land on an allied piece
        if (state.board[toIdx] && state.board[toIdx]->color == color) {
            continue;
        }

        // 2) Can't move into check or adjacent to enemy king
        //    (this tests the *destination* square only)
        if (!isKingProtected(m, state)) {
            continue;
        }

        // if trying to castle from check continue since this is not valid
        if (fromIdx == homeIdx) {
            int df = m.to.file - from.file;
            if (std::abs(df) == 2)
                continue;
        }

        // 3) Basic attack and discovery flags
        bool isCapture = (state.board[toIdx] && state.board[toIdx]->color != color);
        bool discCheck = discoveredAttack(m, state);

        // 5) Categorize into the right vector
        if (discCheck) {
            movesCheck.push_back(m);
        }
        else if (isCapture) {
            movesCapture.push_back(m);
        }
        else {
            movesQuiet.push_back(m);
        }
    }
}