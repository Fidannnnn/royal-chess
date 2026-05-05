#pragma once

#include "../core/GameState.hpp"
#include "../core/Move.hpp"
class AI {
    public:
        //* Default search depth - depth 4 is responsive and plays reasonably well
        static constexpr int DEFAULT_DEPTH = 4;

        explicit AI(int depth = DEFAULT_DEPTH);

        //* Returns the best move for the side to move, searched to the configured depth.
        Move bestMove(const GameState& gs);
        int nodesSearched() const;

        //* Static evaluation
    private:
        int m_depth;
        int m_nodesSearched = 0;
        static int evaluate(const GameState& gs);
        static int pst(Piece p, int r, int c);

        //* Search
        int minimax(const GameState& gs, int depth, int alpha, int beta, bool maximizing);
};
