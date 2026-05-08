#pragma once

#include "../core/Piece.hpp"
#include "../core/Move.hpp"

//  BOARD
//
//  Owns the 8×8 grid of pieces and the applyMove() logic that mutates it.
//
//  Responsibilities:
//    - Store and expose piece placement via at() and set()
//    - Apply a move to produce a new Board (copy-on-write via applyMove)
//    - Track the last move played (for highlight rendering)
//    - Track castling rights and en passant column (these are physical
//      consequences of what has happened on the board)
//
//  Not responsible for:
//    - Knowing whose turn it is (that's GameState)
//    - Move generation or legality (that's MoveGenerator)
//    - Rendering (that's BoardRenderer)
//
//  applyMove() returns a new Board by value rather than mutating in place.
//  This is the key design choice that makes the AI search straightforward:
//  each node in the tree gets its own Board copy with no undo mechanism needed.
//  The compiler's copy elision (NRVO) keeps this efficient in practice.

class Board {
public:
    // Initializes all squares to EMPTY — call reset() to set up a game position
    Board();

    // Sets the board to the standard chess starting position
    void reset();

    // Square access 

    // Returns the piece on the given square (bounds-checked in debug builds)
    Piece at(int row, int col) const;

    // Places a piece on the given square
    void set(int row, int col, Piece p);

    // Returns true if (row, col) is within the 8×8 grid
    static bool inBounds(int row, int col);

    // Move application 

    /*
     * Returns a new Board with the move applied. Does not validate legality —
     * legality filtering is MoveGenerator's job. Handles all special cases:
     *   - Normal moves and captures
     *   - Castling (repositions the rook as well as the king)
     *   - En passant (removes the captured pawn from its actual square)
     *   - Promotion (replaces the pawn with the chosen piece)
     *   - Castling-rights revocation (when a king or corner rook moves)
     *   - En passant target tracking (set after a double pawn push)
     */
    Board applyMove(const Move& m) const;

    // State accessors 

    // Castling availability flags — revoked permanently once the relevant
    // piece leaves its starting square
    bool canCastleKingside (bool white) const;
    bool canCastleQueenside(bool white) const;

    // Column of the pawn that last double-pushed, enabling en passant on the
    // next half-move only. Returns -1 when no en passant is available.
    int enPassantCol() const;

    // Coordinates of the last move played — used by the renderer to highlight
    // the origin and destination squares. All four return -1 before any move.
    int lastFromRow() const;
    int lastFromCol() const;
    int lastToRow()   const;
    int lastToCol()   const;

private:
    Piece m_squares[8][8];

    // Castling rights — four independent flags, one per rook-king pair
    bool m_wCastleK = true;
    bool m_wCastleQ = true;
    bool m_bCastleK = true;
    bool m_bCastleQ = true;

    // En passant target column (-1 = none)
    int m_enPassantCol = -1;

    // Last move tracking for the renderer
    int m_lastFromR = -1, m_lastFromC = -1;
    int m_lastToR   = -1, m_lastToC   = -1;

    // Internal helper — updates castling rights based on what just moved
    void revokeCastlingRights(Piece moving, int fromR, int fromC);
};