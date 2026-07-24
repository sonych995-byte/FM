#include "../include/common.hpp"
#include "../include/utils.hpp"

// =====================================
// Search function
// =====================================

bool search_file(
    const fs::path& path,

    const std::string& target,

    int& count
) {
    bool found = false;

    for (const auto& entry : fs::directory_iterator(path)) {

        if (entry.is_directory()) {

            found |= search_file(entry.path(), target, count);

        } else if (entry.is_regular_file()) {

            if (entry.path().filename() == target) {

                bridge.send(entry.path().string(), "response");
                count++;
                found = true;
            }
        }
    }
    return found;
}

void cmd_find(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        bridge.send("Usage: find [path] [name]", "response");
        return;
    }
    try {
        const fs::path root = args[1] == "." ? fs::current_path() : fs::path(args[1]);
        if (!fs::exists(root)) {
            bridge.send("Error: Path does not exist", "response");
            return;
        }
        std::string output;
        for (const auto& entry : fs::recursive_directory_iterator(
                 root, fs::directory_options::skip_permission_denied)) {
            if (entry.path().filename() == args[2]) {
                output += entry.path().string() + "\n";
            }
        }
        bridge.send(output.empty() ? "No matches found" : output, "response");
    } catch (const std::exception& e) {
        bridge.send(std::string("Error: ") + e.what(), "response");
    }
}