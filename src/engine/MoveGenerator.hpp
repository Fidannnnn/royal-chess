#pragma once

#include "../core/GameState.hpp"
#include "../core/Move.hpp"
#include <vector>

// =============================================================================
//  MOVEGENERATOR
//
//  Stateless move generation — takes a GameState and produces Move lists.
//  All methods are static; the class is never instantiated.
//
//  Two-stage generation pipeline:
//    1. pseudoMoves()  — fast, ignores whether the king is left in check
//    2. legalMoves()   — filters pseudo-moves by calling isInCheck() after
//                        each tentative application
//
//  This split is deliberate. The AI calls pseudoMoves() internally through
//  isInCheck() (to find opponent attacks), while legalMoves() is only needed
//  at the root of each search node and for the game-over check in the loop.
//
//  Castling gets special treatment in legalMoves(): the king must not be in
//  check before, during (intermediate square), or after castling. This cannot
//  be detected with a simple post-move check so it is handled explicitly.
// =============================================================================

class MoveGenerator {
public:
    MoveGenerator() = delete;  // purely static — never instantiated

    // Returns all legal moves for the side to move in the given position.
    // An empty result means checkmate or stalemate — use isInCheck() to tell
    // which one.
    static std::vector<Move> legalMoves(const GameState& gs);

    // Returns true if the given side's king is currently under attack.
    // Implemented by generating all opponent pseudo-moves and checking whether
    // any land on the king's square.
    static bool isInCheck(const GameState& gs, bool white);

private:
    // Returns all pseudo-legal moves for the given side.
    // Pseudo-legal means the moves follow piece movement rules but may leave
    // the moving side's king in check — legality filtering is done in legalMoves().
    static std::vector<Move> pseudoMoves(const GameState& gs, bool forWhite);

    // Per-piece pseudo-move generators — each appends to `moves`
    static void addPawnMoves   (const Board& b, int r, int c, bool white,
                                 int enPassantCol, std::vector<Move>& moves);

    static void addSlidingMoves(const Board& b, int r, int c,
                                 const int dirs[][2], int numDirs,
                                 std::vector<Move>& moves);

    static void addJumpMoves   (const Board& b, int r, int c,
                                 const int jumps[][2], int numJumps,
                                 std::vector<Move>& moves);

    static void addCastlingMoves(const Board& b, bool forWhite,
                                  std::vector<Move>& moves);
};