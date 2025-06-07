#pragma once
#include "miscellanous.h"

// Helper that builds the full dictionary of moves
// for each piece type and direction.
static std::unordered_map<
    PIECE_TYPE,
    std::unordered_map<DIRECTION, std::vector<MOVE>>
> buildMovesDictionary();

// The unified dispatcher:
std::vector<MOVE> computeMovesFor(
    PIECE_TYPE pt,
    DIRECTION   d,
    const POSITION &from
);

// Helper functions for each piece type and direction for creating the move dictionary.
// Knight moves:
std::vector<MOVE> computeKnightUP_LEFT(const POSITION &from);
std::vector<MOVE> computeKnightUP_RIGHT(const POSITION &from);
std::vector<MOVE> computeKnightDOWN_LEFT(const POSITION &from);
std::vector<MOVE> computeKnightDOWN_RIGHT(const POSITION &from);
// Rook moves:
std::vector<MOVE> computeRookUP(const POSITION &from);
std::vector<MOVE> computeRookDOWN(const POSITION &from);
std::vector<MOVE> computeRookLEFT(const POSITION &from);
std::vector<MOVE> computeRookRIGHT(const POSITION &from);
// Bishop moves:
std::vector<MOVE> computeBishopUP_LEFT(const POSITION &from);
std::vector<MOVE> computeBishopUP_RIGHT(const POSITION &from);
std::vector<MOVE> computeBishopDOWN_LEFT(const POSITION &from);
std::vector<MOVE> computeBishopDOWN_RIGHT(const POSITION &from);
// Queen moves:
std::vector<MOVE> computeKingUP(const POSITION &from);
std::vector<MOVE> computeKingDOWN(const POSITION &from);
std::vector<MOVE> computeKingLEFT(const POSITION &from);
std::vector<MOVE> computeKingRIGHT(const POSITION &from);
std::vector<MOVE> computeKingUP_LEFT(const POSITION &from);
std::vector<MOVE> computeKingUP_RIGHT(const POSITION &from);
std::vector<MOVE> computeKingDOWN_LEFT(const POSITION &from);
std::vector<MOVE> computeKingDOWN_RIGHT(const POSITION &from);
// Pawn moves:
std::vector<MOVE> computePawnUP(const POSITION &from);
std::vector<MOVE> computePawnDOWN(const POSITION &from);
std::vector<MOVE> computePawnUP_LEFT(const POSITION &from);
std::vector<MOVE> computePawnUP_RIGHT(const POSITION &from);
std::vector<MOVE> computePawnDOWN_LEFT(const POSITION &from);
std::vector<MOVE> computePawnDOWN_RIGHT(const POSITION &from);