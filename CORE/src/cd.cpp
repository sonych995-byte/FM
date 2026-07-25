#include "../include/common.hpp"
#include "../include/utils.hpp"

// Change current working directory
void cmd_cd(const std::vector<std::string>& args) {

    if (args.size() != 2) {
        bridge.send("Usage: cd [folder/path]", "response");
        return;
    }

    try {

        fs::current_path(args[1]);

        // Send current path back to GUI
        bridge.send(
            "PATH:" + fs::current_path().string(),
            "response"
        );

        return;

    } catch (const fs::filesystem_error& e) {

        bridge.send(
            "Error: Unable to change directory",
            "response"
        );

        return;
    }
}