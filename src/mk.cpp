#include "../include/common.hpp"
#include "../include/utils.hpp"

// Create file or directory
void cmd_mk(const std::vector<std::string>& args) {

    if (args.size() != 3) {
        std::cout << "\n\nUsage: mk [file/dir] [name]";
        pause();
        return;
    }

    // Create an empty file when the subcommand is "file".
    if (args[1] == "file") {

        if (fs::exists(args[2])) {

            std::cout << "\n\nstatus: fail (File already exists)";
            pause();
            return;

        } else {

            std::ofstream file(args[2]);

            if (file.is_open()) {

                file.close();

                show_success();
                return;

            } else {

                show_fail();
                return;
            }
        }

    // Create a directory tree when the subcommand is "dir".
    } else if (args[1] == "dir") {

        try {

            bool status = fs::create_directories(args[2]);

            if (status) {
                show_success();
                return;
            } else {
                show_fail();
                return;
            }

        } catch (const fs::filesystem_error& e) {

            show_error(e);
            pause();
            return;
        }

    } else {

        std::cout << "\n\nUsage: mk [file/dir] [name]";
        pause();
        return;
    }
}