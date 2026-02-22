#include <iostream>
#include <string>
#include "interface/uci.h"
#include "interface/cli.h"

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::string mode = argv[1];
        if (mode == "uci" || mode == "UCI") {
            UCI uci;
            uci.run();
            return 0;
        }
        if (mode == "cli" || mode == "CLI") {
            CLI cli;
            cli.run();
            return 0;
        }
    }

    std::cout << "Nyx Chess Engine 1.0" << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "  nyx uci      - Run in UCI mode (for GUIs)" << std::endl;
    std::cout << "  nyx cli      - Run in command-line mode" << std::endl;
    std::cout << std::endl;
    std::cout << "UCI mode is used by chess GUIs like Arena, Fritz, ChessBase, CuteChess." << std::endl;
    std::cout << "CLI mode provides an interactive command-line interface." << std::endl;

    return 0;
}
