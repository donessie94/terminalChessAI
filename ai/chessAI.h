#ifndef CHESS_AI_H
#define CHESS_AI_H

#include <vector>
#include <utility>
#include <string>
#include <unordered_map>
#include <stack>
#include "../logic/chesslogic.h"

// NODE represents a node in the minimax search tree.
struct NODE {
    // The chess state: indices 0–63 represent board squares, index 64 is the turn indicator.
    std::vector<short> state;
    // Pointer to the parent node.
    NODE* parent;
    // Depth of this node in the tree.
    int depth;
    // Evaluation value
    double evaluation;
    // Best move from this node (represented as a pair: {source, destination}).
    std::pair<short, short> bestMove;
    // A unique string representation of the state for duplicate detection.
    std::string stateString;
    // The move that was applied to the parent's state to reach this node.
    std::pair<short, short> moveFromParent;

    short getRow(short index);

    // Default constructor.
    NODE();
    // Constructor that creates a new node from a parent state and a move.
    NODE(NODE* parent, const std::vector<short>& parentState, int depth, std::pair<short, short> move);
    // Destructor.
    ~NODE();

    // Build the unique string representation of the state.
    void buildStateString();
    // Evaluate the node
    void evaluateNode(const std::vector<std::pair<short, short>>& validMoves);
    // Back up the evaluation value to the parent node.
    void backUpEvaluation();
};

// ALPHA_BETA implements a minimax search with alpha–beta pruning.
class ALPHA_BETA {
public:
    ALPHA_BETA();
    ~ALPHA_BETA();

    // Perform the search starting from the root node.
    void search(NODE* current, double alpha, double beta);
    // Return the best move found from the root node.
    std::pair<short, short> getBestMove() const;
    // Clear any stored search data.
    void clearSearch();
    double heuristicMoveScore(const std::pair<short, short>& move, const std::vector<short>& state);
    // Maximum depth for the search.
    int maxDepth;
    // The best move found at the root.
    std::pair<short, short> bestMove;

    // Root node pointer.
    NODE* root;
    // A hash map for closed nodes (using stateString as the key).
    std::unordered_map<std::string, NODE*> closedNodes;
    CHESSLOGIC* chessLogic;
};

// ChessAI provides a high-level interface to get the best move based on the current CHESSLOGIC state.
class ChessAI {
public:
    ChessAI();
    // Given a CHESSLOGIC instance, return the best move as a {source, destination} pair.
    std::pair<short, short> getBestMove(CHESSLOGIC& game);
    double getRootEvaluation() const;

private:
    ALPHA_BETA ab; // The alpha–beta search object.
};

#endif // CHESS_AI_H