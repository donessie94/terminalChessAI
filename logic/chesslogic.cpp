// chesslogic.cpp
// This file implements the CHESSLOGIC class, which encapsulates the game state,
// move execution (including special moves such as en passant, castling, and promotion),
// move validation (including checking for checks and checkmate), and raw move generation.
// Detailed debug messages are output to std::cout when moves are invalid.

#include "chesslogic.h"
#include "globals.h"
#include <cstdlib>   // For std::abs
#include <cmath>     // For std::abs and std::pow
#include <iostream>
#include <sstream>
#include <limits>

//global pointer to the logic instance running the game
CHESSLOGIC* chessLogicPtr = nullptr;

// Stub for check: In a full implementation, this would determine whether a move creates a check.
// For now, it always returns false for demonstration.
bool check(const std::vector<short>& state, short from, short to) {
    return false;
}

// ------------------------
// CHESSLOGIC Implementation
// ------------------------

CHESSLOGIC::CHESSLOGIC() {
    // Initialize game state with 65 elements:
    // indices 0-63 represent board squares,
    // index 64 is the turn indicator (+1 for white, -1 for black).
    gameState = {
        -5, -3, -6, -9, -127, -6, -3, -5,
        -1, -1, -1, -1, -1, -1, -1, -1,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         1,  1,  1,  1,  1,  1,  1,  1,
         5,  3,  6,  9, 127,  6,  3,  5,
         1   // Turn indicator: white's turn.
    };

    chessLogicPtr = this;

    // Set fixed starting king positions.
    kingBlackPos = 4;   // Black king starts at index 4.
    kingWhitePos = 60;  // White king starts at index 60.

    // Initialize checkmate flag and clear valid moves.
    checkMateFlag = false;
    allValidMoves.clear();

    // Set up mapping from piece code to its move function using lambdas.
    moveFunctions[1]   = [this](std::pair<short, short> moveIndex) { this->movePawn(moveIndex); };
    moveFunctions[3]   = [this](std::pair<short, short> moveIndex) { this->moveKnight(moveIndex); };
    moveFunctions[5]   = [this](std::pair<short, short> moveIndex) { this->moveRook(moveIndex); };
    moveFunctions[6]   = [this](std::pair<short, short> moveIndex) { this->moveBishop(moveIndex); };
    moveFunctions[9]   = [this](std::pair<short, short> moveIndex) { this->moveQueen(moveIndex); };
    moveFunctions[127] = [this](std::pair<short, short> moveIndex) { this->moveKing(moveIndex); };
}

const std::vector<short>& CHESSLOGIC::getState() const {
    return gameState;
}

short CHESSLOGIC::turnToMove() const {
    return gameState[64];
}

void CHESSLOGIC::changeTurn() {
    gameState[64] *= -1;
}

// ---------------- Coordinate Helpers ----------------
// Returns the column (0–7) for a given board index.
short CHESSLOGIC::getCol(short index) {
    return index % 8;
}

// Returns the row (0–7) for a given board index.
short CHESSLOGIC::getRow(short index) {
    for (short k = 1; k <= 8; k++) {
        if (index < 8 * k)
            return k - 1;
    }
    return 7;  // Fallback (should not occur)
}

// ---------------- Sliding Helpers ----------------
// For rook-like moves (horizontal/vertical sliding).
bool CHESSLOGIC::attemptSlideRook(short start, short target, short delta) {
    short current = start;
    while (true) {
        // Prevent horizontal wrap-around.
        if ((delta == 1 && (current % 8) == 7) || (delta == -1 && (current % 8) == 0))
            break;
        short next = current + delta;
        if (next < 0 || next >= 64)
            break;
        if (next == target)
            return true;
        if (gameState[next] != 0)
            break;
        current = next;
    }
    return false;
}

// For bishop-like moves (diagonal sliding).
bool CHESSLOGIC::attemptSlideDiagonal(short start, short target, short delta) {
    short current = start;
    while (true) {
        short curRow = getRow(current);
        short curCol = getCol(current);
        short next = current + delta;
        if (next < 0 || next >= 64)
            break;
        short nextRow = getRow(next);
        short nextCol = getCol(next);
        // Ensure the move is exactly one diagonal step.
        if (std::abs(nextRow - curRow) != 1 || std::abs(nextCol - curCol) != 1)
            break;
        if (next == target)
            return true;
        if (gameState[next] != 0)
            break;
        current = next;
    }
    return false;
}

// ---------------- Move Validity Helpers ----------------
bool CHESSLOGIC::playerMovingEmptySquare(short sourceIndex) {
    return (gameState[sourceIndex] == 0);
}

bool CHESSLOGIC::playerCaptureOwnPiece(short sourceIndex, short destIndex) {
    return ((gameState[sourceIndex] < 0 && gameState[destIndex] < 0) ||
            (gameState[sourceIndex] > 0 && gameState[destIndex] > 0));
}

bool CHESSLOGIC::isMovePrelimValid(std::pair<short, short> moveIndex) {
    if (playerMovingEmptySquare(moveIndex.first)) {
        // std::cout << "Invalid move: Source square is empty.\n";
        return false;
    }
    if (playerMovingEnemyPiece(moveIndex.first, turnToMove())) {
        // std::cout << "Invalid move: Trying to move enemy piece.\n";
        return false;
    }
    if (playerCaptureOwnPiece(moveIndex.first, moveIndex.second)) {
        // std::cout << "Invalid move: Cannot capture your own piece.\n";
        return false;
    }
    return true;
}

bool CHESSLOGIC::playerMovingEnemyPiece(short sourceIndex, short playerTurn) {
    // Return true if the piece at sourceIndex does NOT belong to the player whose turn it is.
    return !((gameState[sourceIndex] < 0 && playerTurn < 0) ||
             (gameState[sourceIndex] > 0 && playerTurn > 0));
}

// ---------------- Undo / Execute Helpers ----------------
// Saves the current game state, king positions, and move details into the undo stack.
void CHESSLOGIC::saveLastMove(std::pair<short, short> moveIndex) {
    MoveInfo info;
    info.priorGameState = gameState;
    info.lastMove = moveIndex;
    info.lastKingWhitePos = kingWhitePos;
    info.lastKingBlackPos = kingBlackPos;
    info.whiteCanCastle = whiteCanCastle; // Save castling rights.
    info.blackCanCastle = blackCanCastle;
    info.movedPiece = gameState[moveIndex.first];
    info.capturedPiece = gameState[moveIndex.second];
    undoStack.push_back(info);
}

// Executes a validated move: saves state, updates the board, toggles turn,
// and then updates valid moves and the checkmate flag using generateAllValidMoves().
void CHESSLOGIC::executeMove(std::pair<short, short> moveIndex) {
    // Save the current state including castling rights.
    saveLastMove(moveIndex);

    // Determine if this move is a castling move.
    bool isCastlingMove = (std::abs(moveIndex.second - moveIndex.first) == 2);

    // If it's not a castling move, update castling rights.
    if (!isCastlingMove) {
        // For white: if the king moves from its starting square (60) or a rook moves from 63 or 56.
        if ((moveIndex.first == 60 && gameState[60] == 127) ||
            (moveIndex.first == 63 && gameState[63] == 5) ||
            (moveIndex.first == 56 && gameState[56] == 5)) {
            whiteCanCastle = false;
        }
        // For black: if the king moves from its starting square (4) or a rook moves from 7 or 0.
        if ((moveIndex.first == 4 && gameState[4] == -127) ||
            (moveIndex.first == 7 && gameState[7] == -5) ||
            (moveIndex.first == 0 && gameState[0] == -5)) {
            blackCanCastle = false;
        }
    }

    // Now execute the move.
    gameState[moveIndex.second] = gameState[moveIndex.first];
    gameState[moveIndex.first] = 0;
    changeTurn();
    allValidMoves = generateAllValidMoves(gameState);
    checkMateFlag = allValidMoves.empty();
}

// ---------------- King Helpers ----------------
// Returns true if the kings at pos1 and pos2 are adjacent.
bool CHESSLOGIC::kingsAreAdjacent(short pos1, short pos2) {
    short row1 = getRow(pos1);
    short col1 = getCol(pos1);
    short row2 = getRow(pos2);
    short col2 = getCol(pos2);
    return (std::abs(row1 - row2) <= 1 && std::abs(col1 - col2) <= 1);
}

// Updates the stored king position for the given side.
void CHESSLOGIC::updateKingPosition(short newPos, bool isWhite) {
    if (isWhite)
        kingWhitePos = newPos;
    else
        kingBlackPos = newPos;
}

// ---------------- Piece-specific Move Functions ----------------

// Pawn move function with en passant and promotion.
void CHESSLOGIC::movePawn(std::pair<short, short> moveIndex) {
    if (!isMovePrelimValid(moveIndex))
        return;
    bool isWhite = (gameState[moveIndex.first] > 0);
    short source = moveIndex.first;
    short target = moveIndex.second;
    short row = getRow(source);
    short col = getCol(source);
    std::vector<short> candidates;
    int forwardDir = isWhite ? -8 : 8;
    int startRow = isWhite ? 6 : 1;

    // Normal one-step forward move.
    short oneStep = source + forwardDir;
    if (oneStep >= 0 && oneStep < 64 && gameState[oneStep] == 0)
        candidates.push_back(oneStep);

    // Two-step move from starting row.
    if (getRow(source) == startRow && gameState[oneStep] == 0) {
        short twoStep = source + (forwardDir * 2);
        if (twoStep >= 0 && twoStep < 64 && gameState[twoStep] == 0)
            candidates.push_back(twoStep);
    }

    // Diagonal capture moves.
    int captureLeft = isWhite ? -9 : 7;
    int captureRight = isWhite ? -7 : 9;
    if (col > 0) {
        short capLeft = source + captureLeft;
        if (capLeft >= 0 && capLeft < 64 &&
            gameState[capLeft] != 0 &&
            ((isWhite && gameState[capLeft] < 0) || (!isWhite && gameState[capLeft] > 0)))
            candidates.push_back(capLeft);
    }
    if (col < 7) {
        short capRight = source + captureRight;
        if (capRight >= 0 && capRight < 64 &&
            gameState[capRight] != 0 &&
            ((isWhite && gameState[capRight] < 0) || (!isWhite && gameState[capRight] > 0)))
            candidates.push_back(capRight);
    }

    // Check if any normal candidate move matches the target.
    for (short cand : candidates) {
        if (cand == target) {
            if (checkAfterMove(gameState, {source, target})) {
                // Move leaves king in check; do nothing.
            } else {
                executeMove({source, target});
                // Promotion: For white, if pawn reaches row 0; for black, if pawn reaches row 7.
                if (isWhite && getRow(target) == 0)
                    gameState[target] = 9;    // Promote to white queen.
                if (!isWhite && getRow(target) == 7)
                    gameState[target] = -9;   // Promote to black queen.
            }
            return;
        }
    }

    // --- En Passant Logic ---
    // En passant is available when a pawn moves diagonally into an empty square.
    if (gameState[target] == 0 && std::abs(getCol(target) - col) == 1) {
        if (isWhite && getRow(source) == 3) { // White pawn eligible.
            if (!undoStack.empty()) {
                MoveInfo lastInfo = undoStack.back();
                if (std::abs(lastInfo.movedPiece) == 1 && lastInfo.movedPiece < 0) {
                    int lastSourceRow = getRow(lastInfo.lastMove.first);
                    int lastTargetRow = getRow(lastInfo.lastMove.second);
                    if (lastSourceRow == 1 && lastTargetRow == 3 &&
                        std::abs(getCol(lastInfo.lastMove.second) - getCol(source)) == 1) {
                        // En passant valid: remove enemy pawn.
                        gameState[lastInfo.lastMove.second] = 0;
                        if (!checkAfterMove(gameState, {source, target})) {
                            executeMove({source, target});
                        }
                        return;
                    }
                }
            }
        }
        if (!isWhite && getRow(source) == 4) { // Black pawn eligible.
            if (!undoStack.empty()) {
                MoveInfo lastInfo = undoStack.back();
                if (std::abs(lastInfo.movedPiece) == 1 && lastInfo.movedPiece > 0) {
                    int lastSourceRow = getRow(lastInfo.lastMove.first);
                    int lastTargetRow = getRow(lastInfo.lastMove.second);
                    if (lastSourceRow == 6 && lastTargetRow == 4 &&
                        std::abs(getCol(lastInfo.lastMove.second) - getCol(source)) == 1) {
                        gameState[lastInfo.lastMove.second] = 0;
                        if (!checkAfterMove(gameState, {source, target})) {
                            executeMove({source, target});
                        }
                        return;
                    }
                }
            }
        }
    }
    // If no candidate move is valid, do nothing.
}

// Knight move function.
void CHESSLOGIC::moveKnight(std::pair<short, short> moveIndex) {
    if (!isMovePrelimValid(moveIndex))
        return;
    short col = getCol(moveIndex.first);
    short row = getRow(moveIndex.first);
    short moveDiff = moveIndex.second - moveIndex.first;
    struct KnightMove { short offset; short dCol; short dRow; };
    KnightMove knightMoves[] = {
        { -6,  +2, -1 },
        {  6,  -2, +1 },
        { -10, -2, -1 },
        {  10, +2, +1 },
        { -15, +1, -2 },
        {  15, -1, +2 },
        { -17, -1, -2 },
        {  17, +1, +2 }
    };
    for (const auto &km : knightMoves) {
        if (moveDiff == km.offset) {
            if ((col + km.dCol >= 0) && (col + km.dCol < 8) &&
                (row + km.dRow >= 0) && (row + km.dRow < 8)) {
                if (!checkAfterMove(gameState, moveIndex)) {
                    executeMove(moveIndex);
                }
                return;
            }
        }
    }
}

// Rook move function.
void CHESSLOGIC::moveRook(std::pair<short, short> moveIndex) {
    if (!isMovePrelimValid(moveIndex))
        return;
    const short directions[4] = { -8, +8, +1, -1 };
    for (int d = 0; d < 4; d++) {
        if (attemptSlideRook(moveIndex.first, moveIndex.second, directions[d])) {
            if (!checkAfterMove(gameState, moveIndex)) {
                executeMove(moveIndex);
            }
            return;
        }
    }
}

// Bishop move function.
void CHESSLOGIC::moveBishop(std::pair<short, short> moveIndex) {
    if (!isMovePrelimValid(moveIndex))
        return;
    const short diagonalDeltas[4] = { -9, -7, +7, +9 };
    for (int d = 0; d < 4; d++) {
        if (attemptSlideDiagonal(moveIndex.first, moveIndex.second, diagonalDeltas[d])) {
            if (!checkAfterMove(gameState, moveIndex)) {
                executeMove(moveIndex);
            }
            return;
        }
    }
}

// Queen move function.
void CHESSLOGIC::moveQueen(std::pair<short, short> moveIndex) {
    if (!isMovePrelimValid(moveIndex))
        return;
    const short allDeltas[8] = { -8, +8, +1, -1, -9, -7, +7, +9 };
    for (int d = 0; d < 8; d++) {
        bool valid = false;
        if (d < 4)
            valid = attemptSlideRook(moveIndex.first, moveIndex.second, allDeltas[d]);
        else
            valid = attemptSlideDiagonal(moveIndex.first, moveIndex.second, allDeltas[d]);
        if (valid) {
            if (!checkAfterMove(gameState, moveIndex)) {
                executeMove(moveIndex);
            }
            return;
        }
    }
}

// King move function with castling support.
void CHESSLOGIC::moveKing(std::pair<short, short> moveIndex) {
    if (!isMovePrelimValid(moveIndex))
        return;
    if (moveIndex.second < 0 || moveIndex.second >= 64)
        return;

    // Determine if this is a castling move (king moving two squares horizontally).
    bool isCastle = (std::abs(moveIndex.second - moveIndex.first) == 2);
    if (isCastle) {
        // --- CASTLING LOGIC ---
        bool kingside = (moveIndex.second > moveIndex.first);
        bool isWhite = (gameState[moveIndex.first] > 0);
        // Expected rook starting index:
        // White: kingside at 63, queenside at 56; Black: kingside at 7, queenside at 0.
        short rookIndex = isWhite ? (kingside ? 63 : 56) : (kingside ? 7 : 0);

        // 1. Verify that the allied rook is still in its original position.
        if ((isWhite && gameState[rookIndex] != 5) ||
            (!isWhite && gameState[rookIndex] != -5)) {
            return;
        }

        // 2. Check that all squares between the king and rook are empty.
        std::vector<short> betweenSquares;
        if (isWhite) {
            betweenSquares = kingside ? std::vector<short>{61, 62} : std::vector<short>{57, 58, 59};
        } else {
            betweenSquares = kingside ? std::vector<short>{5, 6} : std::vector<short>{1, 2, 3};
        }
        for (short sq : betweenSquares) {
            if (gameState[sq] != 0)
                return;
        }

        // 3. Verify that neither the king nor the involved rook has moved before.
        for (const auto &info : undoStack) {
            if (info.lastMove.first == moveIndex.first || info.lastMove.first == rookIndex) {
                return;
            }
        }

        // 4. Check that the king is not in check on its current square,
        // does not pass through check, and does not end in check.
        short passingSquare = isWhite ? (kingside ? 61 : 59) : (kingside ? 5 : 3);
        if (checkAfterMove(gameState, {moveIndex.first, moveIndex.first}))
            return;
        if (checkAfterMove(gameState, {moveIndex.first, passingSquare}))
            return;
        if (checkAfterMove(gameState, moveIndex))
            return;

        // 5. Execute castling: move the king, then the rook.
        executeMove(moveIndex);
        updateKingPosition(moveIndex.second, isWhite);

        std::pair<short, short> rookMove;
        if (isWhite) {
            rookMove = kingside ? std::make_pair(63, 61) : std::make_pair(56, 59);
        } else {
            rookMove = kingside ? std::make_pair(7, 5) : std::make_pair(0, 3);
        }
        // Directly execute the rook move without further validation.
        gameState[rookMove.second] = gameState[rookMove.first];
        gameState[rookMove.first] = 0;
        return;
    } else {
        // --- Normal King Move ---
        short srcRow = getRow(moveIndex.first);
        short srcCol = getCol(moveIndex.first);
        short destRow = getRow(moveIndex.second);
        short destCol = getCol(moveIndex.second);
        if (!(std::abs(destRow - srcRow) <= 1 &&
              std::abs(destCol - srcCol) <= 1 &&
              !(destRow == srcRow && destCol == srcCol))) {
            //std::cout << "King move illegal: King can only move one square.\n";
            return;
        }
        bool isWhite = (gameState[moveIndex.first] > 0);
        short newKingPos = moveIndex.second;
        short otherKingPos = isWhite ? kingBlackPos : kingWhitePos;
        if (kingsAreAdjacent(newKingPos, otherKingPos)) {
            return;
        }
        if (checkAfterMove(gameState, moveIndex)) {
            return;
        }
        executeMove(moveIndex);
        updateKingPosition(newKingPos, isWhite);
    }
}

// ---------------- General Move Function ----------------
// Looks up and calls the appropriate piece-specific move function.
void CHESSLOGIC::move(std::pair<short, short> moveIndex) {
    short pieceCode = std::abs(gameState[moveIndex.first]);
    auto it = moveFunctions.find(pieceCode);
    if (it != moveFunctions.end()) {
        it->second(moveIndex);
    } else {
        // No move function defined for this piece.
    }
}

// ---------------- Undo Function ----------------
// Undoes the last move by restoring the previous game state and king positions.
bool CHESSLOGIC::undoMove() {
    if (undoStack.empty())
        return false;
    MoveInfo lastInfo = undoStack.back();
    undoStack.pop_back();
    gameState = lastInfo.priorGameState;
    kingWhitePos = lastInfo.lastKingWhitePos;
    kingBlackPos = lastInfo.lastKingBlackPos;
    whiteCanCastle = lastInfo.whiteCanCastle;  // Restore castling rights.
    blackCanCastle = lastInfo.blackCanCastle;
    return true;
}

std::vector<MoveInfo> CHESSLOGIC::getMoveHistory() const {
    return undoStack;
}

// ---------------- Raw Move Generation Helpers ----------------

// Generates knight moves as {source, destination} pairs.
std::vector<std::pair<short, short>> CHESSLOGIC::generateKnightMoves(short index, const std::vector<short>& state) {
    std::vector<std::pair<short, short>> moves;
    struct KnightMove { short offset; short dCol; short dRow; };
    KnightMove knightMoves[] = {
        { -6,  +2, -1 },
        {  6,  -2, +1 },
        { -10, -2, -1 },
        {  10, +2, +1 },
        { -15, +1, -2 },
        {  15, -1, +2 },
        { -17, -1, -2 },
        {  17, +1, +2 }
    };
    short col = index % 8;
    short row = index / 8;
    for (const auto &km : knightMoves) {
        short candidate = index + km.offset;
        if (candidate < 0 || candidate >= 64)
            continue;
        short candRow = candidate / 8;
        short candCol = candidate % 8;
        // Verify candidate is exactly a knight move away.
        if (std::abs(candRow - row) == std::abs(km.dRow) &&
            std::abs(candCol - col) == std::abs(km.dCol)) {
            if (state[candidate] == 0 ||
                (state[index] > 0 && state[candidate] < 0) ||
                (state[index] < 0 && state[candidate] > 0)) {
                moves.push_back({index, candidate});
            }
        }
    }
    return moves;
}

// Generates sliding moves (for rook, bishop, queen) using directional deltas.
std::vector<std::pair<short, short>> CHESSLOGIC::generateSlidingMoves(
    short index,
    const std::vector<short>& state,
    const std::vector<short>& deltas)
{
    std::vector<std::pair<short, short>> moves;
    for (short delta : deltas) {
        short current = index;
        while (true) {
            // Compute the next square.
            short next = current + delta;
            // Check board boundaries.
            if (next < 0 || next >= 64)
                break;
            // --- Wrapping Checks ---
            short currentCol = getCol(current);
            short nextCol = getCol(next);
            // Vertical moves (multiples of 8) should not change the column.
            if (delta % 8 == 0) {
                if (nextCol != currentCol)
                    break;
            }
            // Horizontal moves: ensure exactly one column shift.
            else if (delta == 1 || delta == -1) {
                if (std::abs(nextCol - currentCol) != 1)
                    break;
            }
            // Diagonal moves: column must change by exactly 1.
            else {
                if (std::abs(nextCol - currentCol) != 1)
                    break;
            }
            // --- End Wrapping Checks ---
            // If destination square is empty or contains an enemy piece, add the move.
            if (state[next] == 0 ||
                (state[index] > 0 && state[next] < 0) ||
                (state[index] < 0 && state[next] > 0))
            {
                moves.push_back({index, next});
            }
            // Stop sliding if the square is occupied.
            if (state[next] != 0)
                break;
            current = next;
        }
    }
    return moves;
}

// Generates moves for any piece at the given index using appropriate helper functions.
std::vector<std::pair<short, short>> CHESSLOGIC::generateMovesForPiece(short index, const std::vector<short>& state) {
    std::vector<std::pair<short, short>> moves;
    short pieceCode = std::abs(state[index]);
    switch(pieceCode) {
        case 1: { // Pawn
            bool isWhite = (state[index] > 0);
            int forward = isWhite ? -8 : 8;
            short oneStep = index + forward;
            if (oneStep >= 0 && oneStep < 64 && state[oneStep] == 0)
                moves.push_back({index, oneStep});
            int startRow = isWhite ? 6 : 1;
            if (getRow(index) == startRow) {
                short twoStep = index + (forward * 2);
                if (oneStep >= 0 && twoStep >= 0 && oneStep < 64 && twoStep < 64 &&
                    state[oneStep] == 0 && state[twoStep] == 0)
                    moves.push_back({index, twoStep});
            }
            int captureLeft = isWhite ? -9 : 7;
            int captureRight = isWhite ? -7 : 9;
            if (getCol(index) > 0) {
                short capLeft = index + captureLeft;
                if (capLeft >= 0 && capLeft < 64 &&
                    state[capLeft] != 0 &&
                    ((isWhite && state[capLeft] < 0) || (!isWhite && state[capLeft] > 0)))
                    moves.push_back({index, capLeft});
            }
            if (getCol(index) < 7) {
                short capRight = index + captureRight;
                if (capRight >= 0 && capRight < 64 &&
                    state[capRight] != 0 &&
                    ((isWhite && state[capRight] < 0) || (!isWhite && state[capRight] > 0)))
                    moves.push_back({index, capRight});
            }
            // --- En Passant Generation ---
            if (!undoStack.empty()) {
                if (isWhite && getRow(index) == 3) {
                    MoveInfo lastInfo = undoStack.back();
                    if (std::abs(lastInfo.movedPiece) == 1 && lastInfo.movedPiece < 0) {
                        int lastSourceRow = getRow(lastInfo.lastMove.first);
                        int lastTargetRow = getRow(lastInfo.lastMove.second);
                        if (lastSourceRow == 1 && lastTargetRow == 3 &&
                            std::abs(getCol(lastInfo.lastMove.second) - getCol(index)) == 1) {
                            short delta = (getCol(lastInfo.lastMove.second) < getCol(index)) ? -9 : -7;
                            short enPassantTarget = index + delta;
                            moves.push_back({index, enPassantTarget});
                        }
                    }
                }
                if (!isWhite && getRow(index) == 4) {
                    MoveInfo lastInfo = undoStack.back();
                    if (std::abs(lastInfo.movedPiece) == 1 && lastInfo.movedPiece > 0) {
                        int lastSourceRow = getRow(lastInfo.lastMove.first);
                        int lastTargetRow = getRow(lastInfo.lastMove.second);
                        if (lastSourceRow == 6 && lastTargetRow == 4 &&
                            std::abs(getCol(lastInfo.lastMove.second) - getCol(index)) == 1) {
                            short delta = (getCol(lastInfo.lastMove.second) < getCol(index)) ? 7 : 9;
                            short enPassantTarget = index + delta;
                            moves.push_back({index, enPassantTarget});
                        }
                    }
                }
            }
            // --- Promotion (Coronation) Logic ---
            // Identify candidate moves that land on the promotion rank.
            std::vector<std::pair<short, short>> promotionCandidates;
            for (const auto &mv : moves) {
                short targetRow = getRow(mv.second);
                if ((isWhite && targetRow == 0) || (!isWhite && targetRow == 7)) {
                    promotionCandidates.push_back(mv);
                }
            }
            // (Promotion candidates will be handled during move execution.)
            break;
        }
        case 3: { // Knight
            moves = generateKnightMoves(index, state);
            break;
        }
        case 5: { // Rook
            std::vector<short> rookDeltas = { -8, +8, +1, -1 };
            moves = generateSlidingMoves(index, state, rookDeltas);
            break;
        }
        case 6: { // Bishop
            std::vector<short> bishopDeltas = { -9, -7, +7, +9 };
            moves = generateSlidingMoves(index, state, bishopDeltas);
            break;
        }
        case 9: { // Queen
            std::vector<short> queenDeltas = { -8, +8, +1, -1, -9, -7, +7, +9 };
            moves = generateSlidingMoves(index, state, queenDeltas);
            break;
        }
        case 127: { // King
            // Generate normal one-square moves.
            for (int d = -9; d <= 9; d++) {
                if (d == 0)
                    continue;
                short candidate = index + d;
                if (candidate < 0 || candidate >= 64)
                    continue;
                if (std::abs(getRow(candidate) - getRow(index)) <= 1 &&
                    std::abs(getCol(candidate) - getCol(index)) <= 1) {
                    if (state[candidate] == 0 ||
                        (state[index] > 0 && state[candidate] < 0) ||
                        (state[index] < 0 && state[candidate] > 0))
                        moves.push_back({index, candidate});
                }
            }
            // Generate castling moves only if the king is on its starting square and castling rights are available.
            if ((state[index] > 0 && index == 60 && whiteCanCastle) ||
                (state[index] < 0 && index == 4 && blackCanCastle)) {
                // (Castling move generation code as before.)
                bool canKingside = true;
                bool canQueenside = true;
                // Check that the king and corresponding rook have not moved before by scanning undoStack.
                for (const auto &info : undoStack) {
                    if ((state[index] > 0 && (info.lastMove.first == 60 || info.lastMove.first == 63 || info.lastMove.first == 56)) ||
                        (state[index] < 0 && (info.lastMove.first == 4 || info.lastMove.first == 7 || info.lastMove.first == 0))) {
                        canKingside = false;
                        canQueenside = false;
                        break;
                    }
                }
                // Check for empty squares and attacked squares.
                if (state[index] > 0) { // White king.
                    if (state[61] != 0 || state[62] != 0)
                        canKingside = false;
                    else if (checkAfterMove(state, {60, 61}) || checkAfterMove(state, {60, 62}))
                        canKingside = false;
                    if (state[57] != 0 || state[58] != 0 || state[59] != 0)
                        canQueenside = false;
                    else if (checkAfterMove(state, {60, 59}) || checkAfterMove(state, {60, 58}))
                        canQueenside = false;
                } else { // Black king.
                    if (state[5] != 0 || state[6] != 0)
                        canKingside = false;
                    else if (checkAfterMove(state, {4, 5}) || checkAfterMove(state, {4, 6}))
                        canKingside = false;
                    if (state[1] != 0 || state[2] != 0 || state[3] != 0)
                        canQueenside = false;
                    else if (checkAfterMove(state, {4, 3}) || checkAfterMove(state, {4, 2}))
                        canQueenside = false;
                }
                if (canKingside)
                    moves.push_back({index, (state[index] > 0) ? 62 : 6});
                if (canQueenside)
                    moves.push_back({index, (state[index] > 0) ? 58 : 2});
            }
            break;
        }
        default:
            break;
    }
    return moves;
}

// ---------------- New Generate All Valid Moves Function ----------------
// Generates all valid moves for the given state by generating raw moves for each piece
// of the side whose turn it is and filtering out moves that leave the king in check.
std::vector<std::pair<short, short>> CHESSLOGIC::generateAllValidMoves(const std::vector<short>& state) {
    std::vector<std::pair<short, short>> validMoves;
    bool isWhite = (state[64] > 0);

    // Loop over all board squares (indices 0-63).
    for (short i = 0; i < 64; i++) {
        // If the square contains a piece belonging to the current player...
        if (state[i] != 0 && ((isWhite && state[i] > 0) || (!isWhite && state[i] < 0))) {
            std::vector<std::pair<short, short>> rawMoves = generateMovesForPiece(i, state);
            for (const auto &move : rawMoves) {
                // Use the refactored checkAfterMove that accepts the state parameter.
                if (!checkAfterMove(state, move)) {
                    validMoves.push_back(move);
                }
            }
        }
    }
    return validMoves;
}

// ---------------- Refactored Check After Move Function ----------------
// Simulates a candidate move on a dummy copy of the given state and returns true if
// it leaves the moving side's king in check.
bool CHESSLOGIC::checkAfterMove(const std::vector<short>& state, std::pair<short, short> candidateMove) {
    // 1. Create a dummy copy of the provided state.
    std::vector<short> dummyState = state;

    // 2. Simulate the candidate move on the dummy state.
    dummyState[candidateMove.second] = dummyState[candidateMove.first];
    dummyState[candidateMove.first] = 0;

    // 3. Toggle the turn indicator (now representing the enemy's turn).
    dummyState[64] *= -1;

    // 4. Determine the king's position for the moving side.
    short candidateKingPos;
    if (std::abs(state[candidateMove.first]) == 127) {
        candidateKingPos = candidateMove.second;
    } else {
        // Since dummyState[64] now represents the enemy's turn, the moving side is the opposite.
        bool movingSideIsWhite = (dummyState[64] < 0);
        candidateKingPos = getKingPositionInState(dummyState, movingSideIsWhite);
    }

    // 5. For every enemy piece in the dummy state, generate its moves.
    for (short i = 0; i < 64; i++) {
        if (dummyState[i] != 0) {
            if ((dummyState[64] < 0 && dummyState[i] < 0) ||
                (dummyState[64] > 0 && dummyState[i] > 0)) {
                std::vector<std::pair<short, short>> enemyMoves = generateMovesForPiece(i, dummyState);
                for (const auto &move : enemyMoves) {
                    if (move.second == candidateKingPos) {
                        return true; // The king would be in check.
                    }
                }
            }
        }
    }
    return false;
}

// ---------------- Helper: Get King Position In State ----------------
// Returns the index of the king for the given color in the provided state.
// isWhite = true returns white king (127), false returns black king (-127).
short CHESSLOGIC::getKingPositionInState(const std::vector<short>& state, bool isWhite) {
    short kingValue = isWhite ? 127 : -127;
    for (short i = 0; i < 64; i++) {
        if (state[i] == kingValue)
            return i;
    }
    return -1; // Should not happen in a legal state.
}