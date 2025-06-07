#pragma once
#include "moveHelpers.h"

// Helper that builds the full dictionary of moves
// for each piece type and direction.
static std::unordered_map<
    PIECE_TYPE,
    std::unordered_map<DIRECTION, std::vector<MOVE>>
> buildMovesDictionary()
{
    std::unordered_map<
      PIECE_TYPE,
      std::unordered_map<DIRECTION, std::vector<MOVE>>
    > dict;

    for (int idx = 0; idx < 64; ++idx) {
        POSITION from(idx);

        for (auto pt : allPieceTypes()) {
            auto &dirMap = dict[pt]; //reference to the direction map for this piece type
            for (auto d : allDirections(pt)) {
                //
                auto moves = computeMovesFor(pt, d, from);
                dirMap[d].insert(dirMap[d].end(),
                                 moves.begin(), moves.end());
            }
        }
    }
    return dict;
}

// Unified dispatcher:
std::vector<MOVE> computeMovesFor(
    PIECE_TYPE pt,
    DIRECTION   d,
    const POSITION &from
) {
    switch (pt) {
        case PIECE_TYPE::KNIGHT:
            switch (d) {
                case DIRECTION::UP_LEFT:        return computeKnightUP_LEFT(from);
                case DIRECTION::UP_RIGHT:       return computeKnightUP_RIGHT(from);
                case DIRECTION::DOWN_LEFT:      return computeKnightDOWN_LEFT(from);
                case DIRECTION::DOWN_RIGHT:     return computeKnightDOWN_RIGHT(from);
                default:                        return {};
            }

        case PIECE_TYPE::ROOK:
            switch (d) {
                case DIRECTION::UP:             return computeRookUP(from);
                case DIRECTION::DOWN:           return computeRookDOWN(from);
                case DIRECTION::LEFT:           return computeRookLEFT(from);
                case DIRECTION::RIGHT:          return computeRookRIGHT(from);
                default:                        return {};
            }

        case PIECE_TYPE::BISHOP:
            switch (d) {
                case DIRECTION::UP_LEFT:        return computeBishopUP_LEFT(from);
                case DIRECTION::UP_RIGHT:       return computeBishopUP_RIGHT(from);
                case DIRECTION::DOWN_LEFT:      return computeBishopDOWN_LEFT(from);
                case DIRECTION::DOWN_RIGHT:     return computeBishopDOWN_RIGHT(from);
                default:                        return {};
            }

        case PIECE_TYPE::QUEEN:
            // queen is rook + bishop
            switch (d) {
                case DIRECTION::UP:             return computeRookUP(from);
                case DIRECTION::DOWN:           return computeRookDOWN(from);
                case DIRECTION::LEFT:           return computeRookLEFT(from);
                case DIRECTION::RIGHT:          return computeRookRIGHT(from);
                case DIRECTION::UP_LEFT:        return computeBishopUP_LEFT(from);
                case DIRECTION::UP_RIGHT:       return computeBishopUP_RIGHT(from);
                case DIRECTION::DOWN_LEFT:      return computeBishopDOWN_LEFT(from);
                case DIRECTION::DOWN_RIGHT:     return computeBishopDOWN_RIGHT(from);
                default:                        return {};
            }

        case PIECE_TYPE::KING:
            // king is one‐step in all directions
            switch (d) {
                case DIRECTION::UP:             return computeKingUP(from);
                case DIRECTION::DOWN:           return computeKingDOWN(from);
                case DIRECTION::LEFT:           return computeKingLEFT(from);
                case DIRECTION::RIGHT:          return computeKingRIGHT(from);
                case DIRECTION::UP_LEFT:        return computeKingUP_LEFT(from);
                case DIRECTION::UP_RIGHT:       return computeKingUP_RIGHT(from);
                case DIRECTION::DOWN_LEFT:      return computeKingDOWN_LEFT(from);
                case DIRECTION::DOWN_RIGHT:     return computeKingDOWN_RIGHT(from);
                default:                        return {};
            }

        case PIECE_TYPE::PAWN:
            // pawns have asymmetric moves
            switch (d) {
                case DIRECTION::UP:             return computePawnUP(from);
                case DIRECTION::UP_LEFT:        return computePawnUP_LEFT(from);
                case DIRECTION::UP_RIGHT:       return computePawnUP_RIGHT(from);
                case DIRECTION::DOWN:           return computePawnDOWN(from);
                case DIRECTION::DOWN_LEFT:      return computePawnDOWN_LEFT(from);
                case DIRECTION::DOWN_RIGHT:     return computePawnDOWN_RIGHT(from);
                default:                        return {};
            }

        default:
            return {};
    }
}

//-----------------------------------------------------------------------------
// Raw move computation functions for each piece type and direction
std::vector<MOVE> computeKnightUP_LEFT(const POSITION &from) {
    std::vector<MOVE> moves;

    // extract 0..7 file and rank from the flat index
    int fileIdx = from.index % 8;
    int rankIdx = from.index / 8;

    // knight offset for UP_LEFT
    constexpr int df = -1;  // one file left
    constexpr int dr = +2;  // two ranks up

    int nf = fileIdx + df;
    int nr = rankIdx + dr;

    // out of bounds check
    if (nf >= 0 && nf < 8 && nr >= 0 && nr < 8) {
        int toIdx = nr * 8 + nf;
        POSITION toPos(toIdx);

        // build the raw move; other flags stay at their default values until set at move validation
        moves.emplace_back(
            DIRECTION::UP_LEFT,
            from,
            toPos,
            PIECE_TYPE::KNIGHT
        );
    }

    return moves;
}

std::vector<MOVE> computeKnightUP_RIGHT(const POSITION &from) {
    std::vector<MOVE> moves;

    // extract 0..7 file and rank from the flat index
    int fileIdx = from.index % 8;
    int rankIdx = from.index / 8;

    // knight offset for UP_RIGHT
    constexpr int df = +1;  // one file right
    constexpr int dr = +2;  // two ranks up

    int nf = fileIdx + df;
    int nr = rankIdx + dr;

    // only add if still on board
    if (nf >= 0 && nf < 8 && nr >= 0 && nr < 8) {
        int toIdx = nr * 8 + nf;
        POSITION toPos(toIdx);

        // build the raw move; other flags use their defaults
        moves.emplace_back(
            DIRECTION::UP_RIGHT,
            from,
            toPos,
            PIECE_TYPE::KNIGHT
        );
    }

    return moves;
}

std::vector<MOVE> computeKnightDOWN_LEFT(const POSITION &from) {
    std::vector<MOVE> moves;

    // extract 0..7 file and rank from the flat index
    int fileIdx = from.index % 8;
    int rankIdx = from.index / 8;

    // knight offset for DOWN_LEFT
    constexpr int df = -1;  // one file left
    constexpr int dr = -2;  // two ranks down

    int nf = fileIdx + df;
    int nr = rankIdx + dr;

    // only add if still on board
    if (nf >= 0 && nf < 8 && nr >= 0 && nr < 8) {
        int toIdx = nr * 8 + nf;
        POSITION toPos(toIdx);

        // build the raw move; other flags use their defaults
        moves.emplace_back(
            DIRECTION::DOWN_LEFT,
            from,
            toPos,
            PIECE_TYPE::KNIGHT
        );
    }

    return moves;
}

std::vector<MOVE> computeKnightDOWN_RIGHT(const POSITION &from) {
    std::vector<MOVE> moves;

    // extract 0..7 file and rank from the flat index
    int fileIdx = from.index % 8;
    int rankIdx = from.index / 8;

    // knight offset for DOWN_RIGHT
    constexpr int df = +1;  // one file right
    constexpr int dr = -2;  // two ranks down

    int nf = fileIdx + df;
    int nr = rankIdx + dr;

    // only add if still on board
    if (nf >= 0 && nf < 8 && nr >= 0 && nr < 8) {
        int toIdx = nr * 8 + nf;
        POSITION toPos(toIdx);

        // build the raw move; other flags use their defaults
        moves.emplace_back(
            DIRECTION::DOWN_RIGHT,
            from,
            toPos,
            PIECE_TYPE::KNIGHT
        );
    }

    return moves;
}

std::vector<MOVE> computeRookUP(const POSITION &from) {
    std::vector<MOVE> moves;
    int fileIdx = from.index % 8;
    int rankIdx = from.index / 8;

    // keep moving one rank up until off-board
    for (int nr = rankIdx + 1; nr < 8; ++nr) {
        int toIdx = nr * 8 + fileIdx;
        POSITION toPos(toIdx);
        moves.emplace_back(
            DIRECTION::UP,
            from,
            toPos,
            PIECE_TYPE::ROOK
        );
    }
    return moves;
}

std::vector<MOVE> computeRookDOWN(const POSITION &from) {
    std::vector<MOVE> moves;
    int fileIdx = from.index % 8;
    int rankIdx = from.index / 8;

    for (int nr = rankIdx - 1; nr >= 0; --nr) {
        int toIdx = nr * 8 + fileIdx;
        POSITION toPos(toIdx);
        moves.emplace_back(
            DIRECTION::DOWN,
            from,
            toPos,
            PIECE_TYPE::ROOK
        );
    }
    return moves;
}

std::vector<MOVE> computeRookLEFT(const POSITION &from) {
    std::vector<MOVE> moves;
    int fileIdx = from.index % 8;
    int rankIdx = from.index / 8;

    for (int nf = fileIdx - 1; nf >= 0; --nf) {
        int toIdx = rankIdx * 8 + nf;
        POSITION toPos(toIdx);
        moves.emplace_back(
            DIRECTION::LEFT,
            from,
            toPos,
            PIECE_TYPE::ROOK
        );
    }
    return moves;
}

std::vector<MOVE> computeRookRIGHT(const POSITION &from) {
    std::vector<MOVE> moves;
    int fileIdx = from.index % 8;
    int rankIdx = from.index / 8;

    for (int nf = fileIdx + 1; nf < 8; ++nf) {
        int toIdx = rankIdx * 8 + nf;
        POSITION toPos(toIdx);
        moves.emplace_back(
            DIRECTION::RIGHT,
            from,
            toPos,
            PIECE_TYPE::ROOK
        );
    }
    return moves;
}

std::vector<MOVE> computeBishopUP_LEFT(const POSITION &from) {
    std::vector<MOVE> moves;
    int fileIdx = from.index % 8;
    int rankIdx = from.index / 8;

    int nf = fileIdx - 1;
    int nr = rankIdx + 1;
    // keep sliding while on board
    while (nf >= 0 && nr < 8) {
        int toIdx = nr * 8 + nf;
        POSITION toPos(toIdx);
        moves.emplace_back(
            DIRECTION::UP_LEFT,
            from,
            toPos,
            PIECE_TYPE::BISHOP
        );
        nf--; nr++;
    }
    return moves;
}

std::vector<MOVE> computeBishopUP_RIGHT(const POSITION &from) {
    std::vector<MOVE> moves;
    int fileIdx = from.index % 8;
    int rankIdx = from.index / 8;

    int nf = fileIdx + 1;
    int nr = rankIdx + 1;
    while (nf < 8 && nr < 8) {
        int toIdx = nr * 8 + nf;
        POSITION toPos(toIdx);
        moves.emplace_back(
            DIRECTION::UP_RIGHT,
            from,
            toPos,
            PIECE_TYPE::BISHOP
        );
        nf++; nr++;
    }
    return moves;
}

std::vector<MOVE> computeBishopDOWN_LEFT(const POSITION &from) {
    std::vector<MOVE> moves;
    int fileIdx = from.index % 8;
    int rankIdx = from.index / 8;

    int nf = fileIdx - 1;
    int nr = rankIdx - 1;
    while (nf >= 0 && nr >= 0) {
        int toIdx = nr * 8 + nf;
        POSITION toPos(toIdx);
        moves.emplace_back(
            DIRECTION::DOWN_LEFT,
            from,
            toPos,
            PIECE_TYPE::BISHOP
        );
        nf--; nr--;
    }
    return moves;
}

std::vector<MOVE> computeBishopDOWN_RIGHT(const POSITION &from) {
    std::vector<MOVE> moves;
    int fileIdx = from.index % 8;
    int rankIdx = from.index / 8;

    int nf = fileIdx + 1;
    int nr = rankIdx - 1;
    while (nf < 8 && nr >= 0) {
        int toIdx = nr * 8 + nf;
        POSITION toPos(toIdx);
        moves.emplace_back(
            DIRECTION::DOWN_RIGHT,
            from,
            toPos,
            PIECE_TYPE::BISHOP
        );
        nf++; nr--;
    }
    return moves;
}

std::vector<MOVE> computeKingUP(const POSITION &from) {
    std::vector<MOVE> moves;
    int file = from.index % 8, rank = from.index / 8;
    int nr = rank + 1;
    if (nr < 8) {
        int toIdx = nr*8 + file;
        moves.emplace_back(DIRECTION::UP, from, POSITION(toIdx), PIECE_TYPE::KING);
    }
    return moves;
}

std::vector<MOVE> computeKingDOWN(const POSITION &from) {
    std::vector<MOVE> moves;
    int file = from.index % 8, rank = from.index / 8;
    int nr = rank - 1;
    if (nr >= 0) {
        int toIdx = nr*8 + file;
        moves.emplace_back(DIRECTION::DOWN, from, POSITION(toIdx), PIECE_TYPE::KING);
    }
    return moves;
}

std::vector<MOVE> computeKingLEFT(const POSITION &from) {
    std::vector<MOVE> moves;
    int file = from.index % 8, rank = from.index / 8;
    int nf = file - 1;
    if (nf >= 0) {
        int toIdx = rank*8 + nf;
        moves.emplace_back(DIRECTION::LEFT, from, POSITION(toIdx), PIECE_TYPE::KING);
    }
    // raw queenside castle moves (king home squares: e1=4, e8=60)
    if (from.index == 4 || from.index == 60) {
        // castle to c1 (2) or c8 (58)
        int castleIdx = (from.index == 4 ? 2 : 58);
        moves.emplace_back(DIRECTION::LEFT, from, POSITION(castleIdx), PIECE_TYPE::KING);
    }
    return moves;
}

std::vector<MOVE> computeKingRIGHT(const POSITION &from) {
    std::vector<MOVE> moves;
    int file = from.index % 8, rank = from.index / 8;
    int nf = file + 1;
    if (nf < 8) {
        int toIdx = rank*8 + nf;
        moves.emplace_back(DIRECTION::RIGHT, from, POSITION(toIdx), PIECE_TYPE::KING);
    }
    // raw kingside castle moves (king home squares: e1=4, e8=60)
    if (from.index == 4 || from.index == 60) {
        // castle to g1 (6) or g8 (62)
        int castleIdx = (from.index == 4 ? 6 : 62);
        moves.emplace_back(DIRECTION::RIGHT, from, POSITION(castleIdx), PIECE_TYPE::KING);
    }
    return moves;
}

std::vector<MOVE> computeKingUP_LEFT(const POSITION &from) {
    std::vector<MOVE> moves;
    int file = from.index % 8, rank = from.index / 8;
    int nf = file - 1, nr = rank + 1;
    if (nf >= 0 && nr < 8) {
        int toIdx = nr*8 + nf;
        moves.emplace_back(DIRECTION::UP_LEFT, from, POSITION(toIdx), PIECE_TYPE::KING);
    }
    return moves;
}

std::vector<MOVE> computeKingUP_RIGHT(const POSITION &from) {
    std::vector<MOVE> moves;
    int file = from.index % 8, rank = from.index / 8;
    int nf = file + 1, nr = rank + 1;
    if (nf < 8 && nr < 8) {
        int toIdx = nr*8 + nf;
        moves.emplace_back(DIRECTION::UP_RIGHT, from, POSITION(toIdx), PIECE_TYPE::KING);
    }
    return moves;
}

std::vector<MOVE> computeKingDOWN_LEFT(const POSITION &from) {
    std::vector<MOVE> moves;
    int file = from.index % 8, rank = from.index / 8;
    int nf = file - 1, nr = rank - 1;
    if (nf >= 0 && nr >= 0) {
        int toIdx = nr*8 + nf;
        moves.emplace_back(DIRECTION::DOWN_LEFT, from, POSITION(toIdx), PIECE_TYPE::KING);
    }
    return moves;
}

std::vector<MOVE> computeKingDOWN_RIGHT(const POSITION &from) {
    std::vector<MOVE> moves;
    int file = from.index % 8, rank = from.index / 8;
    int nf = file + 1, nr = rank - 1;
    if (nf < 8 && nr >= 0) {
        int toIdx = nr*8 + nf;
        moves.emplace_back(DIRECTION::DOWN_RIGHT, from, POSITION(toIdx), PIECE_TYPE::KING);
    }
    return moves;
}

std::vector<MOVE> computePawnUP(const POSITION &from) {
    std::vector<MOVE> moves;
    int file = from.index % 8;
    int rank = from.index / 8;

    // 1‐step
    if (rank + 1 < 8) {
        int toIdx = (rank + 1)*8 + file;
        moves.emplace_back(DIRECTION::UP, from, POSITION(toIdx), PIECE_TYPE::PAWN);
    }
    // 2‐step (raw) from rank 1
    if (rank == 1 && rank + 2 < 8) {
        int toIdx = (rank + 2)*8 + file;
        moves.emplace_back(DIRECTION::UP, from, POSITION(toIdx), PIECE_TYPE::PAWN);
    }

    return moves;
}

std::vector<MOVE> computePawnDOWN(const POSITION &from) {
    std::vector<MOVE> moves;
    int file = from.index % 8;
    int rank = from.index / 8;

    // 1‐step
    if (rank - 1 >= 0) {
        int toIdx = (rank - 1)*8 + file;
        moves.emplace_back(DIRECTION::DOWN, from, POSITION(toIdx), PIECE_TYPE::PAWN);
    }
    // 2‐step (raw) from rank 6
    if (rank == 6 && rank - 2 >= 0) {
        int toIdx = (rank - 2)*8 + file;
        moves.emplace_back(DIRECTION::DOWN, from, POSITION(toIdx), PIECE_TYPE::PAWN);
    }

    return moves;
}

std::vector<MOVE> computePawnUP_LEFT(const POSITION &from) {
    std::vector<MOVE> moves;
    int file = from.index % 8;
    int rank = from.index / 8;

    if (file - 1 >= 0 && rank + 1 < 8) {
        int toIdx = (rank + 1)*8 + (file - 1);
        moves.emplace_back(DIRECTION::UP_LEFT, from, POSITION(toIdx), PIECE_TYPE::PAWN);
    }
    return moves;
}

std::vector<MOVE> computePawnUP_RIGHT(const POSITION &from) {
    std::vector<MOVE> moves;
    int file = from.index % 8;
    int rank = from.index / 8;

    if (file + 1 < 8 && rank + 1 < 8) {
        int toIdx = (rank + 1)*8 + (file + 1);
        moves.emplace_back(DIRECTION::UP_RIGHT, from, POSITION(toIdx), PIECE_TYPE::PAWN);
    }
    return moves;
}

std::vector<MOVE> computePawnDOWN_LEFT(const POSITION &from) {
    std::vector<MOVE> moves;
    int file = from.index % 8;
    int rank = from.index / 8;

    if (file - 1 >= 0 && rank - 1 >= 0) {
        int toIdx = (rank - 1)*8 + (file - 1);
        moves.emplace_back(DIRECTION::DOWN_LEFT, from, POSITION(toIdx), PIECE_TYPE::PAWN);
    }
    return moves;
}

std::vector<MOVE> computePawnDOWN_RIGHT(const POSITION &from) {
    std::vector<MOVE> moves;
    int file = from.index % 8;
    int rank = from.index / 8;

    if (file + 1 < 8 && rank - 1 >= 0) {
        int toIdx = (rank - 1)*8 + (file + 1);
        moves.emplace_back(DIRECTION::DOWN_RIGHT, from, POSITION(toIdx), PIECE_TYPE::PAWN);
    }
    return moves;
}