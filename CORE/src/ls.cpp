#include "../include/common.hpp"
#include "../include/utils.hpp"

void cmd_ls(const std::vector<std::string>& args) {

    fs::path target;

    // No argument means list the current working directory.
    if (args.size() == 1) {
        target = fs::current_path();
    }
    // ls [path]
    else if (args.size() == 2) {
        target = args[1];
    }
    else {
        bridge.send("Usage: ls [path]", "response");
        return;
    }

    try {
        if (!fs::exists(target)) {
            bridge.send("Error: Path does not exist", "response");
            return;
        }
        if (fs::is_regular_file(target)) {
            bridge.send("Error: Path is not a directory", "response");
            return;
        }
            std::string output;
            for (const auto& entry : fs::directory_iterator(target)) {
                output += (entry.is_directory() ? "[DIR] " : "[FILE] ");
                output += entry.path().filename().string() + "\n";
            }
            bridge.send(output, "response");
    } catch (const std::exception& e) {
        bridge.send(std::string("Error: Unable to list directory contents: ") + e.what(), "response");
    }
}