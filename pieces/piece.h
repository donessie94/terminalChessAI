#pragma once
#include "../utils/miscellanous.h"

// Forward declaration of CHESS:
class CHESS;

class PIECE {
public:
    PIECE_TYPE  type;     // PAWN, ROOK, KNIGHT, ...
    COLOR       color;    // WHITE or BLACK
    POSITION    position; // 0..63 on the board

    std::vector<MOVE> movesCheck;
    std::vector<MOVE> movesCapture;
    std::vector<MOVE> movesDevelopment;
    std::vector<MOVE> movesQuiet;

    std::vector<std::pair<MOVE, POSITION>> directAttackInfo;
    std::vector<std::pair<MOVE, POSITION>> discoveredAttackInfo;

    // 2d table of precomputed raw moves for all pieces
    inline static std::array<std::array<std::vector<MOVE>, 64>, 6> rawMoveTable;
    static int pieceTypeToIndex(PIECE_TYPE pt);
    static void buildRawMoveTable();

    // Constructor for base fields
    PIECE(PIECE_TYPE t, COLOR c, POSITION pos);

    // Virtual destructor (important for base classes)
    virtual ~PIECE() = default;

    // Get piece type as string, e.g. "PAWN", "ROOK"
    std::string getTypeAsString() const;

    // Get piece color as string, e.g. "WHITE", "BLACK"
    std::string getColorAsString() const;

    // Get piece position as string, e.g. "e4"
    std::string getPositionAsString() const;

    // Abstract methods for directional valid moves
    virtual void computeValidMoves(const POSITION& from, const CHESS& state) = 0;
    virtual void computeValidMovesInCheck(const POSITION& from, const CHESS& state, const std::vector<POSITION>& attackers) = 0;

    // General rules: allied‐occupancy + pin‐check.
    // Returns true if the move m (from 'from' to m.to) is allowed by general chess rules:
    //  - Does not land on an allied piece.
    //  - Does not expose own king to sliding attack (pin check).
    // For kings themselves, this helper skips pin logic (king‐specific legality like "not moving into attacked square" must be handled separately).
    bool generalRulesAllow(const MOVE& m, const CHESS& state) const;
    bool discoveredAttack(const MOVE& m, const CHESS& state);

    // Returns true if all squares strictly between 'fromIdx' and 'toIdx' on a rook/bishop/queen move are empty.
    bool isPathClearRook(const MOVE &m, const CHESS &state);
    bool isPathClearBishop(const MOVE &m, const CHESS &state);
    bool isPathClearQueen(const MOVE &m, const CHESS &state);
};