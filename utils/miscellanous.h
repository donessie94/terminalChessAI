#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <utility>   // for std::pair
#include <stdexcept>

constexpr int SQUARE_SIZE  = 90; // Size of each square in pixels
constexpr int SQUARE_WIDTH = 94;
constexpr int SQUARE_HEIGHT = 93;
constexpr int LINE_SIZE = 4;
constexpr int BOARD_START_W = 122;
constexpr int BOARD_START_H = 128;
constexpr int BOARD_WIDTH  = 8 * SQUARE_SIZE; // Width of the board in pixels
constexpr int BOARD_HEIGHT = 8 * SQUARE_SIZE; // Height of the board in pixels

// Color enum for white/black
enum class COLOR { WHITE, BLACK };

// Piece type enum for different chess pieces (if you need it later)
enum class PIECE_TYPE { PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING, EMPTY };

// Return {PAWN, ROOK, …, KING}
std::vector<PIECE_TYPE> allPieceTypes();

// ----------------------------------------------------------------
// Helper: build a map from [0..63] → (rank, fileLetter) in “a1→0, b1→1, …, h8→63” order
std::unordered_map<int, std::pair<int,char>> buildPosToNotation();

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
    void setPosition(int newPos);

    // Constructor to initialize position
    POSITION(int pos);

    //copy constructor
    POSITION(const POSITION& other) = default;

    std::string toAlgebraicNotation() const;
};

class MOVE {
public:
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
    MOVE(POSITION fromPos, POSITION toPos, PIECE_TYPE pt,
        COLOR color = COLOR::WHITE, bool capture = false, PIECE_TYPE capturedType = PIECE_TYPE::EMPTY,
        bool promotion = false, PIECE_TYPE promoType = PIECE_TYPE::EMPTY,
        bool check = false, bool checkmate = false);
};

class SQUARE {
public:
    POSITION position; // Position of the square on the board
    COLOR color; // Color of the square (White or Black)
    bool isOccupied; // Whether the square is occupied by a piece
    PIECE_TYPE pieceType; // Type of the piece on the square, if occupied
};

