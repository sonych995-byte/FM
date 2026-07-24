#include "../include/common.hpp"
#include "../include/utils.hpp"

void cmd_oscmd(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        bridge.send("Usage: oscmd [command]", "response");
        return;
    }

    // Prevent the app from spawning itself recursively.
    if (args[1] == "fm" || args[1] == "./fm" || args[1] == "fm.exe" ||
        args[1] == "core" || args[1] == "./core" || args[1] == "core.exe") {
        bridge.send("Error: Cannot launch FM inside FM", "response");
        return;
    }
    
    try {
        int result = std::system(args[1].c_str());

        if (result != 0) {
            bridge.send("Error: Command failed with exit code " + std::to_string(result), "response");
        } else {
            bridge.send("Command executed successfully", "response");
        }

    } catch (const std::exception& e) {
        bridge.send("Error: Exception occurred while executing command", "response");
        return;
    }

}