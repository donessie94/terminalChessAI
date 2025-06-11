#pragma once
#include "../utils/moveHelpers.h"

class PIECE {
public:
    PIECE_TYPE  type;     // PAWN, ROOK, KNIGHT, ...
    COLOR       color;    // WHITE or BLACK
    POSITION    position; // 0..63 on the board

    // Dictionary for moves (empty board)
    inline static const std::unordered_map<PIECE_TYPE,
                        std::unordered_map<DIRECTION, std::vector<MOVE>>
    > movesDictionary = buildMovesDictionary();

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
    virtual std::vector<MOVE> getUpMoves(const POSITION& from) const = 0;
    virtual std::vector<MOVE> getDownMoves(const POSITION& from) const = 0;
    virtual std::vector<MOVE> getLeftMoves(const POSITION& from) const = 0;
    virtual std::vector<MOVE> getRightMoves(const POSITION& from) const = 0;
    virtual std::vector<MOVE> getUpLeftMoves(const POSITION& from) const = 0;
    virtual std::vector<MOVE> getUpRightMoves(const POSITION& from) const = 0;
    virtual std::vector<MOVE> getDownLeftMoves(const POSITION& from) const = 0;
    virtual std::vector<MOVE> getDownRightMoves(const POSITION& from) const = 0;
};