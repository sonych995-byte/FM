#include "../include/common.hpp"
#include "../include/utils.hpp"

void cmd_oscmd(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        std::cout << "\n\nUsage: oscmd [command]";
        pause();
        return;
    }

    // Prevent the app from spawning itself recursively.
    if (args[1] == "fm" || args[1] == "./fm" || args[1] == "fm.exe") {
        std::cout << "\n\ncannot launch fm inside fm";
        pause();
        return;
    }
    
    std::string conti;
    std::cout << "\n\nSome commands are dangerous";
    std::cout << "\nContinue? [Y/n]: ";
    std::cin >> conti;

    if (conti == "Y" || conti == "y") {
        // Run the requested OS command only after explicit confirmation.
        std::system(args[1].c_str());
        show_success();
        return;
    } else {
        return;
    }

}