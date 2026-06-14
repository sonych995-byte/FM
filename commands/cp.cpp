#include "../include/common.hpp"
#include "../include/utils.hpp"

// Copy file or directory
void cmd_cp(const std::vector<std::string>& args) {

    if (args.size() != 3) {
        std::cout << "\n\nUsage: cp [from] [to]";
        pause();
        return;
    }

    if (args[1] == args[2]) {
        show_fail();
        return;
    }

    if (fs::exists(args[1]) && fs::exists(args[2]) && fs::equivalent(args[1], args[2])) {
        show_fail();
        return;
    }

    try {
        fs::copy(
            args[1],
            args[2],
            fs::copy_options::recursive |
            fs::copy_options::overwrite_existing
        );

        show_success();
        return;

    } catch (const fs::filesystem_error& e) {
        show_error(e);
        pause();
        return;
    }
}