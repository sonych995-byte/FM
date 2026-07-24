#include "../include/common.hpp"
#include "../include/utils.hpp"

void cmd_info(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        bridge.send("Usage: info [path]", "response");
        return;
    }

    try {
        fs::path p = args[1];
        if (!fs::exists(p)) {
            bridge.send("Error: Path does not exist", "response");
            return;
        }
        std::string info = "Path: " + fs::absolute(p).string() + "\n";
        if (fs::is_regular_file(p)) {
            info += "Type: File\nSize: " + std::to_string(fs::file_size(p)) + " bytes\n";
        } else if (fs::is_directory(p)) {
            info += "Type: Directory\n";
        } else {
            info += "Type: Other\n";
        }
        const auto file_time = fs::last_write_time(p);
        const auto system_time = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            file_time - decltype(file_time)::clock::now() + std::chrono::system_clock::now());
        const std::time_t cftime = std::chrono::system_clock::to_time_t(system_time);
        info += "Last Modified: " + std::string(std::asctime(std::localtime(&cftime)));
        bridge.send(info, "response");
    } catch (const std::exception& e) {
        bridge.send(std::string("Error: ") + e.what(), "response");
    }
}