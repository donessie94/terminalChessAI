#include "chesslogic.h"
#include <cstdlib>
#include <cmath>
#include <iostream>

// Stub for check.
bool check(const std::vector<short>& state, short from, short to) {
    return false;
}

// ------------------------
// CHESSLOGIC Implementation
// ------------------------

CHESSLOGIC::CHESSLOGIC() {
    // Initialize game state with 65 elements.
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

    // Fixed starting king positions.
    kingBlackPos = 4;
    kingWhitePos = 60;

    // Set up move function mapping.
    moveFunctions[1] = [this](std::pair<short, short> moveIndex) { this->movePawn(moveIndex); };
    moveFunctions[3] = [this](std::pair<short, short> moveIndex) { this->moveKnight(moveIndex); };
    moveFunctions[5] = [this](std::pair<short, short> moveIndex) { this->moveRook(moveIndex); };
    moveFunctions[6] = [this](std::pair<short, short> moveIndex) { this->moveBishop(moveIndex); };
    moveFunctions[9] = [this](std::pair<short, short> moveIndex) { this->moveQueen(moveIndex); };
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

// Board coordinate helpers.
short CHESSLOGIC::getCol(short index) {
    return index % 8;
}
short CHESSLOGIC::getRow(short index) {
    for (short k = 1; k <= 8; k++) {
        if (index < 8 * k)
            return k - 1;
    }
    return 7;
}

// Sliding helpers.
bool CHESSLOGIC::attemptSlideRook(short start, short target, short delta) {
    short current = start;
    while (true) {
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
        if (std::abs(int(nextRow) - int(curRow)) != 1 || std::abs(int(nextCol) - int(curCol)) != 1)
            break;
        if (next == target)
            return true;
        if (gameState[next] != 0)
            break;
        current = next;
    }
    return false;
}

// Move validity helpers.
bool CHESSLOGIC::playerMovingEmptySquare(short sourceIndex) {
    return (gameState[sourceIndex] == 0);
}
bool CHESSLOGIC::playerCaptureOwnPiece(short sourceIndex, short destIndex) {
    return ((gameState[sourceIndex] < 0 && gameState[destIndex] < 0) ||
            (gameState[sourceIndex] > 0 && gameState[destIndex] > 0));
}
bool CHESSLOGIC::isMovePrelimValid(std::pair<short, short> moveIndex) {
    if (playerMovingEmptySquare(moveIndex.first))
        return false;
    if (playerMovingEnemyPiece(moveIndex.first, turnToMove()))
        return false;
    if (playerCaptureOwnPiece(moveIndex.first, moveIndex.second))
        return false;
    return true;
}
bool CHESSLOGIC::playerMovingEnemyPiece(short sourceIndex, short playerTurn) {
    return !((gameState[sourceIndex] < 0 && playerTurn < 0) ||
             (gameState[sourceIndex] > 0 && playerTurn > 0));
}

// Undo helper.
void CHESSLOGIC::saveLastMove(std::pair<short, short> moveIndex) {
    MoveInfo info;
    info.priorGameState = gameState;
    info.lastMove = moveIndex;
    info.lastKingWhitePos = kingWhitePos;
    info.lastKingBlackPos = kingBlackPos;
    // Record moved piece and captured piece.
    info.movedPiece = gameState[moveIndex.first];
    info.capturedPiece = gameState[moveIndex.second]; // 0 if none.
    undoStack.push_back(info);
}

// Execute move helper.
void CHESSLOGIC::executeMove(std::pair<short, short> moveIndex) {
    saveLastMove(moveIndex);
    gameState[moveIndex.second] = gameState[moveIndex.first];
    gameState[moveIndex.first] = 0;
    changeTurn();
}

// King proximity check.
bool CHESSLOGIC::kingsAreAdjacent(short pos1, short pos2) {
    short row1 = getRow(pos1);
    short col1 = getCol(pos1);
    short row2 = getRow(pos2);
    short col2 = getCol(pos2);
    return (std::abs(row1 - row2) <= 1 && std::abs(col1 - col2) <= 1);
}
void CHESSLOGIC::updateKingPosition(short newPos, bool isWhite) {
    if (isWhite)
        kingWhitePos = newPos;
    else
        kingBlackPos = newPos;
}

// ---------------- Piece-specific move functions ----------------
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
    short oneStep = source + forwardDir;
    if (oneStep >= 0 && oneStep < 64 && gameState[oneStep] == 0)
        candidates.push_back(oneStep);
    if (row == startRow && oneStep >= 0 && oneStep < 64 && gameState[oneStep] == 0) {
        short twoStep = source + (forwardDir * 2);
        if (twoStep >= 0 && twoStep < 64 && gameState[twoStep] == 0)
            candidates.push_back(twoStep);
    }
    int captureLeft = isWhite ? -9 : 7;
    int captureRight = isWhite ? -7 : 9;
    if (col > 0) {
        short capLeft = source + captureLeft;
        if (capLeft >= 0 && capLeft < 64 && (isWhite ? (gameState[capLeft] < 0) : (gameState[capLeft] > 0)))
            candidates.push_back(capLeft);
    }
    if (col < 7) {
        short capRight = source + captureRight;
        if (capRight >= 0 && capRight < 64 && (isWhite ? (gameState[capRight] < 0) : (gameState[capRight] > 0)))
            candidates.push_back(capRight);
    }
    for (short cand : candidates) {
        if (cand == target) {
            executeMove(moveIndex);
            return;
        }
    }
}
void CHESSLOGIC::moveKnight(std::pair<short, short> moveIndex) {
    if (!isMovePrelimValid(moveIndex))
        return;
    short col = getCol(moveIndex.first);
    short row = getRow(moveIndex.first);
    short moveDiff = moveIndex.second - moveIndex.first;
    struct KnightMove {
        short offset;
        short dCol;
        short dRow;
    };
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
                executeMove(moveIndex);
                return;
            }
        }
    }
}
void CHESSLOGIC::moveRook(std::pair<short, short> moveIndex) {
    if (!isMovePrelimValid(moveIndex))
        return;
    const short directions[4] = { -8, +8, +1, -1 };
    for (int d = 0; d < 4; d++) {
        if (attemptSlideRook(moveIndex.first, moveIndex.second, directions[d])) {
            executeMove(moveIndex);
            return;
        }
    }
}
void CHESSLOGIC::moveBishop(std::pair<short, short> moveIndex) {
    if (!isMovePrelimValid(moveIndex))
        return;
    const short diagonalDeltas[4] = { -9, -7, +7, +9 };
    for (int d = 0; d < 4; d++) {
        if (attemptSlideDiagonal(moveIndex.first, moveIndex.second, diagonalDeltas[d])) {
            executeMove(moveIndex);
            return;
        }
    }
}
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
            executeMove(moveIndex);
            return;
        }
    }
}
void CHESSLOGIC::moveKing(std::pair<short, short> moveIndex) {
    // Check the common preliminary conditions.
    if (!isMovePrelimValid(moveIndex))
        return;

    // Ensure the destination is within board boundaries.
    if (moveIndex.second < 0 || moveIndex.second >= 64)
        return;

    // Enforce that the king moves exactly one square.
    // Compute source and destination rows and columns.
    short srcRow = getRow(moveIndex.first);
    short srcCol = getCol(moveIndex.first);
    short destRow = getRow(moveIndex.second);
    short destCol = getCol(moveIndex.second);
    // If the absolute difference in rows or columns is greater than 1, or if
    // the move does not change the square at all, the move is illegal.
    if (!(std::abs(destRow - srcRow) <= 1 &&
          std::abs(destCol - srcCol) <= 1 &&
          !(destRow == srcRow && destCol == srcCol))) {
        std::cout << "King move illegal: king can only move one square.\n";
        return;
    }

    // Determine which king is moving.
    bool isWhite = (gameState[moveIndex.first] > 0);
    short newKingPos = moveIndex.second;
    // Get the other king's current position.
    short otherKingPos = isWhite ? kingBlackPos : kingWhitePos;

    // Use the helper function kingsAreAdjacent to check proximity.
    if (kingsAreAdjacent(newKingPos, otherKingPos)) {
        std::cout << "King move illegal: kings would be adjacent.\n";
        return;
    }

    // If all checks pass, execute the move and update the king's position.
    executeMove(moveIndex);
    updateKingPosition(newKingPos, isWhite);
}
void CHESSLOGIC::move(std::pair<short, short> moveIndex) {
    short pieceCode = std::abs(gameState[moveIndex.first]);
    auto it = moveFunctions.find(pieceCode);
    if (it != moveFunctions.end()) {
        it->second(moveIndex);
    } else {
        std::cout << "No move function defined for piece code " << pieceCode << "\n";
    }
}
bool CHESSLOGIC::undoMove() {
    if (undoStack.empty())
        return false;
    MoveInfo lastInfo = undoStack.back();
    undoStack.pop_back();
    gameState = lastInfo.priorGameState;
    kingWhitePos = lastInfo.lastKingWhitePos;
    kingBlackPos = lastInfo.lastKingBlackPos;
    return true;
}
std::vector<MoveInfo> CHESSLOGIC::getMoveHistory() const {
    return undoStack;
}