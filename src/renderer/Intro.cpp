#include "Intro.hpp"
#include "Terminal.hpp"
#include "Color.hpp"

#include <iostream>
#include <string>

// =============================================================================
//  INTRO  —  Implementation
// =============================================================================

void Intro::drawTitle() {
    // ASCII block-letter art for "ROYAL CHESS", printed line by line.
    // The gradient shifts from gold toward amber as we move down — achieved
    // by incrementing the green channel and decrementing blue per line.
    static const std::string LINES[] = {
        "",
        "  ██████╗  ██████╗ ██╗   ██╗ █████╗ ██╗      ",
        "  ██╔══██╗██╔═══██╗╚██╗ ██╔╝██╔══██╗██║      ",
        "  ██████╔╝██║   ██║ ╚████╔╝ ███████║██║      ",
        "  ██╔══██╗██║   ██║  ╚██╔╝  ██╔══██║██║      ",
        "  ██║  ██║╚██████╔╝   ██║   ██║  ██║███████╗ ",
        "  ╚═╝  ╚═╝ ╚═════╝    ╚═╝   ╚═╝  ╚═╝╚══════╝ ",
        "",
        "   ██████╗██╗  ██╗███████╗███████╗███████╗   ",
        "  ██╔════╝██║  ██║██╔════╝██╔════╝██╔════╝   ",
        "  ██║     ███████║█████╗  ███████╗███████╗   ",
        "  ██║     ██╔══██║██╔══╝  ╚════██║╚════██║   ",
        "  ╚██████╗██║  ██║███████╗███████║███████║   ",
        "   ╚═════╝╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝   ",
        "",
        "         ♔  Terminal Edition  ♚              ",
    };

    const int numLines = static_cast<int>(sizeof(LINES) / sizeof(LINES[0]));
    for (int i = 0; i < numLines; i++) {
        int g = (i * 30) % 256;
        int b = 200 - (i * 10) % 200;
        std::cout << "\033[38;2;255;" << g << ";" << b << "m"
                  << BOLD << LINES[i] << RESET << "\n";
        std::cout.flush();
        Terminal::sleepMs(60);
    }
}

void Intro::drawParade() {
    // Both rows use the same outline glyphs — color does the work of
    // distinguishing the sides, just like on the board itself.
    static const std::string DARK[]  = {
        "♖","♘","♗","♕","♔","♗","♘","♖","  ",
        "♙","♙","♙","♙","♙","♙","♙","♙"
    };
    static const std::string LIGHT[] = {
        "♖","♘","♗","♕","♔","♗","♘","♖","  ",
        "♙","♙","♙","♙","♙","♙","♙","♙"
    };

    // Dark pieces on parchment ivory background
    std::cout << "\n  ";
    for (const auto& glyph : DARK) {
        std::cout << CLR_LIGHT_SQ << CLR_B_PIECE << BOLD << glyph << " " << RESET;
        std::cout.flush();
        Terminal::sleepMs(80);
    }

    // Light pieces on mahogany background
    std::cout << "\n  ";
    for (const auto& glyph : LIGHT) {
        std::cout << CLR_DARK_SQ << CLR_W_PIECE << BOLD << glyph << " " << RESET;
        std::cout.flush();
        Terminal::sleepMs(80);
    }

    std::cout << RESET << "\n\n";
}

void Intro::play() {
    Terminal::clearScreen();
    Terminal::hideCursor();

    drawTitle();

    std::cout << "\n";
    Terminal::sleepMs(200);
    Terminal::printCentered(
        "Full chess engine with Minimax AI & Alpha-Beta Pruning",
        60,
        CLR_INFO
    );
    std::cout << "\n";

    drawParade();

    Terminal::sleepMs(400);
    Terminal::showCursor();
}