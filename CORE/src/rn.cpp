#include "../include/common.hpp"
#include "../include/utils.hpp"

// Rename file or directory
void cmd_rn(const std::vector<std::string>& args) {

    if (args.size() != 3) {
        bridge.send("Usage: rn [from] [to]", "response");
        return;
    }

    try {
        fs::rename(
            args[1],
            args[2]
        );

        bridge.send("File or directory renamed successfully", "response");
        return;

    } catch (const fs::filesystem_error& e) {
        bridge.send("Error: Unable to rename file or directory", "response");
        return;
    }
}