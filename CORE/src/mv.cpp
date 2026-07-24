#include "../include/common.hpp"
#include "../include/utils.hpp"


void cmd_mv(const std::vector<std::string>& args) {
    
    if (args.size() != 3) {
        bridge.send("Usage: mv [from] [to]", "response");
        return;
    }

    try {

        fs::rename(args[1], args[2]);

        bridge.send("File/directory moved successfully", "response");

        return;

    } catch (const fs::filesystem_error& e) {
        bridge.send("Error: Unable to move file or directory", "response");
        return;
    }
}