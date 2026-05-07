#include "game/Game.hpp"
#include "renderer/Intro.hpp"

#include <cstdlib>
#include <ctime>

// =============================================================================
//  MAIN
//
//  Entry point. Deliberately thin — all logic lives in Game and its
//  dependencies. main() does only three things:
//    1. Seed the RNG (not currently used, but good practice for any future
//       randomized features like opening book selection)
//    2. Play the animated intro screen
//    3. Hand off to Game::run() which owns the menu and game loop
// =============================================================================

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    Intro::play();

    Game game;
    game.run();

    return 0;
}