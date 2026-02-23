#include <iostream>
#include <string>
#include "interface/uci.h"
#include "interface/cli.h"

int main(int argc, char* argv[]) {
    std::string mode = "uci";  // Default to UCI mode
    
    if (argc > 1) {
        mode = argv[1];
    }
    
    // Convert to lowercase for comparison
    for (auto& c : mode) {
        c = std::tolower(c);
    }
    
    if (mode == "cli") {
        CLI cli;
        cli.run();
        return 0;
    }
    
    // Default to UCI mode (for GUIs)
    UCI uci;
    uci.run();
    return 0;
}
