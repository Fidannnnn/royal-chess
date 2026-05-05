#pragma once

#include "../core/GameState.hpp"
#include "../core/Move.hpp"

/* 
? AI
*  Chess engine: static position evaluation + minimax search with alpha-beta
*  pruning. The AI always plays for the side to move in the given GameState.
? Responsibilities:
*    - Evaluate a position statically (material + piece-square tables)
*    - Search for the best move using minimax with alpha-beta pruning
*    - Order moves (captures first) to maximize pruning efficiency
*    - Track the node count for the last search (reported in the status bar)
? Not responsible for:
*    - Move generation (delegated to MoveGenerator)
*    - Knowing whose turn it is beyond what GameState provides
*    - Any rendering or I/O
? Design:
*    The AI is instantiated once per game (not per search call) so it can
*    hold the node counter as instance state without global variables.
*    All search methods are private — the only public interface is bestMove().
*/
class AI {
public:
    // Default search depth — depth 4 is responsive and plays reasonably well
    static constexpr int DEFAULT_DEPTH = 4;

    explicit AI(int depth = DEFAULT_DEPTH);

    // Returns the best move for the side to move, searched to the configured depth.
    // Also updates the internal node counter — call nodesSearched() afterwards.
    Move bestMove(const GameState& gs);

    // Number of nodes evaluated during the last call to bestMove()
    int nodesSearched() const;

private:
    int m_depth;
    int m_nodesSearched = 0;

    //? Static evaluation
    // Returns a centipawn score from White's perspective.
    // Positive = white is ahead, negative = black is ahead.
    // Combines raw material value with piece-square table bonuses.
    static int evaluate(const GameState& gs);

    // Looks up the piece-square table bonus for piece p on square (r,c).
    // Black's tables are mirrored vertically so both sides "think forward".
    static int pst(Piece p, int r, int c);

    //? Search

    /*
     * Minimax with alpha-beta pruning.
     *
     * White maximizes, black minimizes. Alpha is the best score the maximizer
     * has found so far; beta is the best the minimizer has found. A branch is
     * pruned as soon as we know the opponent would never let us reach it.
     *
     * Move ordering (captures first by victim value) is applied before the
     * recursive call so high-value captures are tried first, producing earlier
     * cutoffs and a shallower effective tree.
     */
    int minimax(const GameState& gs, int depth, int alpha, int beta, bool maximizing);
};
