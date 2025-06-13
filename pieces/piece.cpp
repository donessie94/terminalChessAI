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
