#include <iostream>
#include <vector>
#include <limits>

const int INF = 10000; // A large value to represent ±∞ for alpha-beta bounds

// Node structure for tree
struct Node {
    int id;             // Node identifier
    bool isMax;         // True if MAX node, false if MIN node
    int value;          // Static value if leaf (meaningful only when children empty)
    std::vector<Node*> children;
    Node(bool maxNode, int val = 0) : id(nextId++), isMax(maxNode), value(val) {}
    void addChild(Node* child) {
        children.push_back(child);
    }
private:
    static int nextId;
};
int Node::nextId = 1;   // initialize static ID counter

// Global counters for node visits
int nodeCountAB = 0;
int nodeCountPVS = 0;

int AlphaBetaMin(Node* node, int alpha, int beta, int depth = 0);

// Alpha-Beta search functions (Max and Min)
int AlphaBetaMax(Node* node, int alpha, int beta, int depth = 0) {
    nodeCountAB++;
    // Indentation for log readability
    std::string indent(depth * 2, ' ');
    if (node->children.empty()) {
        // Leaf node: return its static value
        std::cout << indent << "AlphaBeta: Node " << node->id
                  << " (leaf) returns " << node->value << "\n";
        return node->value;
    }
    std::cout << indent << "AlphaBeta: Node " << node->id
              << " (MAX) [alpha=" << alpha << ", beta=" << beta << "]\n";
    int bestVal = -INF;
    // Iterate over children (possible moves)
    for (Node* child : node->children) {
        std::cout << indent << "  AlphaBeta: Node " << node->id
                  << " exploring child Node " << child->id << "\n";
        // MAX node calls AlphaBetaMin on child
        int val = AlphaBetaMin(child, alpha, beta, depth + 1);
        std::cout << indent << "  AlphaBeta: Node " << node->id
                  << " got value " << val << " from child Node " << child->id << "\n";
        if (val > bestVal) {
            bestVal = val;
            std::cout << indent << "  AlphaBeta: Node " << node->id
                      << " updates best value to " << bestVal << "\n";
        }
        if (bestVal > alpha) {
            alpha = bestVal;
            std::cout << indent << "  AlphaBeta: Node " << node->id
                      << " updates alpha to " << alpha << "\n";
        }
        // Beta cutoff: prune remaining children
        if (bestVal >= beta) {
            std::cout << indent << "  AlphaBeta: Node " << node->id
                      << " cutoff (best " << bestVal << " >= beta " << beta << ")\n";
            break;
        }
    }
    std::cout << indent << "AlphaBeta: Node " << node->id
              << " returns " << bestVal << "\n";
    return bestVal;
}

int AlphaBetaMin(Node* node, int alpha, int beta, int depth) {
    nodeCountAB++;
    std::string indent(depth * 2, ' ');
    if (node->children.empty()) {
        // Leaf node
        std::cout << indent << "AlphaBeta: Node " << node->id
                  << " (leaf) returns " << node->value << "\n";
        return node->value;
    }
    std::cout << indent << "AlphaBeta: Node " << node->id
              << " (MIN) [alpha=" << alpha << ", beta=" << beta << "]\n";
    int bestVal = INF;
    for (Node* child : node->children) {
        std::cout << indent << "  AlphaBeta: Node " << node->id
                  << " exploring child Node " << child->id << "\n";
        // MIN node calls AlphaBetaMax on child
        int val = AlphaBetaMax(child, alpha, beta, depth + 1);
        std::cout << indent << "  AlphaBeta: Node " << node->id
                  << " got value " << val << " from child Node " << child->id << "\n";
        if (val < bestVal) {
            bestVal = val;
            std::cout << indent << "  AlphaBeta: Node " << node->id
                      << " updates best value to " << bestVal << "\n";
        }
        if (bestVal < beta) {
            beta = bestVal;
            std::cout << indent << "  AlphaBeta: Node " << node->id
                      << " updates beta to " << beta << "\n";
        }
        // Alpha cutoff: prune remaining children
        if (bestVal <= alpha) {
            std::cout << indent << "  AlphaBeta: Node " << node->id
                      << " cutoff (best " << bestVal << " <= alpha " << alpha << ")\n";
            break;
        }
    }
    std::cout << indent << "AlphaBeta: Node " << node->id
              << " returns " << bestVal << "\n";
    return bestVal;
}

// Principal Variation Search (Negascout) function
int PVS(Node* node, int alpha, int beta, int depth = 0) {
    nodeCountPVS++;
    std::string indent(depth * 2, ' ');
    if (node->children.empty()) {
        // Leaf node: evaluate from perspective of side to move.
        // If it's a MIN node's turn, invert the value (since static value is from MAX perspective).
        int eval = node->isMax ? node->value : node->value * -1;
        std::cout << indent << "PVS: Node " << node->id
                  << " (leaf) returns " << eval << "\n";
        return eval;
    }
    // Identify node type for logging
    std::string nodeType = node->isMax ? "MAX" : "MIN";
    std::cout << indent << "PVS: Node " << node->id
              << " (" << nodeType << ") [alpha=" << alpha << ", beta=" << beta << "]\n";
    int bestScore = -INF;
    bool firstChild = true;
    // Loop over children moves
    for (Node* child : node->children) {
        std::cout << indent << "  PVS: Node " << node->id
                  << " exploring child Node " << child->id << "\n";
        int score;
        if (firstChild) {
            // Full window search on first child (principal variation move)
            score = -PVS(child, -beta, -alpha, depth + 1);
            firstChild = false;
        } else {
            // Null-window (zero window) search on subsequent children
            score = -PVS(child, -alpha - 1, -alpha, depth + 1);
            // If the narrow search indicates a possible improvement, re-search with full window
            if (score > alpha && score < beta) {
                std::cout << indent << "  PVS: Node " << node->id
                          << " re-searching child Node " << child->id
                          << " with full window\n";
                score = -PVS(child, -beta, -alpha, depth + 1);
            }
        }
        std::cout << indent << "  PVS: Node " << node->id
                  << " got value " << score << " from child Node " << child->id << "\n";
        if (score > bestScore) {
            bestScore = score;
            std::cout << indent << "  PVS: Node " << node->id
                      << " updates best value to " << bestScore << "\n";
        }
        if (bestScore > alpha) {
            alpha = bestScore;
            std::cout << indent << "  PVS: Node " << node->id
                      << " updates alpha to " << alpha << "\n";
        }
        // Beta cutoff for PVS (same condition as Alpha-Beta)
        if (bestScore >= beta) {
            std::cout << indent << "  PVS: Node " << node->id
                      << " cutoff (best " << bestScore << " >= beta " << beta << ")\n";
            break;
        }
    }
    std::cout << indent << "PVS: Node " << node->id
              << " returns " << bestScore << "\n";
    return bestScore;
}

int main() {
    // Manually construct the game tree (depth 4: MAX–MIN–MAX–MIN)
    // Level 1 (Root MAX)
    Node* root = new Node(true);
    // Level 2 (MIN children of root)
    Node* node2 = new Node(false);
    Node* node3 = new Node(false);
    Node* node4 = new Node(false);
    Node* node5 = new Node(false);
    root->addChild(node2);
    root->addChild(node3);
    root->addChild(node4);
    root->addChild(node5);
    // Level 3 (MAX children)
    Node* node6 = new Node(true);
    Node* node7 = new Node(true);
    node2->addChild(node6);
    node2->addChild(node7);
    Node* node8 = new Node(true);
    Node* node9 = new Node(true);
    node3->addChild(node8);
    node3->addChild(node9);
    Node* node10 = new Node(true);
    Node* node11 = new Node(true);
    node4->addChild(node10);
    node4->addChild(node11);
    Node* node12 = new Node(true);
    Node* node13 = new Node(true);
    node5->addChild(node12);
    node5->addChild(node13);
    // Level 4 (leaves, MIN nodes with static values)
    // Node6's children (leaves with values 8, 0)
    Node* leaf6a = new Node(false, 8);
    Node* leaf6b = new Node(false, 0);
    node6->addChild(leaf6a);
    node6->addChild(leaf6b);
    // Node7's children (leaves with values 9, 3)
    Node* leaf7a = new Node(false, 9);
    Node* leaf7b = new Node(false, 3);
    node7->addChild(leaf7a);
    node7->addChild(leaf7b);
    // Node8's children (leaves with values 9, 2, 0)
    Node* leaf8a = new Node(false, 9);
    Node* leaf8b = new Node(false, 2);
    Node* leaf8c = new Node(false, 0);
    node8->addChild(leaf8a);
    node8->addChild(leaf8b);
    node8->addChild(leaf8c);
    // Node9's children (leaves with values 7, 4)
    Node* leaf9a = new Node(false, 7);
    Node* leaf9b = new Node(false, 4);
    node9->addChild(leaf9a);
    node9->addChild(leaf9b);
    // Node10's children (leaves with values 9, 3, 1)
    Node* leaf10a = new Node(false, 9);
    Node* leaf10b = new Node(false, 3);
    Node* leaf10c = new Node(false, 1);
    node10->addChild(leaf10a);
    node10->addChild(leaf10b);
    node10->addChild(leaf10c);
    // Node11's children (leaves with values 6, 2)
    Node* leaf11a = new Node(false, 6);
    Node* leaf11b = new Node(false, 2);
    node11->addChild(leaf11a);
    node11->addChild(leaf11b);
    // Node12's children (leaves with values 10, 4)
    Node* leaf12a = new Node(false, 10);
    Node* leaf12b = new Node(false, 4);
    node12->addChild(leaf12a);
    node12->addChild(leaf12b);
    // Node13's children (leaves with values 5, 0)
    Node* leaf13a = new Node(false, 5);
    Node* leaf13b = new Node(false, 0);
    node13->addChild(leaf13a);
    node13->addChild(leaf13b);

    // Run Alpha-Beta and PVS on the constructed tree
    std::cout << "Alpha-Beta logs:\n";
    int resultAB = AlphaBetaMax(root, -INF, INF);
    std::cout << "AlphaBeta node visits: " << nodeCountAB << "\n\n";
    std::cout << "PVS logs:\n";
    // For PVS, we call with the same initial window; note PVS returns a score in a negamax sense
    // (score from root's perspective). We negated inside PVS accordingly, so it should match result.
    int resultPVS = PVS(root, -INF, INF);
    std::cout << "PVS node visits: " << nodeCountPVS << "\n\n";
    std::cout << "AlphaBeta result = " << resultAB
              << ", PVS result = " << resultPVS << std::endl;

    // Cleanup: free allocated nodes (optional in this short-lived program)
    // In a real application, you'd delete all nodes to avoid memory leaks.
    return 0;
}








