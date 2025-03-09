// chesslogic.cpp

#include "chesslogic.h"
#include <cstdlib>
#include <cmath>
#include <iostream>

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

    // Set up mapping from piece code to move functions.
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

// ---------------- Sliding Helpers ----------------
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
        // Must move diagonally exactly one square.
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

// ---------------- Move Validity Helpers ----------------
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

// ---------------- Undo / Execute Helpers ----------------
void CHESSLOGIC::saveLastMove(std::pair<short, short> moveIndex) {
    MoveInfo info;
    info.priorGameState = gameState;
    info.lastMove = moveIndex;
    info.lastKingWhitePos = kingWhitePos;
    info.lastKingBlackPos = kingBlackPos;
    // Record moved piece and captured piece (0 if none).
    info.movedPiece = gameState[moveIndex.first];
    info.capturedPiece = gameState[moveIndex.second];
    undoStack.push_back(info);
}
void CHESSLOGIC::executeMove(std::pair<short, short> moveIndex) {
    // Save the move before making any changes.
    saveLastMove(moveIndex);
    gameState[moveIndex.second] = gameState[moveIndex.first];
    gameState[moveIndex.first] = 0;
    changeTurn();
}

// ---------------- King Helpers ----------------
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

// ---------------- Piece-specific Move Functions ----------------
// Each function now calls checkAfterMove() before executing the move.

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
    // For each candidate move, if it matches target and does not leave king in check, execute it.
    for (short cand : candidates) {
        if (cand == target) {
            // Validate move with checkAfterMove.
            if (!checkAfterMove({source, target})) {
                executeMove({source, target});
            }
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
                // Validate with checkAfterMove.
                if (!checkAfterMove(moveIndex)) {
                    executeMove(moveIndex);
                }
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
            if (!checkAfterMove(moveIndex)) {
                executeMove(moveIndex);
            }
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
            if (!checkAfterMove(moveIndex)) {
                executeMove(moveIndex);
            }
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
            if (!checkAfterMove(moveIndex)) {
                executeMove(moveIndex);
            }
            return;
        }
    }
}

void CHESSLOGIC::moveKing(std::pair<short, short> moveIndex) {
    if (!isMovePrelimValid(moveIndex))
        return;
    if (moveIndex.second < 0 || moveIndex.second >= 64)
        return;
    short srcRow = getRow(moveIndex.first);
    short srcCol = getCol(moveIndex.first);
    short destRow = getRow(moveIndex.second);
    short destCol = getCol(moveIndex.second);
    if (!(std::abs(destRow - srcRow) <= 1 &&
          std::abs(destCol - srcCol) <= 1 &&
          !(destRow == srcRow && destCol == srcCol))) {
        std::cout << "King move illegal: king can only move one square.\n";
        return;
    }
    bool isWhite = (gameState[moveIndex.first] > 0);
    short newKingPos = moveIndex.second;
    short otherKingPos = isWhite ? kingBlackPos : kingWhitePos;
    if (kingsAreAdjacent(newKingPos, otherKingPos)) {
        std::cout << "King move illegal: kings would be adjacent.\n";
        return;
    }
    if (!checkAfterMove(moveIndex)) {
        executeMove(moveIndex);
        updateKingPosition(newKingPos, isWhite);
    }
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
std::vector<std::pair<short, short>> CHESSLOGIC::generateSlidingMoves(short index, const std::vector<short>& state, const std::vector<short>& deltas) {
    std::vector<std::pair<short, short>> moves;
    for (short delta : deltas) {
        short current = index;
        while (true) {
            // Prevent wrapping for horizontal moves.
            if ((delta == 1 && (current % 8) == 7) || (delta == -1 && (current % 8) == 0))
                break;
            short next = current + delta;
            if (next < 0 || next >= 64)
                break;
            if (state[next] == 0 ||
               (state[index] > 0 && state[next] < 0) ||
               (state[index] < 0 && state[next] > 0)) {
                moves.push_back({index, next});
            }
            if (state[next] != 0)
                break;
            current = next;
        }
    }
    return moves;
}

// Generates moves for any piece at index using appropriate helper functions.
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
            break;
        }
        default:
            break;
    }
    return moves;
}

// Simulate a candidate move on a dummy state and return true if it leaves the king in check.
bool CHESSLOGIC::checkAfterMove(std::pair<short, short> candidateMove) {
    // 1. Create a dummy copy of the current game state.
    std::vector<short> dummyState = gameState;

    // 2. Simulate the candidate move on the dummy state.
    dummyState[candidateMove.second] = dummyState[candidateMove.first];
    dummyState[candidateMove.first] = 0;

    // 3. Toggle the turn indicator in the dummy state.
    //    (After our move, it becomes the enemy's turn.)
    dummyState[64] *= -1;

    // 4. Determine the king's position for the moving side.
    //    If the candidate move is moving the king (code 127), then its new position is candidateMove.second.
    //    Otherwise, the king remains at its stored position.
    //    Since dummyState[64] now represents the enemy's turn, the moving side's king is the opposite.
    short candidateKingPos;
    if (std::abs(gameState[candidateMove.first]) == 127) {
        candidateKingPos = candidateMove.second;
    } else {
        candidateKingPos = (dummyState[64] < 0) ? kingWhitePos : kingBlackPos;
    }

    // 5. For every enemy piece in dummyState, generate its moves and check if any can target candidateKingPos.
    for (short i = 0; i < 64; i++) {
        if (dummyState[i] != 0) {
            // Check if this piece belongs to the enemy.
            if ((dummyState[64] < 0 && dummyState[i] < 0) ||
                (dummyState[64] > 0 && dummyState[i] > 0)) {
                std::vector<std::pair<short, short>> enemyMoves = generateMovesForPiece(i, dummyState);
                for (const auto &move : enemyMoves) {
                    if (move.second == candidateKingPos) {
                        return true; // The king is in check.
                    }
                }
            }
        }
    }
    return false; // No enemy move threatens the king.
}