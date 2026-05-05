#include "Terminal.hpp"

#include <iostream>
#include <string>
#include <thread>
#include <chrono>

// =============================================================================
//  TERMINAL  —  Implementation
// =============================================================================

// -- Screen control -----------------------------------------------------------

void Terminal::clearScreen() {
    // ESC[2J  — erase entire display
    // ESC[H   — move cursor to home position (row 1, col 1)
    std::cout << "\033[2J\033[H";
}

void Terminal::moveCursor(int row, int col) {
    // ESC[row;colH — cursor position (both values are 1-based)
    std::cout << "\033[" << row << ";" << col << "H";
}

void Terminal::hideCursor() {
    // ESC[?25l — hide cursor (DEC private mode 25 off)
    // Called at the start of any animated sequence to prevent flicker
    std::cout << "\033[?25l";
}

void Terminal::showCursor() {
    // ESC[?25h — show cursor (DEC private mode 25 on)
    // Must always be called before the program exits, otherwise the terminal
    // is left with an invisible cursor for the rest of the session
    std::cout << "\033[?25h";
}

// -- Timing -------------------------------------------------------------------

void Terminal::sleepMs(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// -- Text output --------------------------------------------------------------

void Terminal::printCentered(const std::string& text,
                              int width,
                              std::string_view color) {
    // Left-pad only — right padding isn't needed for single-line centered output
    int pad = (width - static_cast<int>(text.size())) / 2;
    if (pad > 0) std::cout << std::string(pad, ' ');

    if (!color.empty()) std::cout << color;
    std::cout << text;
    if (!color.empty()) std::cout << RESET;
}