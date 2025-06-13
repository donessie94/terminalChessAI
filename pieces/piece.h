#pragma once
#include "../utils/miscellanous.h"

// Forward declaration of CHESS:
class CHESS;

class PIECE {
public:
    PIECE_TYPE  type;     // PAWN, ROOK, KNIGHT, ...
    COLOR       color;    // WHITE or BLACK
    POSITION    position; // 0..63 on the board

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
    virtual std::vector<MOVE> getValidMoves(const POSITION& from, const CHESS& state) const = 0;
};