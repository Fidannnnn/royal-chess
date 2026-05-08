#include "PipeMode.hpp"

#include "core/GameState.hpp"
#include "core/Piece.hpp"
#include "engine/MoveGenerator.hpp"
#include "input/InputParser.hpp"

#include <iostream>
#include <string>

// =============================================================================
//  PIPEMODE  —  Implementation
// =============================================================================

static std::string pieceStr(Piece p) {
    if (isEmpty(p)) return "";
    const char side = isWhite(p) ? 'w' : 'b';
    switch (toWhite(p)) {
        case W_PAWN:   return {side, 'P'};
        case W_KNIGHT: return {side, 'N'};
        case W_BISHOP: return {side, 'B'};
        case W_ROOK:   return {side, 'R'};
        case W_QUEEN:  return {side, 'Q'};
        case W_KING:   return {side, 'K'};
        default:       return "";
    }
}

// Rank 1 first (row 7 in storage), rank 8 last (row 0).
// Every square followed by a comma — the format has exactly 64 commas.
static std::string serializeBoard(const GameState& gs) {
    std::string out;
    out.reserve(256);
    for (int r = 7; r >= 0; r--) {
        for (int c = 0; c < 8; c++) {
            out += pieceStr(gs.board().at(r, c));
            out += ',';
        }
    }
    return out;
}

static std::string gameResult(const GameState& gs) {
    auto legal = MoveGenerator::legalMoves(gs);
    if (!legal.empty()) return "?-?";
    if (MoveGenerator::isInCheck(gs, gs.isWhiteTurn()))
        return gs.isWhiteTurn() ? "0-1" : "1-0";
    return "1/2-1/2";
}

// Converts a promotion letter line (Q/R/B/N, upper or lower case)
// to the correct Piece for the given side.
// Returns EMPTY if the line is not a valid promotion letter.
static Piece parsePromoLetter(const std::string& line, bool white) {
    if (line.size() != 1) return EMPTY;
    char c = static_cast<char>(toupper(static_cast<unsigned char>(line[0])));
    Piece base = EMPTY;
    if      (c == 'Q') base = W_QUEEN;
    else if (c == 'R') base = W_ROOK;
    else if (c == 'B') base = W_BISHOP;
    else if (c == 'N') base = W_KNIGHT;
    else return EMPTY;
    return white ? base : static_cast<Piece>(base + 6);
}

int PipeMode::run() {
    GameState gs;
    std::string line;

    // pendingPromoMoves: when a pawn reaches the back rank we store all
    // promotion variants and wait for the next line to tell us which piece.
    std::vector<Move> pendingPromos;

    while (std::getline(std::cin, line)) {
        // Strip trailing whitespace / carriage returns
        while (!line.empty() && (line.back() == '\r' || line.back() == ' '))
            line.pop_back();

        if (line.empty() || line[0] == '#') continue;
        if (line == "/quit") break;

        // -- Promotion choice line (single letter after a promoting pawn move) --
        if (!pendingPromos.empty()) {
            Piece chosen = parsePromoLetter(line, gs.isWhiteTurn());
            if (chosen != EMPTY) {
                // Find the pending move with this promotion piece
                for (const Move& m : pendingPromos) {
                    if (m.promotion == chosen) {
                        gs = gs.applyMove(m);
                        pendingPromos.clear();
                        auto nextLegal = MoveGenerator::legalMoves(gs);
                        if (nextLegal.empty()) goto done;
                        break;
                    }
                }
                // If we matched and cleared, continue; if not found default to queen
                if (!pendingPromos.empty()) {
                    gs = gs.applyMove(pendingPromos[0]);  // default: queen (index 0)
                    pendingPromos.clear();
                }
                continue;
            }
            // Not a promo letter — apply default (queen) and fall through to parse this line as a move
            gs = gs.applyMove(pendingPromos[0]);
            pendingPromos.clear();
            auto nextLegal = MoveGenerator::legalMoves(gs);
            if (nextLegal.empty()) goto done;
        }

        // -- Normal move line --
        {
            Move parsed;
            if (!InputParser::parse(line, parsed)) continue;

            auto legal = MoveGenerator::legalMoves(gs);
            Move found;
            if (!InputParser::matchLegal(legal, parsed, gs.isWhiteTurn(), found)) continue;

            // If this move is a promotion, collect all promotion variants
            // and wait for the next line to choose the piece
            if (found.promotion != EMPTY) {
                // Gather all promotion variants for this from/to square
                pendingPromos.clear();
                for (const Move& m : legal) {
                    if (m.fromR == found.fromR && m.fromC == found.fromC &&
                        m.toR  == found.toR   && m.toC  == found.toC &&
                        m.promotion != EMPTY) {
                        pendingPromos.push_back(m);
                    }
                }
                // Don't apply yet — wait for the promotion letter on next line
                continue;
            }

            gs = gs.applyMove(found);
            auto nextLegal = MoveGenerator::legalMoves(gs);
            if (nextLegal.empty()) break;
        }
    }

done:
    // If we exited with a pending promotion, apply default (queen)
    if (!pendingPromos.empty()) {
        gs = gs.applyMove(pendingPromos[0]);
    }

    std::cout << serializeBoard(gs) << " " << gameResult(gs) << "\n";
    return 0;
}