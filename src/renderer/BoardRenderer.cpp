#include "BoardRenderer.hpp"
#include "Terminal.hpp"

#include <iostream>
#include <algorithm>
#include <string>

// =============================================================================
//  BOARDRENDERER  —  Implementation
// =============================================================================

// -- squareBG -----------------------------------------------------------------

std::string_view BoardRenderer::squareBG(const GameState& gs, int r, int c,
                                          const RenderOptions& opt) {
    // Highlight priority: check > selected > valid move target > last move > normal
    if (r == opt.checkR && c == opt.checkC) return CLR_HL_CHECK;
    if (r == opt.selR   && c == opt.selC)   return CLR_HL_SQ;

    for (const auto& p : opt.validMoves)
        if (p.first == r && p.second == c) return CLR_HL_MOVE;

    const Board& b = gs.board();
    if ((r == b.lastFromRow() && c == b.lastFromCol()) ||
        (r == b.lastToRow()   && c == b.lastToCol()))
        return CLR_HL_LAST;

    return ((r + c) % 2 == 0) ? CLR_LIGHT_SQ : CLR_DARK_SQ;
}

// -- drawTitle ----------------------------------------------------------------

void BoardRenderer::drawTitle() {
    std::cout << CLR_BORDER << BOLD
              << "  ╔══════════════════════════╗\n"
              << "  ║  " << CLR_TITLE << "   ♔  ROYAL CHESS  ♚   "
              << CLR_BORDER << "║\n"
              << "  ╚══════════════════════════╝"
              << RESET << "\n\n";
}

// -- drawFileLabels -----------------------------------------------------------

void BoardRenderer::drawFileLabels() {
    // 4-char indent aligns with rank-number (3 chars) + border pipe (1 char)
    std::cout << "    " << CLR_COORD << BOLD;
    for (char ch = 'a'; ch <= 'h'; ch++) std::cout << " " << ch << "  ";
    std::cout << RESET << "\n";
}

// -- drawPanelRow -------------------------------------------------------------

void BoardRenderer::drawPanelRow(int r, const GameState& gs,
                                  const std::vector<std::string>& capturedW,
                                  const std::vector<std::string>& capturedB,
                                  const std::string& statusMsg, bool aiMode) {
    switch (r) {
        case 0: {
            // Dark-side player label
            const char* label = aiMode ? "AI Engine" : "Player 2";
            std::cout << "    " << CLR_BLACK_SIDE << BOLD << "♘ " << label << RESET;
            break;
        }
        case 1:
            // Black pieces captured by white — shown next to black's side
            std::cout << "    " << CLR_CAPTURED;
            for (const auto& s : capturedB) std::cout << s;
            std::cout << RESET;
            break;

        case 2: {
            // Material balance: sum all piece values, white positive, black negative
            int mat = 0;
            for (int i = 0; i < 8; i++)
                for (int j = 0; j < 8; j++) {
                    Piece p = gs.board().at(i, j);
                    if (!isEmpty(p))
                        mat += isWhite(p) ? PIECE_VALUE[p] : -PIECE_VALUE[p];
                }
            if (mat > 0)
                std::cout << "    " << CLR_STATUS_OK << BOLD
                          << "⊕ White +" << mat / 100.0 << RESET;
            else if (mat < 0)
                std::cout << "    " << CLR_STATUS_WARN << BOLD
                          << "⊕ Black +" << -mat / 100.0 << RESET;
            else
                std::cout << "    " << CLR_INFO << "  Equal material" << RESET;
            break;
        }
        case 3:
            // Move counter and whose turn it is
            std::cout << "    " << CLR_MOVE_NUM << BOLD
                      << "Move " << gs.fullMoveNum() << "  ";
            if (gs.isWhiteTurn())
                std::cout << CLR_WHITE_SIDE << "● White";
            else
                std::cout << CLR_BLACK_SIDE << "● Black";
            std::cout << RESET << CLR_MOVE_NUM << " to play" << RESET;
            break;

        case 4:
            // Status message (pre-colored by the Game loop)
            std::cout << "    " << statusMsg;
            break;

        case 5:
            // White pieces captured by black — shown next to white's side
            std::cout << "    " << CLR_CAPTURED;
            for (const auto& s : capturedW) std::cout << s;
            std::cout << RESET;
            break;

        case 6:
            std::cout << "    " << CLR_WHITE_SIDE << BOLD << "♘ Player 1" << RESET;
            break;

        case 7:
            std::cout << "    " << CLR_INFO << DIM
                      << "Enter moves: e2e4, O-O, quit" << RESET;
            break;
    }
}

// -- drawRow ------------------------------------------------------------------

void BoardRenderer::drawRow(int r, const GameState& gs, const RenderOptions& opt,
                             const std::vector<std::string>& capturedW,
                             const std::vector<std::string>& capturedB,
                             const std::string& statusMsg, bool aiMode) {
    // Rank number on the left
    std::cout << CLR_COORD << BOLD << " " << (8 - r) << " " << RESET;
    std::cout << CLR_BORDER << "│" << RESET;

    // Eight squares
    for (int c = 0; c < 8; c++) {
        std::string_view bg = squareBG(gs, r, c, opt);
        Piece p = gs.board().at(r, c);

        std::cout << bg << BOLD;
        if (isEmpty(p)) {
            std::cout << "   ";
        } else {
            std::string_view fg = isWhite(p) ? CLR_W_PIECE : CLR_B_PIECE;
            std::cout << fg << PIECE_UNICODE[p] << " ";
        }
        std::cout << RESET << CLR_BORDER << "│" << RESET;
    }

    // Rank number on the right
    std::cout << CLR_COORD << BOLD << " " << (8 - r) << RESET;

    // Side panel element aligned to this row
    drawPanelRow(r, gs, capturedW, capturedB, statusMsg, aiMode);
    std::cout << "\n";
}

// -- drawHistory --------------------------------------------------------------

void BoardRenderer::drawHistory(const GameState& gs) {
    const auto& hist = gs.moveHistory();
    if (hist.empty()) return;

    // Show only the last 10 half-moves to keep the display compact
    int start = std::max(0, static_cast<int>(hist.size()) - 10);

    std::cout << "  " << CLR_BORDER << BOLD << "──── Last moves ────" << RESET << "\n  ";
    for (int i = start; i < static_cast<int>(hist.size()); i++) {
        // Print move number before each white move (even index = white's turn)
        if ((i % 2) == 0)
            std::cout << CLR_MOVE_NUM << BOLD << (i / 2 + 1) << ". " << RESET;
        std::cout << CLR_INFO << hist[i].toAlgebraic() << "  " << RESET;
    }
    std::cout << "\n";
}

// -- draw (public entry point) ------------------------------------------------

void BoardRenderer::draw(const GameState& gs,
                          const RenderOptions& opt,
                          const std::vector<std::string>& capturedW,
                          const std::vector<std::string>& capturedB,
                          const std::string& statusMsg,
                          bool aiMode) {
    Terminal::clearScreen();
    Terminal::moveCursor(1, 1);

    drawTitle();
    drawFileLabels();

    // Top border of the grid
    std::cout << "   " << CLR_BORDER << "┌";
    for (int i = 0; i < 8; i++) std::cout << "───" << (i < 7 ? "┬" : "┐");
    std::cout << RESET << "\n";

    // Eight board rows with inter-row dividers
    for (int r = 0; r < 8; r++) {
        drawRow(r, gs, opt, capturedW, capturedB, statusMsg, aiMode);

        if (r < 7) {
            std::cout << "   " << CLR_BORDER << "├";
            for (int i = 0; i < 8; i++) std::cout << "───" << (i < 7 ? "┼" : "┤");
            std::cout << RESET << "\n";
        }
    }

    // Bottom border and file labels
    std::cout << "   " << CLR_BORDER << "└";
    for (int i = 0; i < 8; i++) std::cout << "───" << (i < 7 ? "┴" : "┘");
    std::cout << RESET << "\n";

    drawFileLabels();
    std::cout << "\n";

    drawHistory(gs);
    std::cout << "\n";

    // Input prompt
    std::cout << "  " << CLR_BORDER << "▶ " << RESET;
    std::cout.flush();
}