#pragma once

#include "../logic/chess.h"
#include <limits>

#pragma once
#include "Chess.h"  // your CHESS, MOVE, POSITION, COLOR definitions
#include <vector>
#include <algorithm>
#include <limits>

// A simple minimax+alpha-beta AI for CHESS positions.
class REDSTONE {
public:
    // depth = how many half-moves to look ahead
    explicit REDSTONE(int depth = 3)
        : maxDepth(depth) {}

    MOVE findBestMove(const CHESS &rootState) {
        rootColor = rootState.currentPlayer;

        // 1) Gather all candidate moves
        CHESS tmp = rootState;
        tmp.computeNewValidMoves();
        std::vector<MOVE> candidates;
        appendMoves(tmp, candidates);

        // 2) If no moves, just return something (or throw)
        if (candidates.empty()) {
            // maybe return a sentinel move() or handle stalemate elsewhere
            return candidates.front();  // (this is just to compile; replace with your own logic)
        }

        // 3) Initialize bestValue and bestMove from the first candidate
        int bestValue = std::numeric_limits<int>::min();
        MOVE bestMove = candidates[0];

        // 4) Loop over all moves
        for (const MOVE &m : candidates) {
            CHESS next = rootState;
            next.movePiece(m);
            next.changeTurn();
            int val = minimax(next, maxDepth - 1,
                            std::numeric_limits<int>::min(),
                            std::numeric_limits<int>::max());
            if (val > bestValue) {
                bestValue = val;
                bestMove = m;
            }
        }

        return bestMove;
    }

private:
    int maxDepth;
    COLOR rootColor;

    // Minimax with alpha-beta. returns score relative to rootColor
    int minimax(CHESS &state, int depth, int alpha, int beta) {
        // terminal
        if (depth == 0 || state.checkMate || state.staleMate) {
            return evaluate(state);
        }

        state.computeNewValidMoves();
        std::vector<MOVE> moves;
        appendMoves(state, moves);
        if (moves.empty()) {
            // no moves: treat as mate or stalemate
            if (state.checkMate) return (state.currentPlayer == rootColor)
                                        ? std::numeric_limits<int>::min()
                                        : std::numeric_limits<int>::max();
            return 0;
        }

        bool isMaximizing = (state.currentPlayer == rootColor);
        if (isMaximizing) {
            int value = std::numeric_limits<int>::min();
            for (auto &m : moves) {
                CHESS next = state;
                next.movePiece(m);
                next.changeTurn();
                int score = minimax(next, depth - 1, alpha, beta);
                value = std::max(value, score);
                alpha = std::max(alpha, value);
                if (alpha >= beta) break;
            }
            return value;
        } else {
            int value = std::numeric_limits<int>::max();
            for (auto &m : moves) {
                CHESS next = state;
                next.movePiece(m);
                next.changeTurn();
                int score = minimax(next, depth - 1, alpha, beta);
                value = std::min(value, score);
                beta = std::min(beta, value);
                if (beta <= alpha) break;
            }
            return value;
        }
    }

    // Flatten movesCheck, movesCapture, movesDevelopment, movesQuiet in priority
    void appendMoves(const CHESS &state, std::vector<MOVE> &out) const {
        out.insert(out.end(), state.movesCheck.begin(), state.movesCheck.end());
        out.insert(out.end(), state.movesCapture.begin(), state.movesCapture.end());
        out.insert(out.end(), state.movesDevelopment.begin(), state.movesDevelopment.end());
        out.insert(out.end(), state.movesQuiet.begin(), state.movesQuiet.end());
    }

    // A simple material‐only evaluation: positive is good for rootColor
    int evaluate(const CHESS &state) const {
        static const int VAL[6] = {
            100,  // pawn
            320,  // knight
            330,  // bishop
            500,  // rook
            900,  // queen
            20000 // king
        };
        int score = 0;
        for (int i = 0; i < 64; ++i) {
            auto &p = state.board[i];
            if (!p) continue;
            int v = 0;
            switch (p->type) {
                case PIECE_TYPE::PAWN:   v = VAL[0]; break;
                case PIECE_TYPE::KNIGHT: v = VAL[1]; break;
                case PIECE_TYPE::BISHOP: v = VAL[2]; break;
                case PIECE_TYPE::ROOK:   v = VAL[3]; break;
                case PIECE_TYPE::QUEEN:  v = VAL[4]; break;
                case PIECE_TYPE::KING:   v = VAL[5]; break;
            }
            score += (p->color == rootColor) ? +v : -v;
        }
        return score;
    }
};