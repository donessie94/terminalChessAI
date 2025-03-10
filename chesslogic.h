#ifndef CHESSLOGIC_H
#define CHESSLOGIC_H

#include <vector>
#include <utility>
#include <unordered_map>
#include <functional>
#include <string>
#include "moveinfo.h"

// CHESSLOGIC encapsulates the game state, move dispatching (including special moves),
// move validation (checking for check and checkmate), undo functionality, and raw move generation.
// It also stores all valid moves for the current turn (for use in algorithms like MiniMax) and a flag
// to indicate checkmate.
struct CHESSLOGIC {
public:
    CHESSLOGIC();

    // ------------------ Accessors ------------------
    // Returns the current game state vector (board squares + turn indicator).
    const std::vector<short>& getState() const;
    // Returns whose turn it is (value at index 64; +1 for white, -1 for black).
    short turnToMove() const;
    // Toggles the turn indicator.
    void changeTurn();

    // ------------------ Move Dispatch & Undo ------------------
    // Dispatches a move request to the appropriate piece-specific move function.
    void move(std::pair<short, short> moveIndex);
    // Undoes the last move; returns false if no moves remain.
    bool undoMove();
    // Returns the history of moves (each move stored as a MoveInfo record).
    std::vector<MoveInfo> getMoveHistory() const;

    std::vector<std::pair<short, short>> generateAllValidMoves(const std::vector<short>& state);
    bool checkAfterMove(const std::vector<short>& state, std::pair<short, short> candidateMove);
    short getKingPositionInState(const std::vector<short>& state, bool isWhite);
    // Stores all valid moves for the current turn.
    std::vector<std::pair<short, short>> allValidMoves;
    // True if the current player is in checkmate.
    bool checkMateFlag;

    // ------------------ Raw Move Generation ------------------
    // These helper functions generate raw moves (without full check validation) for each piece.
    // They return a vector of {source, destination} pairs.
    std::vector<std::pair<short, short>> generateKnightMoves(short index, const std::vector<short>& state);
    std::vector<std::pair<short, short>> generateSlidingMoves(short index, const std::vector<short>& state, const std::vector<short>& deltas);
    std::vector<std::pair<short, short>> generateMovesForPiece(short index, const std::vector<short>& state);
    bool gameOver() {return checkMateFlag;}

    // ------------------ Public Helper for Move Validity ------------------
    // Returns true if the piece at sourceIndex does not belong to the player whose turn it is.
    bool playerMovingEnemyPiece(short sourceIndex, short playerTurn);

private:
    // ------------------ Game State ------------------
    // gameState[0..63] represent board squares; gameState[64] is the turn indicator (+1 for white, -1 for black).
    std::vector<short> gameState;

    // ------------------ Move Function Mapping ------------------
    // Maps the absolute piece code to its corresponding move function.
    std::unordered_map<short, std::function<void(std::pair<short, short>)>> moveFunctions;

    // ------------------ King Position Storage ------------------
    // For a standard starting position:
    //   - Black king (-127) is at index 4.
    //   - White king (127) is at index 60.
    short kingWhitePos;
    short kingBlackPos;

    // ------------------ Undo Stack ------------------
    // Stores the information needed to undo moves (including the game state before the move,
    // the move itself, king positions, and which piece moved and was captured).
    std::vector<MoveInfo> undoStack;

    // ------------------ Board Coordinate Helpers ------------------
    // Returns the column (0–7) for a given board index.
    short getCol(short index);
    // Returns the row (0–7) for a given board index.
    short getRow(short index);

    // ------------------ Sliding Helpers ------------------
    // For rook-like moves (vertical/horizontal).
    bool attemptSlideRook(short start, short target, short delta);
    // For bishop-like moves (diagonal).
    bool attemptSlideDiagonal(short start, short target, short delta);

    // ------------------ Move Validity Helpers ------------------
    bool playerMovingEmptySquare(short sourceIndex);
    bool playerCaptureOwnPiece(short sourceIndex, short destIndex);
    bool isMovePrelimValid(std::pair<short, short> moveIndex);

    // ------------------ Undo / Execute Helpers ------------------
    // Saves the current game state, king positions, and move details into the undo stack.
    void saveLastMove(std::pair<short, short> moveIndex);
    // Executes a move (updates gameState, toggles turn, and then calls checkMate() to update valid moves and checkmate flag).
    void executeMove(std::pair<short, short> moveIndex);

    // ------------------ King Helpers ------------------
    // Returns true if the kings at the given positions are adjacent.
    bool kingsAreAdjacent(short pos1, short pos2);
    // Updates the stored king position for the given side.
    void updateKingPosition(short newPos, bool isWhite);

    // ------------------ Piece-specific Move Functions ------------------
    // Each of these functions implements the move logic for the respective piece (pawn, knight, rook, bishop, queen, king),
    // including special moves such as en passant, promotion, and castling.
    void movePawn(std::pair<short, short> moveIndex);
    void moveKnight(std::pair<short, short> moveIndex);
    void moveRook(std::pair<short, short> moveIndex);
    void moveBishop(std::pair<short, short> moveIndex);
    void moveQueen(std::pair<short, short> moveIndex);
    void moveKing(std::pair<short, short> moveIndex);
};

#endif // CHESSLOGIC_H