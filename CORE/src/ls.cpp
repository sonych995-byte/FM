#include "../include/common.hpp"
#include "../include/utils.hpp"

void cmd_ls(const std::vector<std::string>& args) {

    fs::path target;

    if (args.size() == 1) {
        target = fs::current_path();
    }
    else if (args.size() == 2) {
        target = args[1];
    }
    else {
        bridge.send("Usage: ls [path]", "response");
        return;
    }

    try {

        if (!fs::exists(target) && !fs::is_symlink(target)) {
            bridge.send("Error: Path does not exist", "response");
            return;
        }

        if (!fs::is_directory(target)) {
            bridge.send("Error: Path is not a directory", "response");
            return;
        }

        std::string output;

        for (const auto& entry : fs::directory_iterator(target)) {

            auto status = fs::symlink_status(entry.path());

            if (fs::is_symlink(status)) {
                output += "[LINK] ";
            }
            else if (fs::is_directory(status)) {
                output += "[DIR] ";
            }
            else if (fs::is_regular_file(status)) {
                output += "[FILE] ";
            }
            else {
                output += "[OTHER] ";
            }

            output += entry.path().filename().string();
            output += "\n";
        }

        bridge.send(output, "response");

    }
    catch (const fs::filesystem_error& e) {

        bridge.send(
            std::string("Filesystem error: ") + e.what(),
            "response"
        );

    }
    catch (const std::exception& e) {

        bridge.send(
            std::string("Error: ") + e.what(),
            "response"
        );
    }
}