#include "../include/common.hpp"
#include "../include/utils.hpp"

// Remove file or directory
void cmd_rm(const std::vector<std::string>& args) {

    if (args.size() != 2 && args.size() != 3) {
        bridge.send("Usage: rm [path] [Y]", "response");
        return;
    }

    try {
        if (!fs::exists(args[1])) {
            bridge.send("Error: Path does not exist", "response");
            return;
        }
        if (args.size() == 2) {
            bridge.send("Error: confirmation required", "response");
            return;
        }
        if (args[2] != "Y" && args[2] != "y") {
            bridge.send("Delete cancelled", "response");
            return;
        }

        fs::remove_all(args[1]);
        bridge.send("File or directory removed successfully", "response");
        return;

    } catch (const fs::filesystem_error& e) {
        bridge.send("Error: Unable to remove file or directory", "response");
        return;
    }
}