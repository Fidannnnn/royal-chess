#include "MoveGenerator.hpp"
#include "../core/Board.hpp"

#include <algorithm>

// =============================================================================
//  MOVEGENERATOR  —  Implementation
// =============================================================================

// Direction tables — declared static so they're shared across calls
// without repeated stack allocation. All offsets are (rowDelta, colDelta).
static const int ROOK_DIRS  [4][2] = {{ 1, 0},{-1, 0},{ 0, 1},{ 0,-1}};
static const int BISHOP_DIRS[4][2] = {{ 1, 1},{ 1,-1},{-1, 1},{-1,-1}};
static const int QUEEN_DIRS [8][2] = {{ 1, 0},{-1, 0},{ 0, 1},{ 0,-1},
                                       { 1, 1},{ 1,-1},{-1, 1},{-1,-1}};
static const int KNIGHT_J   [8][2] = {{ 2, 1},{ 2,-1},{-2, 1},{-2,-1},
                                       { 1, 2},{ 1,-2},{-1, 2},{-1,-2}};
static const int KING_J     [8][2] = {{ 1, 0},{-1, 0},{ 0, 1},{ 0,-1},
                                       { 1, 1},{ 1,-1},{-1, 1},{-1,-1}};

// =============================================================================
//  Private helpers
// =============================================================================

/*
 * Appends all pseudo-legal pawn moves from (r,c) to `moves`.
 *
 * Covers:
 *   - Single forward push (blocked if occupied)
 *   - Double push from starting rank (both squares must be clear)
 *   - Diagonal captures of enemy pieces
 *   - En passant capture (enPassantCol records the file of the double-pushed pawn)
 *   - Promotion: one move emitted per legal promotion piece at the back rank
 */
void MoveGenerator::addPawnMoves(const Board& b, int r, int c, bool white,
                                  int enPassantCol, std::vector<Move>& moves) {
    const int dir      = white ? -1 :  1;  // white advances up (row decreases)
    const int startRow = white ?  6 :  1;
    const int promRow  = white ?  0 :  7;

    // -- Forward push ---------------------------------------------------------
    if (Board::inBounds(r+dir, c) && b.at(r+dir, c) == EMPTY) {
        Move m; m.fromR=r; m.fromC=c; m.toR=r+dir; m.toC=c;

        if (r+dir == promRow) {
            // Emit one move per promotion target — queen, rook, bishop, knight.
            // Queen is almost always correct, but underpromotion (knight) can
            // be forced in some positions so we must generate all choices.
            for (Piece pp : {white ? W_QUEEN  : B_QUEEN,
                             white ? W_ROOK   : B_ROOK,
                             white ? W_BISHOP : B_BISHOP,
                             white ? W_KNIGHT : B_KNIGHT}) {
                m.promotion = pp;
                moves.push_back(m);
            }
        } else {
            moves.push_back(m);
        }

        // Double push — only from the starting rank, only if both squares clear
        if (r == startRow && b.at(r+2*dir, c) == EMPTY) {
            Move m2; m2.fromR=r; m2.fromC=c; m2.toR=r+2*dir; m2.toC=c;
            moves.push_back(m2);
        }
    }

    // -- Diagonal captures (normal + en passant) ------------------------------
    for (int dc : {-1, 1}) {
        int nr = r+dir, nc = c+dc;
        if (!Board::inBounds(nr, nc)) continue;

        Piece target = b.at(nr, nc);
        // En passant is available when the opponent's pawn just double-pushed
        // to this file and the attacker is on the correct rank (5th for white, 4th for black)
        bool enp = (enPassantCol == nc && nr == (white ? 2 : 5));

        if ((!isEmpty(target) && !sameColor(b.at(r, c), target)) || enp) {
            Move m; m.fromR=r; m.fromC=c; m.toR=nr; m.toC=nc;
            m.isEnPassant = enp;

            if (nr == promRow) {
                for (Piece pp : {white ? W_QUEEN  : B_QUEEN,
                                 white ? W_ROOK   : B_ROOK,
                                 white ? W_BISHOP : B_BISHOP,
                                 white ? W_KNIGHT : B_KNIGHT}) {
                    m.promotion = pp;
                    moves.push_back(m);
                }
            } else {
                moves.push_back(m);
            }
        }
    }
}

/*
 * Appends pseudo-legal sliding moves from (r,c) along each direction in `dirs`.
 * Slides until it hits the board edge, a friendly piece (stops before),
 * or an enemy piece (captures it and stops).
 */
void MoveGenerator::addSlidingMoves(const Board& b, int r, int c,
                                     const int dirs[][2], int numDirs,
                                     std::vector<Move>& moves) {
    for (int d = 0; d < numDirs; d++) {
        for (int step = 1; step < 8; step++) {
            int nr = r + dirs[d][0] * step;
            int nc = c + dirs[d][1] * step;
            if (!Board::inBounds(nr, nc)) break;

            Piece target = b.at(nr, nc);
            if (sameColor(b.at(r, c), target)) break;  // blocked by friendly

            Move m; m.fromR=r; m.fromC=c; m.toR=nr; m.toC=nc;
            moves.push_back(m);

            if (!isEmpty(target)) break;  // captured enemy — can't slide further
        }
    }
}

/*
 * Appends pseudo-legal jump moves from (r,c) using the given offset table.
 * Used for knights and kings — no sliding, no blocking, just check bounds
 * and friendly-piece collision.
 */
void MoveGenerator::addJumpMoves(const Board& b, int r, int c,
                                  const int jumps[][2], int numJumps,
                                  std::vector<Move>& moves) {
    for (int i = 0; i < numJumps; i++) {
        int nr = r + jumps[i][0];
        int nc = c + jumps[i][1];
        if (!Board::inBounds(nr, nc)) continue;
        if (sameColor(b.at(r, c), b.at(nr, nc))) continue;

        Move m; m.fromR=r; m.fromC=c; m.toR=nr; m.toC=nc;
        moves.push_back(m);
    }
}

/*
 * Appends pseudo-legal castling moves.
 * Only checks that the path is physically clear — attack-safety (can't castle
 * out of, through, or into check) is enforced in legalMoves().
 *
 * Kingside:  king e→g, rook h→f  (toC == 6)
 * Queenside: king e→c, rook a→d  (toC == 2)
 */
void MoveGenerator::addCastlingMoves(const Board& b, bool forWhite,
                                      std::vector<Move>& moves) {
    const int row = forWhite ? 7 : 0;

    if (b.canCastleKingside(forWhite)) {
        if (b.at(row,5)==EMPTY && b.at(row,6)==EMPTY && b.at(row,7)==(forWhite?W_ROOK:B_ROOK)) {
            Move m; m.fromR=row; m.fromC=4; m.toR=row; m.toC=6; m.isCastle=true;
            moves.push_back(m);
        }
    }
    if (b.canCastleQueenside(forWhite)) {
        if (b.at(row,3)==EMPTY && b.at(row,2)==EMPTY &&
            b.at(row,1)==EMPTY && b.at(row,0)==(forWhite?W_ROOK:B_ROOK)) {
            Move m; m.fromR=row; m.fromC=4; m.toR=row; m.toC=2; m.isCastle=true;
            moves.push_back(m);
        }
    }
}

// =============================================================================
//  Public interface
// =============================================================================

std::vector<Move> MoveGenerator::pseudoMoves(const GameState& gs, bool forWhite) {
    std::vector<Move> moves;
    moves.reserve(64);  // typical position has ~30 legal moves; reserve avoids reallocs

    const Board& b = gs.board();
    const int epCol = b.enPassantCol();

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            Piece p = b.at(r, c);
            if (isEmpty(p)) continue;
            if (forWhite != isWhite(p)) continue;

            switch (p) {
                case W_PAWN: case B_PAWN:
                    addPawnMoves(b, r, c, isWhite(p), epCol, moves); break;
                case W_ROOK: case B_ROOK:
                    addSlidingMoves(b, r, c, ROOK_DIRS,   4, moves); break;
                case W_BISHOP: case B_BISHOP:
                    addSlidingMoves(b, r, c, BISHOP_DIRS, 4, moves); break;
                case W_QUEEN: case B_QUEEN:
                    addSlidingMoves(b, r, c, QUEEN_DIRS,  8, moves); break;
                case W_KNIGHT: case B_KNIGHT:
                    addJumpMoves(b, r, c, KNIGHT_J, 8, moves); break;
                case W_KING: case B_KING:
                    addJumpMoves(b, r, c, KING_J,   8, moves); break;
                default: break;
            }
        }
    }

    addCastlingMoves(b, forWhite, moves);
    return moves;
}

bool MoveGenerator::isInCheck(const GameState& gs, bool white) {
    // Find the king's square
    Piece king = white ? W_KING : B_KING;
    int kr = -1, kc = -1;
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            if (gs.board().at(r, c) == king) { kr = r; kc = c; }

    // King missing — treat as in check (guards against corrupted state)
    if (kr < 0) return true;

    // The king is in check if any opponent pseudo-move targets its square
    for (const Move& m : pseudoMoves(gs, !white))
        if (m.toR == kr && m.toC == kc) return true;

    return false;
}

std::vector<Move> MoveGenerator::legalMoves(const GameState& gs) {
    const bool white = gs.isWhiteTurn();
    auto pseudo = pseudoMoves(gs, white);
    std::vector<Move> legal;
    legal.reserve(pseudo.size());

    for (const Move& m : pseudo) {
        // Castling requires three separate checks:
        //   1. Can't castle while already in check
        //   2. King can't pass through an attacked square (intermediate)
        //   3. King can't end in check (caught by the post-apply check below)
        if (m.isCastle) {
            if (isInCheck(gs, white)) continue;

            // Test the intermediate square by temporarily moving the king there
            int midC = (m.fromC + m.toC) / 2;
            GameState mid = gs;
            mid.board().set(m.fromR, midC,   mid.board().at(m.fromR, m.fromC));
            mid.board().set(m.fromR, m.fromC, EMPTY);
            if (isInCheck(mid, white)) continue;
        }

        // Apply the move and reject it if the king is left in check
        GameState next = gs.applyMove(m);
        if (!isInCheck(next, white)) legal.push_back(m);
    }

    return legal;
}