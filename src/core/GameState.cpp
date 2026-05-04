#include "GameState.hpp"


// =============================================================================
//  GAMESTATE  —  Implementation
// =============================================================================

GameState::GameState() {
    m_board.reset();
}

// -- Board access -------------------------------------------------------------

const Board& GameState::board() const { return m_board; }
Board& GameState::board() { return m_board; }

// -- Turn state ---------------------------------------------------------------

bool GameState::isWhiteTurn() const {
    return m_whiteTurn; }

int GameState::fulMoveNum() const {
    return m_fullMoveNum; }

int GameState::halfMoveClock() const {
    return m_halfMoveClock; }   

// -- Move application ---------------------------------------------------------

GameState GameState::applyMove(const Move& m) const {
    GameState next;

    // Delegate all board-level changes to Board::applyMove()
    next.m_board = m_board.applyMove(m);

    // Advance turn-level state
    next.m_whiteTurn = !m_whiteTurn;

    // Full-move number increments after black completes a move
    next.m_fullMoveNum = m_fullMoveNum + (!m_whiteTurn ? 1 : 0);

    // Half-move clock resets on captures and pawn moves, otherwise increments.
    // A capture is any move that lands on an occupied square.
    Piece moving   = m_board.at(m.fromR, m.fromC);
    Piece captured = m_board.at(m.toR,   m.toC);
    bool isPawn    = (moving == W_PAWN || moving == B_PAWN);
    bool isCapture = !isEmpty(captured) || m.isEnPassant;

    next.m_halfMoveClock = (isPawn || isCapture) ? 0 : m_halfMoveClock + 1;

    // History is intentionally not propagated — search nodes don't need it
    // and copying a growing vector at every node would be expensive.

    return next;
}

// -- Move history -------------------------------------------------------------

void GameState::pushHistory(const Move& m) {
    m_moveHistory.push_back(m);
}

const std::vector<Move>& GameState::moveHistory() const {
    return m_moveHistory;
}