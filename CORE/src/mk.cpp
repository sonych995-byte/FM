#include "../include/common.hpp"
#include "../include/utils.hpp"

// Create file or directory
void cmd_mk(const std::vector<std::string>& args) {

    if (args.size() != 3) {
        bridge.send("Usage: mk [file/dir] [name]", "response");
        return;
    }

    // Create an empty file when the subcommand is "file".
    if (args[1] == "file") {

        if (fs::exists(args[2])) {

            bridge.send("Error: File already exists", "response");
            return;

        } else {

            std::ofstream file(args[2]);

            if (file.is_open()) {

                file.close();

                bridge.send("File created successfully", "response");
                return;

            } else {

                bridge.send("Error: Unable to create file", "response");
                return;
            }
        }

    // Create a directory tree when the subcommand is "dir".
    } else if (args[1] == "dir") {

        try {

            bool status = fs::create_directories(args[2]);

            if (status) {
                bridge.send("Directory created successfully", "response");
                return;
            } else {
                bridge.send("Error: Unable to create directory", "response");
                return;
            }

        } catch (const fs::filesystem_error& e) {

            bridge.send("Error: Unable to create directory", "response");
            return;
        }

    } else {

        bridge.send("Usage: mk [file/dir] [name]", "response");
        return;
    }
}