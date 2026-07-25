#include "../include/common.hpp"
#include "../include/utils.hpp"

// Copy file or directory
void cmd_cp(const std::vector<std::string>& args) {

    if (args.size() != 3) {
        bridge.send(
            "Usage: cp [source] [destination]",
            "response"
        );
        return;
    }

    if (args[1] == args[2]) {
        bridge.send(
            "Source and destination are the same",
            "response"
        );
        return;
    }

    try {
        if (fs::exists(args[1]) && fs::exists(args[2]) && fs::equivalent(args[1], args[2])) {
            bridge.send("Source and destination are the same", "response");
            return;
        }
        fs::copy(
            args[1],
            args[2],
            fs::copy_options::recursive |
            fs::copy_options::overwrite_existing
        );

        bridge.send("File copied successfully", "response");
        return;

    } catch (const fs::filesystem_error& e) {
        bridge.send("Error copying file", "response");
        return;
    }
}