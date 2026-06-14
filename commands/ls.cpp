#include "../include/common.hpp"
#include "../include/utils.hpp"

void cmd_ls(const std::vector<std::string>& args) {

    fs::path target;

    // No argument means list the current working directory.
    if (args.size() == 1) {
        target = fs::current_path();
    }
    // ls [path]
    else if (args.size() == 2) {
        target = args[1];
    }
    else {
        std::cout << "\n\nUsage: ls [path]";
        pause();
        return;
    }

    // check exists
    if (!fs::exists(target)) {
        std::cout << "\n\nError: path not found";
        pause();
        return;
    }

    if (fs::is_regular_file(target)) {
        std::cout << "\nListing: " << fs::absolute(target).string() << "\n\n";
        std::cout << "[FILE] " << target.filename().string() << '\n';
        pause();
        return;
}

    try {
        std::cout << "\nListing: " << fs::absolute(target).string() << "\n\n";

        // Print one level of contents for folders.
        for (const auto& entry : fs::directory_iterator(target)) {
            
            if (entry.is_directory()) {
                std::cout << "[DIR] ";
            } else {
                std::cout << "[FILE] ";
            }
            
            std::cout << entry.path().filename().string() << '\n';
        }

    } catch (const fs::filesystem_error& e) {
        show_error(e);
        pause();
        return;
    }

    pause();
    return;
}