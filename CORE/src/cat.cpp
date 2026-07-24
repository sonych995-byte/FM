#include "../include/common.hpp"
#include "../include/utils.hpp"

void cmd_cat(const std::vector<std::string>& args) {

    if (args.size() != 2) {
        bridge.send("Usage: cat [filename]", "response");
        return;
    }

    try {
        std::ifstream file(args[1]);
        if (!file.is_open()) {
            bridge.send("Error: Unable to open file", "response");
            return;
        }

        std::string line;
        std::string output;
        while (std::getline(file, line)) {
            output += line + '\n';
        }

        file.close();
        bridge.send(output, "response");
        return;

    } catch (const std::exception& e) {
        bridge.send("Error: Unable to display file contents", "response");
        return;
    }
}