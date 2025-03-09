#ifndef CHESSLOGIC_H
#define CHESSLOGIC_H

#include <vector>
#include <utility>
#include <unordered_map>
#include <functional>
#include <string>
#include "moveinfo.h"

struct CHESSLOGIC {
public:
    CHESSLOGIC();

    // Accessors
    const std::vector<short>& getState() const;
    short turnToMove() const;
    void changeTurn();

    // Move dispatch and undo functionality.
    void move(std::pair<short, short> moveIndex);
    bool undoMove();
    std::vector<MoveInfo> getMoveHistory() const;

    // Public helper needed for move validity.
    bool playerMovingEnemyPiece(short sourceIndex, short playerTurn);

private:
    // Game state: indices 0–63 represent board squares; index 64 is the turn indicator.
    std::vector<short> gameState;

    // Mapping from absolute piece code to its move function.
    std::unordered_map<short, std::function<void(std::pair<short, short>)>> moveFunctions;

    // --- King Position Storage ---
    // For standard starting position:
    //   - Black king (-127) is at index 4.
    //   - White king (127) is at index 60.
    short kingWhitePos;
    short kingBlackPos;

    // Undo stack: each move pushed here.
    std::vector<MoveInfo> undoStack;

    // --- Board Coordinate Helpers ---
    short getCol(short index);
    short getRow(short index);

    // --- Sliding Helpers (for rook and bishop moves) ---
    bool attemptSlideRook(short start, short target, short delta);
    bool attemptSlideDiagonal(short start, short target, short delta);

    // --- Move Validity Helpers ---
    bool playerMovingEmptySquare(short sourceIndex);
    bool playerCaptureOwnPiece(short sourceIndex, short destIndex);
    bool isMovePrelimValid(std::pair<short, short> moveIndex);
    bool checkAfterMove(std::pair<short, short> candidateMove);

    // --- Undo Helper: Save current state and king positions, and moved/captured pieces ---
    void saveLastMove(std::pair<short, short> moveIndex);

    // --- Execute Move Helper ---
    void executeMove(std::pair<short, short> moveIndex);

    // --- King Proximity Check ---
    bool kingsAreAdjacent(short pos1, short pos2);
    void updateKingPosition(short newPos, bool isWhite);

    // --- Piece-specific Move Functions ---
    void movePawn(std::pair<short, short> moveIndex);
    void moveKnight(std::pair<short, short> moveIndex);
    void moveRook(std::pair<short, short> moveIndex);
    void moveBishop(std::pair<short, short> moveIndex);
    void moveQueen(std::pair<short, short> moveIndex);
    void moveKing(std::pair<short, short> moveIndex);

    // ---------------- Raw Move Generation Helpers ----------------

    // Generates knight moves as {source, destination} pairs.
    std::vector<std::pair<short, short>> generateKnightMoves(short index, const std::vector<short>& state);
    std::vector<std::pair<short, short>> generateSlidingMoves(short index, const std::vector<short>& state, const std::vector<short>& deltas);
    std::vector<std::pair<short, short>> generateMovesForPiece(short index, const std::vector<short>& state);
};

#endif // CHESSLOGIC_H