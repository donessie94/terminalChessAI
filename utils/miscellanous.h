#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <utility>   // for std::pair
#include <stdexcept>

// Color enum for white/black
enum class COLOR { WHITE, BLACK };

// Piece type enum for different chess pieces (if you need it later)
enum class PIECE_TYPE { PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING, EMPTY };

// Direction enum for move directions
enum class DIRECTION { UP, DOWN, LEFT, RIGHT, UP_LEFT, UP_RIGHT, DOWN_LEFT, DOWN_RIGHT };

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

// ----------------------------------------------------------------
// Helper: build a map from [0..63] → (rank, fileLetter) in “a1→0, b1→1, …, h8→63” order
static std::unordered_map<int, std::pair<int,char>> buildPosToNotation() {
    std::unordered_map<int, std::pair<int,char>> dict;
    dict.reserve(64);

    // We will have a mapping like this:
    //   index  0 → (rank=1, file='a')   → "a1"
    //   index  1 → (rank=1, file='b')   → "b1"
    //   ...
    //   index  7 → (rank=1, file='h')   → "h1"
    //   index  8 → (rank=2, file='a')   → "a2"
    //   ...
    //   index 63 → (rank=8, file='h')   → "h8"

    for (int i = 0; i < 64; ++i) {
        int rank = 1 + (i / 8);             // i=0..7 → rank=1; i=8..15 → rank=2; …; i=56..63 → rank=8
        char file = static_cast<char>('a' + (i % 8));
        dict.emplace(i, std::make_pair(rank, file));
    }
    return dict;
}

// ----------------------------------------------------------------
class POSITION {
public:
    int index;  // 0..63, where 0→a1, 1→b1, …, 63→h8
    char file; // 'a' to 'h'
    int rank;  // 1 to 8
    int diagonal; // 1 to 15 up-right direction (1→a8, 2→a7, 3→a6, ..., 8→a1, 9→b1, 10→c1, ..., 15→h1)
    int antiDiagonal; // 1 to 15 down-right direction (1→a1, 2→a2, 3→a3, ..., 8→a8, 9→b8, 10→c8, ..., 15→h8)

    // Build once (C++17 and later)
    inline static const std::unordered_map<int, std::pair<int,char>> posToNotation
        = buildPosToNotation();

    // Setter that validates range, then stores the new index
    void setPosition(int newPos) {
        if (newPos < 0 || newPos > 63) {
            throw std::out_of_range("POSITION index must be between 0 and 63");
        }
        index = newPos;
        // Lookup file/rank
        auto [r, f] = posToNotation.at(index);
        rank = r;
        file = f;

        // Convert file char → 0..7
        int fileIdx = file - 'a';      // 'a'→0, 'b'→1, … 'h'→7

        // Diagonal (NW→SE): a8=1, a7=2…a1=8, b1=9…h1=15
        diagonal = fileIdx + (8 - rank) + 1;

        // Anti-diagonal (NE→SW): a1=1…a8=8, b8=9…h8=15
        antiDiagonal = fileIdx + rank;
    }

    // Constructor to initialize position
    POSITION(int pos) {
        setPosition(pos);
    }

    //copy constructor
    POSITION(const POSITION& other) = default;

    std::string toAlgebraicNotation() const {
        return std::string{file} + std::to_string(rank);
    }
};

class MOVE {
public:
    DIRECTION direction; // Direction of the move (UP, DOWN, etc.)
    POSITION from; // Starting position of the move
    POSITION to; // Ending position of the move
    PIECE_TYPE pieceType; // Type of the piece being moved

    COLOR pieceColor; // Color of the piece being moved
    bool isCapture; // Whether the move is a capture
    PIECE_TYPE capturedPieceType; // Type of the piece being captured, if any
    bool isPromotion; // Whether the move is a promotion
    PIECE_TYPE promotionType; // Type of piece to promote to, if applicable
    bool isCheck; // Whether the move puts the opponent in check
    bool isCheckmate; // Whether the move results in checkmate

    // Constructor to initialize a move
    MOVE(DIRECTION dir, POSITION fromPos, POSITION toPos, PIECE_TYPE pt,
        COLOR color = COLOR::WHITE, bool capture = false, PIECE_TYPE capturedType = PIECE_TYPE::EMPTY,
        bool promotion = false, PIECE_TYPE promoType = PIECE_TYPE::EMPTY,
        bool check = false, bool checkmate = false)
        : direction(dir), from(fromPos), to(toPos), pieceType(pt),
          pieceColor(color), isCapture(capture), capturedPieceType(capturedType),
          isPromotion(promotion), promotionType(promoType),
          isCheck(check), isCheckmate(checkmate) {}
};

class SQUARE {
public:
    POSITION position; // Position of the square on the board
    COLOR color; // Color of the square (White or Black)
    bool isOccupied; // Whether the square is occupied by a piece
    PIECE_TYPE pieceType; // Type of the piece on the square, if occupied
};

