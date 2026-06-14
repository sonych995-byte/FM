#include "../include/common.hpp"
#include "../include/utils.hpp"

void cmd_info(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        std::cout << "\n\nUsage: info [path]";
        pause();
        return;
    }

    fs::path p = args[1];

    if (!fs::exists(p)) {
        std::cout << "\n\nError: not found";
        pause();
        return;
    }

    try {
        clear_screen();
        std::cout << "\n\n==== INFO ====\n";
        std::cout << "Name: " << p.filename().string() << "\n";
        std::cout << "Path: " << fs::absolute(p).string() << "\n";

        if (fs::is_regular_file(p)) {

            // File-specific metadata.
            std::cout << "Type: File\n";
            std::cout << "Extension: " << p.extension().string() << "\n";
            std::cout << "Size: " << fs::file_size(p) << " bytes\n";

            auto perm = fs::status(p).permissions();
            std::cout << "Permission: "
                      << ((perm & fs::perms::owner_read)  != fs::perms::none ? "r" : "-")
                      << ((perm & fs::perms::owner_write) != fs::perms::none ? "w" : "-")
                      << ((perm & fs::perms::owner_exec)  != fs::perms::none ? "x" : "-")
                      << "\n";
        }
        else if (fs::is_directory(p)) {

            // Folder-specific metadata, including a recursive size estimate.
            std::cout << "Type: Folder\n";

            uintmax_t total_size = 0;

            for (auto& entry : fs::recursive_directory_iterator(p)) {
                try {
                    if (fs::is_regular_file(entry.status())) {
                        total_size += fs::file_size(entry.path());
                    }
                } catch (...) {}
            }

            std::cout << "Total size: " << total_size << " bytes\n";

            std::cout << "\nInside (1 level):\n";
            for (auto& entry : fs::directory_iterator(p)) {
                std::cout << " - " << entry.path().filename().string() << "\n";
            }
        }

    } catch (const fs::filesystem_error& e) {
        show_error(e);
        pause();
        return;
    }

    pause();
    return;
}