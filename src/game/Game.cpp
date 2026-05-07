#include "Game.hpp"

#include "../engine/MoveGenerator.hpp"
#include "../input/InputParser.hpp"
#include "../renderer/Terminal.hpp"
#include "../renderer/Color.hpp"

#include <iostream>
#include <chrono>
#include <string>

// =============================================================================
//  GAME  —  Implementation
// =============================================================================

Game::Game() : m_ai(AI::DEFAULT_DEPTH) {}

// -- Menu ---------------------------------------------------------------------

char Game::showMenu() {
    std::cout << "\n"
              << CLR_BORDER << BOLD
              << "  ┌─────────────────────────────┐\n"
              << "  │       SELECT  GAME  MODE    │\n"
              << "  ├─────────────────────────────┤\n"
              << "  │  " << CLR_STATUS_OK   << "[1]" << CLR_BORDER << " Player vs Player         │\n"
              << "  │  " << CLR_STATUS_WARN << "[2]" << CLR_BORDER << " Player vs AI  (depth 4)  │\n"
              << "  │  " << CLR_STATUS_ERR  << "[q]" << CLR_BORDER << " Quit                     │\n"
              << "  └─────────────────────────────┘\n"
              << RESET
              << "\n  " << CLR_BORDER << "▶ " << RESET;
    std::cout.flush();

    std::string choice;
    if (!std::getline(std::cin, choice)) return 'q';
    if (choice == "1") return '1';
    if (choice == "2") return '2';
    if (choice == "q" || choice == "quit" || choice == "exit") return 'q';
    return ' ';  // unknown — caller re-shows the menu
}

// -- Helper: build render options for current frame ---------------------------

RenderOptions Game::buildRenderOptions(const GameState& gs, bool inCheck) {
    RenderOptions opt;
    if (!inCheck) return opt;

    // Find the king's square so the renderer can highlight it in terracotta
    Piece king = gs.isWhiteTurn() ? W_KING : B_KING;
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            if (gs.board().at(r, c) == king) { opt.checkR = r; opt.checkC = c; }
    return opt;
}

// -- Helper: captured piece tracking -----------------------------------------

void Game::trackCapture(Piece captured, bool capturedIsWhite,
                         std::vector<std::string>& capturedW,
                         std::vector<std::string>& capturedB) {
    // White pieces go in capturedW (shown on white's side of the panel),
    // black pieces go in capturedB
    if (capturedIsWhite)
        capturedW.push_back(std::string(PIECE_UNICODE[captured]));
    else
        capturedB.push_back(std::string(PIECE_UNICODE[captured]));
}

// -- AI turn ------------------------------------------------------------------

Move Game::doAiTurn(GameState& gs,
                    std::vector<std::string>& capturedB,
                    std::string& status) {
    std::cout << CLR_STATUS_WARN << BOLD << "  AI is thinking..." << RESET;
    std::cout.flush();

    auto t0 = std::chrono::steady_clock::now();
    Move aiMove = m_ai.bestMove(gs);
    auto t1 = std::chrono::steady_clock::now();
    int ms = static_cast<int>(
        std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count());

    // Track whatever the AI captured (normal capture only — en passant is rare
    // and the pawn is removed by applyMove, so the glyph is added here too)
    Piece cap = gs.board().at(aiMove.toR, aiMove.toC);
    if (!isEmpty(cap))
        // AI always plays black, so white pieces it captures go into capturedB
        capturedB.push_back(std::string(PIECE_UNICODE[cap]));

    if (aiMove.isEnPassant)
        capturedB.push_back(std::string(PIECE_UNICODE[W_PAWN]));

    gs = gs.applyMove(aiMove);
    gs.pushHistory(aiMove);

    status = std::string(CLR_STATUS_WARN) + std::string(BOLD)
           + "AI played: " + aiMove.toAlgebraic()
           + "  (" + std::to_string(m_ai.nodesSearched()) + " nodes, "
           + std::to_string(ms) + "ms)"
           + std::string(RESET);

    return aiMove;
}

// -- Human turn ---------------------------------------------------------------

bool Game::doHumanTurn(GameState& gs,
                        const std::vector<Move>& legal,
                        std::vector<std::string>& capturedW,
                        std::vector<std::string>& capturedB,
                        std::string& status,
                        bool& quit,
                        Move& out) {
    std::string input;
    if (!std::getline(std::cin, input)) { quit = true; return false; }

    if (input == "quit" || input == "exit" || input == "q") {
        quit = true;
        return false;
    }

    if (input == "help") {
        status = std::string(CLR_INFO)
               + "Moves: e2e4, g1f3, O-O (kingside), O-O-O (queenside), e7e8q (promotion)"
               + std::string(RESET);
        return false;  // re-render with the help message, wait for next input
    }

    // Parse the raw string into a Move
    Move parsed;
    if (!InputParser::parse(input, parsed)) {
        status = std::string(CLR_STATUS_ERR)
               + "Unrecognized format. Try: e2e4 or O-O"
               + std::string(RESET);
        return false;
    }

    // Validate against the legal move list
    Move legalMove;
    if (!InputParser::matchLegal(legal, parsed, gs.isWhiteTurn(), legalMove)) {
        status = std::string(CLR_STATUS_ERR)
               + "Illegal move. Type 'help' for input format."
               + std::string(RESET);
        return false;
    }

    // Track any piece captured by this move
    Piece cap = gs.board().at(legalMove.toR, legalMove.toC);
    if (!isEmpty(cap))
        trackCapture(cap, isWhite(cap), capturedW, capturedB);

    // En passant: the captured pawn isn't on the destination square
    if (legalMove.isEnPassant) {
        Piece ep = gs.isWhiteTurn() ? B_PAWN : W_PAWN;
        trackCapture(ep, isWhite(ep), capturedW, capturedB);
    }

    gs = gs.applyMove(legalMove);
    gs.pushHistory(legalMove);

    status = std::string(CLR_STATUS_OK)
           + "Move played: " + legalMove.toAlgebraic()
           + std::string(RESET);

    out = legalMove;
    return true;
}

// -- Game loop ----------------------------------------------------------------

void Game::playGame(bool aiMode) {
    GameState gs;
    std::vector<std::string> capturedW;  // white pieces taken by black
    std::vector<std::string> capturedB;  // black pieces taken by white

    std::string status = std::string(CLR_STATUS_OK)
                       + "Ready! White moves first."
                       + std::string(RESET);

    while (true) {
        bool inCheck  = MoveGenerator::isInCheck(gs, gs.isWhiteTurn());
        RenderOptions opt = buildRenderOptions(gs, inCheck);
        auto legal    = MoveGenerator::legalMoves(gs);

        // Game over — draw the final position then announce result
        if (legal.empty()) {
            BoardRenderer::draw(gs, opt, capturedW, capturedB, status, aiMode);
            if (inCheck) {
                const char* winner = gs.isWhiteTurn() ? "Black" : "White";
                std::cout << "\n  " << CLR_TITLE << BOLD
                          << "★  CHECKMATE!  " << winner << " wins!  ★"
                          << RESET << "\n\n";
            } else {
                std::cout << "\n  " << CLR_INFO << BOLD
                          << "½  STALEMATE!  Draw.  ½"
                          << RESET << "\n\n";
            }
            break;
        }

        if (inCheck) {
            status = std::string(CLR_STATUS_ERR) + std::string(BOLD)
                   + "⚠  CHECK!  Your king is under attack."
                   + std::string(RESET);
        }

        BoardRenderer::draw(gs, opt, capturedW, capturedB, status, aiMode);

        // AI plays black
        if (aiMode && !gs.isWhiteTurn()) {
            doAiTurn(gs, capturedB, status);
            continue;
        }

        // Human input
        bool quit = false;
        Move move;
        bool played = doHumanTurn(gs, legal, capturedW, capturedB, status, quit, move);

        if (quit) break;
        (void)played;  // re-render on next iteration regardless
    }

    Terminal::showCursor();
}

// -- Entry point --------------------------------------------------------------

void Game::run() {
    while (true) {
        char choice = showMenu();

        if      (choice == '1') playGame(false);
        else if (choice == '2') playGame(true);
        else if (choice == 'q') break;
        else continue;  // invalid input — re-show menu

        // Play again prompt
        std::cout << "\n  " << CLR_INFO << "Play again? [y/n]: " << RESET;
        std::string again;
        if (!std::getline(std::cin, again)) break;
        if (again != "y" && again != "Y") break;
    }

    Terminal::clearScreen();
    std::cout << "\n\n" << CLR_TITLE << BOLD
              << "  ♔  Thanks for playing Royal Chess!  ♚\n\n"
              << RESET;
    Terminal::showCursor();
}