#include "../include/common.hpp"
#include "../include/utils.hpp"

// Remove file or directory
void cmd_rm(const std::vector<std::string>& args) {

    if (args.size() != 2) {
        std::cout << "\n\nUsage: rm [file/folder]";
        pause();
        return;
    }

    // Refuse to delete a path that does not exist.
    if (!fs::exists(args[1])) {
        std::cout << "\n\nError: not found";
        pause();
        return;
    }

    std::string conti;

    std::cout << "\n\nContinue? [Y/n]: ";
    std::getline(std::cin >> std::ws, conti);

    // Only continue when the user confirms the destructive action.
    if (conti != "Y" && conti != "y") {
        return;
    }

    try {

        fs::remove_all(args[1]);

        show_success();
        return;

    } catch (const fs::filesystem_error& e) {
        show_error(e);
        pause();
        return;
    }
}