#pragma once
#include "piece.h"
#include <vector>

// Return {PAWN, ROOK, …, KING}
std::vector<PIECE_TYPE> allPieceTypes(){
    return {
        PIECE_TYPE::PAWN,
        PIECE_TYPE::ROOK,
        PIECE_TYPE::KNIGHT,
        PIECE_TYPE::BISHOP,
        PIECE_TYPE::QUEEN,
        PIECE_TYPE::KING,
    };
}

std::vector<DIRECTION> allDirections(PIECE_TYPE pt) {
    switch (pt) {
        case PIECE_TYPE::PAWN:
            return {DIRECTION::UP, DIRECTION::UP_LEFT, DIRECTION::UP_RIGHT,
                    DIRECTION::DOWN, DIRECTION::DOWN_LEFT, DIRECTION::DOWN_RIGHT};
        case PIECE_TYPE::ROOK:
            return {DIRECTION::UP, DIRECTION::DOWN, DIRECTION::LEFT, DIRECTION::RIGHT};
        case PIECE_TYPE::KNIGHT:
            return {DIRECTION::UP_LEFT, DIRECTION::UP_RIGHT, DIRECTION::DOWN_LEFT, DIRECTION::DOWN_RIGHT};
        case PIECE_TYPE::BISHOP:
            return {DIRECTION::UP_LEFT, DIRECTION::UP_RIGHT, DIRECTION::DOWN_LEFT, DIRECTION::DOWN_RIGHT};
        case PIECE_TYPE::QUEEN:
            return {DIRECTION::UP, DIRECTION::DOWN, DIRECTION::LEFT, DIRECTION::RIGHT,
                    DIRECTION::UP_LEFT, DIRECTION::UP_RIGHT, DIRECTION::DOWN_LEFT, DIRECTION::DOWN_RIGHT};
        case PIECE_TYPE::KING:
            return {DIRECTION::UP, DIRECTION::DOWN, DIRECTION::LEFT, DIRECTION::RIGHT,
                    DIRECTION::UP_LEFT, DIRECTION::UP_RIGHT, DIRECTION::DOWN_LEFT, DIRECTION::DOWN_RIGHT};
        default:
            return {};
    }
}

// Small helper to compute moves for a piece type in a given direction
std::vector<MOVE> computeMovesFor(
    PIECE_TYPE pt,
    DIRECTION   d,
    const POSITION &from
);

// 1. Top-level build function
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
                // Call your small helper:
                auto moves = computeMovesFor(pt, d, from);
                dirMap[d].insert(dirMap[d].end(),
                                 moves.begin(), moves.end());
            }
        }
    }
    return dict;
}