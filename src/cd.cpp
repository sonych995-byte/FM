#include "../include/common.hpp"
#include "../include/utils.hpp"

// Change current working directory
void cmd_cd(const std::vector<std::string>& args) {

    if (args.size() != 2) {
        std::cout << "\n\nUsage: cd [folder/path]";
        pause();
        return;
    }

    try {

        // Change the process working directory for the next commands.
        fs::current_path(args[1]);

        return;

    } catch (const fs::filesystem_error& e) {
        show_error(e);
        pause();
        return;
    }
}