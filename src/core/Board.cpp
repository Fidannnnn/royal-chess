#include "Board.hpp"

#include <cassert>
#include <cstdlib>

// =============================================================================
//  BOARD  —  Implementation
// =============================================================================

Board::Board() {
    for (int row = 0; row < 8; row++) 
        for (int col = 0; col < 8; col++) 
            m_squares[row][col] = EMPTY;
}

void Board::reset() {
    // Standard back rank, left to right (a-file to h-file)
    const Piece backRank[] = {
        W_ROOK, W_KNIGHT, W_BISHOP, W_QUEEN,
        W_KING, W_BISHOP, W_KNIGHT, W_ROOK
    };

    for (int c = 0; c < 8; c++) {
        m_squares[7][c] = backRank[c];           // white back rank  (rank 1)
        m_squares[6][c] = W_PAWN;                // white pawns      (rank 2)
        m_squares[1][c] = B_PAWN;                // black pawns      (rank 7)
        m_squares[0][c] = static_cast<Piece>(backRank[c] + 6); // black back rank (rank 8)
    }

    // Clear the four middle ranks
    for (int r = 2; r < 6; r++)
        for (int c = 0; c < 8; c++)
            m_squares[r][c] = EMPTY;

    // Reset all metadata to starting state
    m_wCastleK = m_wCastleQ = m_bCastleK = m_bCastleQ = true;
    m_enPassantCol = -1;
    m_lastFromR = m_lastFromC = m_lastToR = m_lastToC = -1;
}

// -- Square access ------------------------------------------------------------

Piece Board::at(int row, int col) const {
    assert(inBounds(row, col));
    return m_squares[row][col];
}

void Board::set(int row, int col, Piece p) {
    assert(inBounds(row, col));
    m_squares[row][col] = p;
}

bool Board::inBounds(int row, int col) {
    return 0 <= row && row < 8 && 0 <= col && col < 8;
}

Board Board::applyMove(const Move& m) const {
    Board newBoard = *this; // copy the current board to modify and return - the AI relies on this

    Piece moving = next.m_squares[m.fromR][m.fromC];

    // Place the piece (or promotion target) on the destination square
    next.m_squares[m.toR][m.toC]   = (m.promotion != EMPTY) ? m.promotion : moving;
    next.m_squares[m.fromR][m.fromC] = EMPTY;

    // Castling: king destination is already set above; now move the rook.
    // Kingside: rook travels from h-file (col 7) to f-file (col 5)
    // Queenside: rook travels from a-file (col 0) to d-file (col 3)
    if (m.isCastle) {
        if (m.toC == 6) { // kingside
            next.m_squares[m.toR][5] = next.m_squares[m.toR][7];
            next.m_squares[m.toR][7] = EMPTY;
        } else {          // queenside
            next.m_squares[m.toR][3] = next.m_squares[m.toR][0];
            next.m_squares[m.toR][0] = EMPTY;
        }
    }

    // En passant: the captured pawn is not on the destination square —
    // it sits on the same rank as the attacking pawn, one column over
    if (m.isEnPassant) {
        next.m_squares[m.fromR][m.toC] = EMPTY;
    }

    // Update castling rights based on what moved and from where
    next.revokeCastlingRights(moving, m.fromR, m.fromC);

    // Set en passant target for the next ply — only after a double pawn push.
    // Cleared immediately otherwise so en passant can't be claimed late.
    next.m_enPassantCol = -1;
    if ((moving == W_PAWN || moving == B_PAWN) && std::abs(m.toR - m.fromR) == 2) {
        next.m_enPassantCol = m.fromC;
    }

    // Record last move coordinates so the renderer can highlight them
    next.m_lastFromR = m.fromR;
    next.m_lastFromC = m.fromC;
    next.m_lastToR   = m.toR;
    next.m_lastToC   = m.toC;

    return next;
}

// -- Private helper -----------------------------------------------------------

void Board::revokeCastlingRights(Piece moving, int fromR, int fromC) {
    // King moves — lose both castling rights for that side immediately
    if (moving == W_KING) { m_wCastleK = m_wCastleQ = false; return; }
    if (moving == B_KING) { m_bCastleK = m_bCastleQ = false; return; }

    // Rook moves — revoke only the right corresponding to that rook's corner
    if (moving == W_ROOK) {
        if (fromR == 7 && fromC == 7) m_wCastleK = false;
        if (fromR == 7 && fromC == 0) m_wCastleQ = false;
    }
    if (moving == B_ROOK) {
        if (fromR == 0 && fromC == 7) m_bCastleK = false;
        if (fromR == 0 && fromC == 0) m_bCastleQ = false;
    }
}


// -- State accessors ----------------------------------------------------------

bool Board::canCastleKingside (bool white) const { return white ? m_wCastleK : m_bCastleK; }
bool Board::canCastleQueenside(bool white) const { return white ? m_wCastleQ : m_bCastleQ; }
int  Board::enPassantCol()  const { return m_enPassantCol; }
int  Board::lastFromRow()   const { return m_lastFromR; }
int  Board::lastFromCol()   const { return m_lastFromC; }
int  Board::lastToRow()     const { return m_lastToR; }
int  Board::lastToCol()     const { return m_lastToC; }