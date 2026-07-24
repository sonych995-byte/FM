#include "../include/common.hpp"
#include "../include/utils.hpp"

void cmd_pwd() {
    try {

        std::string current_path = fs::current_path().string();

        bridge.send(current_path, "response");

    } catch (const fs::filesystem_error& e) {
        bridge.send("Error: Unable to get current working directory", "response");
        return;
    }
  return;
}