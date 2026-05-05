#include "AI.hpp"
#include "MoveGenerator.hpp"

#include <algorithm>
#include <climits>

//?  PIECE-SQUARE TABLES
//
//  Positional bonuses in centipawns, from White's perspective (row 0 = rank 8).
//  Black's scores are read from the same tables but with the row mirrored
//  in pst() so both sides evaluate "towards their own back rank" consistently.
//
//?  Adapted from the Chess Programming Wiki starter tables.

static const int PST_PAWN[8][8] = {
    {  0,  0,  0,  0,  0,  0,  0,  0},
    { 50, 50, 50, 50, 50, 50, 50, 50},  //? advanced pawns are very valuable
    { 10, 10, 20, 30, 30, 20, 10, 10},
    {  5,  5, 10, 25, 25, 10,  5,  5},
    {  0,  0,  0, 20, 20,  0,  0,  0},  //? center control bonus
    {  5, -5,-10,  0,  0,-10, -5,  5},
    {  5, 10, 10,-20,-20, 10, 10,  5},  //? discourage blocking e/d pawns
    {  0,  0,  0,  0,  0,  0,  0,  0}
};

static const int PST_KNIGHT[8][8] = {
    {-50,-40,-30,-30,-30,-30,-40,-50},
    {-40,-20,  0,  0,  0,  0,-20,-40},
    {-30,  0, 10, 15, 15, 10,  0,-30},
    {-30,  5, 15, 20, 20, 15,  5,-30},  //? strongest near the center
    {-30,  0, 15, 20, 20, 15,  0,-30},
    {-30,  5, 10, 15, 15, 10,  5,-30},
    {-40,-20,  0,  5,  5,  0,-20,-40},
    {-50,-40,-30,-30,-30,-30,-40,-50}   //? rim squares are almost always bad
};

static const int PST_BISHOP[8][8] = {
    {-20,-10,-10,-10,-10,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0,  5, 10, 10,  5,  0,-10},
    {-10,  5,  5, 10, 10,  5,  5,-10},
    {-10,  0, 10, 10, 10, 10,  0,-10},
    {-10, 10, 10, 10, 10, 10, 10,-10},
    {-10,  5,  0,  0,  0,  0,  5,-10},
    {-20,-10,-10,-10,-10,-10,-10,-20}
};

static const int PST_ROOK[8][8] = {
    {  0,  0,  0,  0,  0,  0,  0,  0},
    {  5, 10, 10, 10, 10, 10, 10,  5},  //? 7th rank is powerful in endgames
    { -5,  0,  0,  0,  0,  0,  0, -5},
    { -5,  0,  0,  0,  0,  0,  0, -5},
    { -5,  0,  0,  0,  0,  0,  0, -5},
    { -5,  0,  0,  0,  0,  0,  0, -5},
    { -5,  0,  0,  0,  0,  0,  0, -5},
    {  0,  0,  0,  5,  5,  0,  0,  0}   //? slight incentive to centralize
};

static const int PST_QUEEN[8][8] = {
    {-20,-10,-10, -5, -5,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0,  5,  5,  5,  5,  0,-10},
    { -5,  0,  5,  5,  5,  5,  0, -5},
    {  0,  0,  5,  5,  5,  5,  0, -5},
    {-10,  5,  5,  5,  5,  5,  0,-10},
    {-10,  0,  5,  0,  0,  0,  0,-10},
    {-20,-10,-10, -5, -5,-10,-10,-20}
};

static const int PST_KING[8][8] = {
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-20,-30,-30,-40,-40,-30,-30,-20},
    {-10,-20,-20,-20,-20,-20,-20,-10},
    { 20, 20,  0,  0,  0,  0, 20, 20},  //? castled king behind pawns is safer
    { 20, 30, 10,  0,  0, 10, 30, 20}
};

//*  AI-Implementation

AI::AI(int depth) : m_depth(depth) {}

int AI::nodesSearched() const { return m_nodesSearched; }

//*Static evaluation 
int AI::pst(Piece p, int r, int c) {
    // Black's tables are mirrored: row 7 for black reads from row 0 of the
    // table, so both sides assess "towards their own promotion rank" the same way
    const int row = isWhite(p) ? r : (7 - r);
    switch (toWhite(p)) {
        case W_PAWN:   return PST_PAWN  [row][c];
        case W_KNIGHT: return PST_KNIGHT[row][c];
        case W_BISHOP: return PST_BISHOP[row][c];
        case W_ROOK:   return PST_ROOK  [row][c];
        case W_QUEEN:  return PST_QUEEN [row][c];
        case W_KING:   return PST_KING  [row][c];
        default:       return 0;
    }
}

int AI::evaluate(const GameState& gs) {
    int score = 0;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            Piece p = gs.board().at(r, c);
            if (isEmpty(p)) continue;
            int val = PIECE_VALUE[p] + pst(p, r, c);
            // White adds to the score, black subtracts
            // the result is always from White's perspective
            if (isWhite(p)) score += val;
            else            score -= val;
        }
    }
    return score;
}

//* Search 
int AI::minimax(const GameState& gs, int depth, int alpha, int beta, bool maximizing) {
    m_nodesSearched++;

    // Leaf node - return static evaluation
    if (depth == 0) return evaluate(gs);

    auto moves = MoveGenerator::legalMoves(gs);

    // Terminal node - checkmate or stalemate
    if (moves.empty()) {
        if (MoveGenerator::isInCheck(gs, gs.isWhiteTurn()))
            return maximizing ? -100000 : 100000;  // checkmate
        return 0;                                   // stalemate
    }

    // Move ordering: try captures first, ranked by the victim's material value.
    // Higher-value captures are explored first, producing earlier beta cutoffs
    // and significantly reducing the number of nodes the search visits.
    std::sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b) {
        int va = !isEmpty(gs.board().at(a.toR, a.toC))
                 ? PIECE_VALUE[gs.board().at(a.toR, a.toC)] : 0;
        int vb = !isEmpty(gs.board().at(b.toR, b.toC))
                 ? PIECE_VALUE[gs.board().at(b.toR, b.toC)] : 0;
        return va > vb;
    });

    if (maximizing) {
        int best = INT_MIN;
        for (const Move& m : moves) {
            GameState next = gs.applyMove(m);
            best  = std::max(best, minimax(next, depth - 1, alpha, beta, false));
            alpha = std::max(alpha, best);
            if (beta <= alpha) break;  // beta cutoff - minimizer won't allow this
        }
        return best;
    } else {
        int best = INT_MAX;
        for (const Move& m : moves) {
            GameState next = gs.applyMove(m);
            best = std::min(best, minimax(next, depth - 1, alpha, beta, true));
            beta = std::min(beta, best);
            if (beta <= alpha) break;  // alpha cutoff - maximizer won't allow this
        }
        return best;
    }
}

Move AI::bestMove(const GameState& gs) {
    auto moves = MoveGenerator::legalMoves(gs);
    m_nodesSearched = 0;

    Move best     = moves[0];
    int bestScore = gs.isWhiteTurn() ? INT_MIN : INT_MAX;

    for (const Move& m : moves) {
        GameState next = gs.applyMove(m);
        int score = minimax(next, m_depth - 1, INT_MIN, INT_MAX, !gs.isWhiteTurn());

        if (gs.isWhiteTurn() ? score > bestScore : score < bestScore) {
            bestScore = score;
            best      = m;
        }
    }

    return best;
}