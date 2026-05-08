#pragma once

#include "Board.hpp"
#include "Move.hpp"
#include <vector>

//  GAMESTATE
//
//  Composes a Board with the turn-level metadata needed to run a full game:
//  whose turn it is, move counters, and the move history used by the renderer.
//
//  Responsibilities:
//    - Know whose turn it is (whiteTurn)
//    - Track full-move number and half-move clock
//    - Own the move history for display in the side panel
//    - Expose applyMove() which advances all of the above after delegating
//      the board mutation to Board::applyMove()
//
//  Not responsible for:
//    - The physical board layout (that's Board)
//    - Move generation or legality (that's MoveGenerator)
//    - Rendering (that's BoardRenderer)
//
//  Like Board, applyMove() returns a new GameState by value so the AI
//  search can branch without an undo mechanism. Move history is intentionally
//  NOT copied into AI-search states — the search doesn't need it and copying
//  a growing vector at every node would be expensive. History is only
//  maintained in the live game state inside the Game loop.

class GameState {
public:
    // Constructs a GameState with the board at starting position and
    // white to move. Call this once at the beginning of each game.
    GameState();

    // Board access 
    
    // Read-only access to the underlying board
    const Board& board() const;

    // Mutable access — used by the Game loop to update the live state.
    // The AI and MoveGenerator only ever receive const references.
    Board& board();

    // Turn state 

    bool isWhiteTurn() const;

    // Full-move number — starts at 1, increments after black's move
    int fullMoveNum() const;

    // Half-move clock — moves since the last capture or pawn push.
    // Currently tracked but not enforced (50-move rule is not implemented).
    int halfMoveClock() const;

    // Move application 
    
    /*
     * Returns a new GameState with the move applied.
     * Delegates board mutation to Board::applyMove(), then advances:
     *   - whiteTurn (toggled)
     *   - fullMoveNum (incremented after black's move)
     *   - halfMoveClock (reset on captures/pawn moves, incremented otherwise)
     *
     * Move history is NOT carried into the returned state — it's only
     * meaningful in the live game, not inside the AI search tree.
     * The Game loop appends to history separately after calling this.
     */
    GameState applyMove(const Move& m) const;

    // Move history 
    // Only used by the live game state — never populated during AI search.

    void pushHistory(const Move& m);
    const std::vector<Move>& moveHistory() const;

private:
    Board m_board;
    bool  m_whiteTurn    = true;
    int   m_fullMoveNum  = 1;
    int   m_halfMoveClock = 0;

    std::vector<Move> m_moveHistory;
};