#include "../include/common.hpp"
#include "../include/utils.hpp"

// Rename file or directory
void cmd_rn(const std::vector<std::string>& args) {

    if (args.size() != 3) {
        std::cout << "\n\nUsage: rn [oldname] [newname]";
        pause();
        return;
    }

    try {
        fs::rename(
            args[1],
            args[2]
        );

        show_success();
        return;

    } catch (const fs::filesystem_error& e) {
        show_error(e);
        pause();
        return;
    }
}