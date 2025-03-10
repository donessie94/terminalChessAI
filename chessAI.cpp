#include "chessAI.h"
#include "chesslogic.h"
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
    return 7;  // Fallback (should not occur)
}

NODE::NODE(NODE* parent, const std::vector<short>& parentState, int depth, std::pair<short, short> move)
    : state(parentState), parent(parent), depth(depth + 1), evaluation(0),
      bestMove({0, 0}), moveFromParent(move)
{
    // Simulate applying the move to the state.
    // Copy the parent's state.
    state = parentState;
    // Apply the move: destination gets the value from source, and source becomes empty.
    state[move.second] = state[move.first];
    state[move.first] = 0;

    // Handle pawn promotion if needed:
    // (Assume a pawn is represented by 1 (white) or -1 (black),
    // and the promotion rank is row 0 for white and row 7 for black.)
    bool isWhite = (state[64] > 0); // or you can determine turn from parent state if stored separately
    int promotionRank = isWhite ? 0 : 7;
    // Determine row of the destination square.
    // (Assuming you have a helper getRow function.)
    if (getRow(move.second) == promotionRank && std::abs(state[move.second]) == 1) {
        state[move.second] = isWhite ? 9 : -9;
    }

    // Build a unique string representation of the new state
    // for use in the search tree's closed set.
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

void NODE::evaluateNode() {
    // Reset the evaluation value.
    evaluation = 0;

    // Iterate over board squares only (indices 0 through 63).
    for (size_t i = 0; i < 64; i++) {
        short piece = state[i];

        // Skip empty squares.
        if (piece == 0)
            continue;

        // Skip kings since they are not factored into the evaluation.
        if (std::abs(piece) == 127)
            continue;

        // For bishops, use half the raw value.
        if (std::abs(piece) == 6) {
            evaluation += piece / 2;  // e.g., white bishop: 6/2 = +3, black bishop: (-6)/2 = -3.
        } else {
            // For all other pieces, add their raw value.
            evaluation += piece;
        }
    }
    // std::cout << "Evaluation: " << evaluation << std::endl;
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
    : chessLogic(new CHESSLOGIC()), maxDepth(3), bestMove({0, 0}), root(nullptr), closedNodes() {}

ALPHA_BETA::~ALPHA_BETA() {
        delete chessLogic;
    }

void ALPHA_BETA::clearSearch() {
    closedNodes.clear();
}

void ALPHA_BETA::search(NODE* current, int alpha, int beta) {
    // Generate all valid moves for the current node's state.
    std::vector<std::pair<short, short>> moves = chessLogic->generateAllValidMoves(current->state);

    // Terminal condition: if no valid moves exist, then this is a terminal node.
    if (moves.empty()) {
        current->evaluateNode();
        current->backUpEvaluation();
        return;
    }

    // Terminal condition: if maximum search depth is reached.
    if (current->depth == maxDepth /* || additional game-over conditions */) {
        current->evaluateNode();
        current->backUpEvaluation();
        return;
    }

    // Check whose turn it is in the current node's state.
    // The turn indicator is stored at index 64.
    if (current->state[64] > 0) { // White to move (maximizing player)
        int value = std::numeric_limits<int>::min();
        for (auto move : moves) {
            // Create a child node by applying the move to the current node's state.
            // (The NODE constructor should simulate the move and update the state accordingly.)
            NODE child(current, current->state, current->depth, move);
            search(&child, alpha, beta);
            if (child.evaluation > value) {
                value = child.evaluation;
                current->bestMove = move;
            }
            alpha = std::max(alpha, value);
            if (alpha >= beta) { // Beta cutoff
                break;
            }
        }
        current->evaluation = value;
    } else { // Black to move (minimizing player)
        int value = std::numeric_limits<int>::max();
        for (auto move : moves) {
            NODE child(current, current->state, current->depth, move);
            search(&child, alpha, beta);
            if (child.evaluation < value) {
                value = child.evaluation;
                current->bestMove = move;
            }
            beta = std::min(beta, value);
            if (beta <= alpha) { // Alpha cutoff
                break;
            }
        }
        current->evaluation = value;
    }

    // Cache the current node in the closed nodes map.
    closedNodes[current->stateString] = current;

    // Back up the evaluation to the parent (if any).
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