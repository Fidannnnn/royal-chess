#include "game/Game.hpp"
#include "renderer/Intro.hpp"
#include "PipeMode.hpp"

#include <cstdlib>
#include <ctime>
#include <sys/stat.h>
#include <unistd.h>

//  MAIN
//
//  Detects whether stdin is a pipe or a terminal and branches accordingly.
//
//  Pipe / file redirect (teacher's test script):
//    PipeMode::run() reads moves, applies legal ones, skips illegal ones,
//    and prints the final board state + result in the format expected by
//    test-level.sh.
//
//  Terminal (normal play):
//    Plays the animated intro and runs the interactive game menu.
//
//  Detection: fstat on STDIN_FILENO — S_ISFIFO or S_ISREG means pipe/file.

static bool stdinIsPipe() {
    struct stat st;
    if (fstat(STDIN_FILENO, &st) != 0) return false;
    return S_ISFIFO(st.st_mode) || S_ISREG(st.st_mode);
}

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    if (stdinIsPipe()) {
        return PipeMode::run();
    }

    Intro::play();
    Game game;
    game.run();
    return 0;
}