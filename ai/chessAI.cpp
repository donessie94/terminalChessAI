#include "chessAI.h"
#include "chesslogic.h"
#include "globals.h"
#include <sstream>
#include <limits>
#include <algorithm>
#include <iostream>

// -----------------------
// NODE Implementation
// -----------------------

NODE::NODE()
    : state(65, 0), parent(nullptr), depth(0), evaluation(0),
      bestMove({0, 0}), stateString(""), moveFromParent({0, 0})
{
    buildStateString();
}

// Returns the row (0–7) for a given board index.
short NODE::getRow(short index) {
    for (short k = 1; k <= 8; k++) {
        if (index < 8 * k)
            return k - 1;
    }
    return 7;
}

NODE::NODE(NODE* parent, const std::vector<short>& parentState, int depth, std::pair<short, short> move)
    : state(parentState), parent(parent), depth(depth + 1), evaluation(0.0),
      bestMove({0, 0}), moveFromParent(move)
{
    // Copy the parent's state.
    state = parentState;

    // Apply the move: move the piece from source to destination.
    state[move.second] = state[move.first];
    state[move.first] = 0;

    // Toggle the turn indicator so the state now reflects the new turn.
    state[64] *= -1;

    // Handle pawn promotion.
    // Assume that a pawn is represented by 1 (white) or -1 (black)
    // and that the promotion ranks are row 0 for white and row 7 for black.
    bool movingSideIsWhite = (parentState[64] > 0); // parent's turn indicates moving side.
    int promotionRank = movingSideIsWhite ? 0 : 7;
    if (getRow(move.second) == promotionRank && std::abs(state[move.second]) == 1) {
        state[move.second] = movingSideIsWhite ? 9 : -9;
    }

    // Set an initial evaluation bonus if the move is by a king.
    // For a king move:
    //   - If the move covers more than one column (castle), set bonus to +3 (if white) or -3 (if black).
    //   - Otherwise, set bonus to +1 (if white) or -1 (if black).
    if (std::abs(parentState[move.first]) == 127) {
        int colFrom = move.first % 8;
        int colTo = move.second % 8;
        int colDiff = std::abs(colTo - colFrom);
        if (colDiff > 1) { // Castle move
            evaluation = movingSideIsWhite ? 3.0 : -3.0;
        } else {
            evaluation = movingSideIsWhite ? 1.0 : -1.0;
        }
    }

    // Build a unique string representation of the new state
    // (this helps when storing nodes in a closed set for search).
    buildStateString();
}

NODE::~NODE() {
    // Clean up if needed.
}

void NODE::buildStateString() {
    std::ostringstream oss;
    for (short s : state) {
        oss << s << ",";
    }
    stateString = oss.str();
}

void NODE::evaluateNode(const std::vector<std::pair<short, short>>& validMoves) {
    double score = 0.0;

    // (1) Count total pieces on the board (indices 0-63; ignore turn indicator at 64).
    int pieceCount = 0;
    for (int i = 0; i < 64; i++) {
        if (state[i] != 0)
            pieceCount++;
    }

    // (2) Iterate over board squares (0-63) and compute score for each piece.
    for (int i = 0; i < 64; i++) {
        short piece = state[i];
        if (piece == 0)
            continue;  // Skip empty squares.
        if (std::abs(piece) == 127)
            continue;  // Skip kings.

        // (2a) Base value: half value for bishops, full raw value for others.
        double pieceScore = (std::abs(piece) == 6) ? piece / 2.0 : piece;

        // (2b) Conditional bonus: if >20 pieces, knights get ±0.3; else bishops get ±0.3.
        if (std::abs(piece) == 3 && pieceCount > 20) {
            pieceScore += (piece > 0) ? 0.3 : -0.3;
        } else if (std::abs(piece) == 6 && pieceCount <= 20) {
            pieceScore += (piece > 0) ? 0.3 : -0.3;
        }

        // (2c) Positional bonus: center and immediate perimeter of center.
        int row = i / 8;
        int col = i % 8;
        bool inCenter4 = ((row == 3 || row == 4) && (col == 3 || col == 4));
        bool inPerimeterOfCenter = (row >= 2 && row <= 5 && col >= 2 && col <= 5 && !inCenter4);
        if (inCenter4) {
            pieceScore += (piece > 0) ? 1.0 : -1.0;
        } else if (inPerimeterOfCenter) {
            pieceScore += (piece > 0) ? 0.5 : -0.5;
        }

        // (2d) Mobility and capture bonus:
        // For every valid move originating from square i add 0.1; if the move is a capture, add an extra 0.1.
        int mobilityCount = 0;
        int captureCount = 0;
        for (const auto &mv : validMoves) {
            if (mv.first == i) {
                mobilityCount++;
                if (state[mv.second] != 0)
                    captureCount++;
            }
        }
        double mobilityBonus = (mobilityCount * 0.1) + (captureCount * 0.1);
        if (piece < 0)
            mobilityBonus = -mobilityBonus;
        pieceScore += mobilityBonus;

        // (2e) Major piece penalty: if a major piece (knight, rook, bishop, queen) is in its initial column, subtract 0.3.
        short absP = std::abs(piece);
        bool isMajor = (absP == 3 || absP == 5 || absP == 6 || absP == 9);
        if (isMajor) {
            bool inInitialCol = false;
            // Define initial columns (these can be adjusted as needed).
            if (absP == 5 && (col == 0 || col == 7)) inInitialCol = true;
            else if (absP == 3 && (col == 1 || col == 6)) inInitialCol = true;
            else if (absP == 6 && (col == 2 || col == 5)) inInitialCol = true;
            else if (absP == 9 && col == 3) inInitialCol = true;

            if (inInitialCol) {
                double penalty = 0.3;
                pieceScore += (piece > 0) ? -penalty : penalty;
            }
        }

        // (2f) Mobility penalty: if the piece has fewer than 3 moves, apply an additional penalty of 0.3.
        if (mobilityCount < 3) {
            double penalty = 0.3;
            pieceScore += (piece > 0) ? -penalty : penalty;
        }

        // Accumulate the piece's score.
        score += pieceScore;
    }

    // (3) --- Permanent Castling Bonus ---
    // If castling rights are still available, add a bonus.
    // Assume a global pointer 'chessLogicPtr' of type CHESSLOGIC* is available.
    if (chessLogicPtr != nullptr) {
        if (chessLogicPtr->whiteCanCastle)
            score += 5;  // White gets a +5 bonus.
        if (chessLogicPtr->blackCanCastle)
            score -= 5;  // Black gets a -5 bonus.
    }

    // Store the final computed score as the node's evaluation.
    evaluation = score;
}

void NODE::backUpEvaluation() {
    // If this is the root node (no parent), nothing to do.
    if (parent == nullptr)
        return;

    // If the parent's evaluation hasn't been set yet (first time),
    // assign the current evaluation and best move.
    if (parent->evaluation == 0) {
        parent->evaluation = evaluation;  // Set parent's evaluation to this node's evaluation.
        parent->bestMove = moveFromParent;  // Record the move that led to this node.
    } else {
        // Determine whether we are maximizing or minimizing.
        // For example, if parent's turn is white (turn > 0), we might maximize.
        // (You can adjust this condition based on your specific evaluation convention.)
        if (parent->state[64] > 0) { // White to move, maximize evaluation.
            if (evaluation > parent->evaluation) {
                parent->evaluation = evaluation;
                parent->bestMove = moveFromParent;
            }
        } else { // Black to move, minimize evaluation.
            if (evaluation < parent->evaluation) {
                parent->evaluation = evaluation;
                parent->bestMove = moveFromParent;
            }
        }
    }
}

// -----------------------
// ALPHA_BETA Implementation
// -----------------------

ALPHA_BETA::ALPHA_BETA()
    : chessLogic(new CHESSLOGIC()), maxDepth(4), bestMove({0, 0}), root(nullptr), closedNodes() {}

ALPHA_BETA::~ALPHA_BETA() {
        delete chessLogic;
    }

void ALPHA_BETA::clearSearch() {
    closedNodes.clear();
}

double ALPHA_BETA::heuristicMoveScore(const std::pair<short, short>& move, const std::vector<short>& state) {
    double score = 0.0;

    // Capture bonus: if the destination square is occupied, add bonus proportional to the piece's value.
    if (state[move.second] != 0) {
        score += std::abs(state[move.second]) * 0.5;
    }

    // Central control bonus: if the move lands in the center (rows 2-5, cols 2-5), add bonus.
    int destRow = move.second / 8;
    int destCol = move.second % 8;
    if (destRow >= 2 && destRow <= 5 && destCol >= 2 && destCol <= 5) {
        score += 0.2;
    }

    // Outer squares penalty: if the move lands on one of the outer files (columns 0, 1, 6, or 7), subtract a small penalty.
    if (destCol == 0 || destCol == 1 || destCol == 6 || destCol == 7) {
        score -= 0.2;
    }

    // Mobility bonus: add bonus based on the distance of the move.
    int srcRow = move.first / 8;
    int srcCol = move.first % 8;
    double distance = std::sqrt((destRow - srcRow) * (destRow - srcRow) +
                                (destCol - srcCol) * (destCol - srcCol));
    score += distance * 0.05;

    // Development bonus for knights.
    if (std::abs(state[move.first]) == 3) { // Knight.
        int initialRank = (state[move.first] > 0) ? 7 : 0;
        if (srcRow == initialRank && destRow != initialRank) {
            score += 0.2;
        }
    }
    // Development bonus for bishops.
    if (std::abs(state[move.first]) == 6) { // Bishop.
        int initialRank = (state[move.first] > 0) ? 7 : 0;
        if (srcRow == initialRank && destRow != initialRank) {
            score += 0.2;
        }
    }

    // Castling bonus: if the move is a king move of two squares, add bonus.
    //BIG BONUS FOR CASTLE
    if (std::abs(move.second - move.first) == 2) {
        score += 3;
    }

    // --- New Bonus: "After Your Half" Bonus ---
    // For white, if the move lands in rows 0-3 (opponent's half), add a bonus.
    // For black, if the move lands in rows 4-7 (opponent's half), add a bonus.
    if (state[move.first] > 0) { // White piece.
        if (destRow < 4) {
            score += 0.15;
        }
    } else { // Black piece.
        if (destRow >= 4) {
            score += 0.15;
        }
    }

    return score;
}

void ALPHA_BETA::search(NODE* current, double alpha, double beta) {
    // Generate all valid moves for the current node's state.
    std::vector<std::pair<short, short>> moves = chessLogic->generateAllValidMoves(current->state);

    // --- MOVE ORDERING ---
    std::sort(moves.begin(), moves.end(), [&](const std::pair<short, short>& a, const std::pair<short, short>& b) {
        return heuristicMoveScore(a, current->state) > heuristicMoveScore(b, current->state);
    });

    // Terminal condition: if no valid moves exist, this is a terminal node.
    if (moves.empty()) {
        if (current->state[64] > 0) { // White to move => White is checkmated.
            current->evaluation = -9999;
        } else { // Black to move => Black is checkmated.
            current->evaluation = 9999;
        }
        current->backUpEvaluation();
        return;
    }

    // Terminal condition: maximum search depth reached.
    if (current->depth == maxDepth /* || additional game-over conditions */) {
        current->evaluateNode(moves);
        current->backUpEvaluation();
        return;
    }

    // --- (Optional Aggressive Pruning) ---
    // The following block can be used to prune branches that appear very weak
    // compared to the parent's evaluation. Tune the threshold as needed.
    /*
    double currentBestHeuristic = -std::numeric_limits<double>::infinity();
    for (const auto &move : moves) {
        double hScore = heuristicMoveScore(move, current->state);
        if (hScore > currentBestHeuristic) {
            currentBestHeuristic = hScore;
        }
    }
    if (current->parent != nullptr) {
        double parentEval = current->parent->evaluation;
        if (current->state[64] > 0 && currentBestHeuristic < parentEval - 0.5) {
            current->evaluation = currentBestHeuristic;
            current->backUpEvaluation();
            return;
        }
        if (current->state[64] < 0 && currentBestHeuristic > parentEval + 0.5) {
            current->evaluation = currentBestHeuristic;
            current->backUpEvaluation();
            return;
        }
    }
    */

    // Recursive minimax with alpha–beta pruning.
    if (current->state[64] > 0) { // White to move (maximizing)
        double value = std::numeric_limits<double>::lowest();
        for (auto move : moves) {
            NODE child(current, current->state, current->depth, move);
            search(&child, alpha, beta);
            if (child.evaluation > value) {
                value = child.evaluation;
                current->bestMove = move;
            }
            alpha = std::max(alpha, value);
            if (alpha >= beta) { // Beta cutoff.
                break;
            }
        }
        current->evaluation = value;
    } else { // Black to move (minimizing)
        double value = std::numeric_limits<double>::max();
        for (auto move : moves) {
            NODE child(current, current->state, current->depth, move);
            search(&child, alpha, beta);
            if (child.evaluation < value) {
                value = child.evaluation;
                current->bestMove = move;
            }
            beta = std::min(beta, value);
            if (beta <= alpha) { // Alpha cutoff.
                break;
            }
        }
        current->evaluation = value;
    }

    // Cache the current node.
    closedNodes[current->stateString] = current;

    if (current->parent != nullptr)
        current->backUpEvaluation();
}

std::pair<short, short> ALPHA_BETA::getBestMove() const {
    return bestMove; // Typically, bestMove would be set on the root node.
}

// -----------------------
// ChessAI Implementation
// -----------------------

ChessAI::ChessAI() {
    // The ALPHA_BETA object 'ab' is initialized by its constructor.
}

std::pair<short, short> ChessAI::getBestMove(CHESSLOGIC& game) {
    // Set the root node's state to the current game state.
    if (ab.root != nullptr) {
        delete ab.root;
    }
    ab.root = new NODE();
    ab.root->state = game.getState();
    ab.root->buildStateString();

    // Clear previous search data.
    ab.clearSearch();

    // Run the alpha–beta search from the root node.
    ab.search(ab.root, std::numeric_limits<int>::min(), std::numeric_limits<int>::max());

    // Return the best move stored in the root node.
    return ab.root->bestMove;
}

double ChessAI::getRootEvaluation() const {
    if (ab.root != nullptr) {
        return ab.root->evaluation;
    }
    return 0; // or some default value if no root exists.
}