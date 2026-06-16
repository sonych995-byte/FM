#include "../include/common.hpp"
#include "../include/utils.hpp"

// ==================================================
// Utility Functions
// ==================================================

void home() {
    std::cout << "=========================\n\n";
    std::cout << "file manager\n\n";
    std::cout << "=========================\n\n";

    std::cout << "current path: " << fs::current_path() << "\n\n";

    std::cout << "Files:\n";

    fs::path path = ".";

    std::vector<fs::path> file_list;

    int pointer = 0;

    try {
        for (const auto& entry : fs::directory_iterator(path)) {

            file_list.push_back(entry);

            // Show item type
            if (entry.is_directory()) {
                std::cout << "[DIR] ";
            } else {
                std::cout << "[FILE] ";
            }

            std::cout << entry.path().filename() << '\n';
        }
    }
    catch (const fs::filesystem_error& e) {
        std::cout << "Error:\n";
        std::cout << e.what();
    }
}

// Split command line into arguments
// Supports quoted strings:
// cd "my folder"
std::vector<std::string> split(const std::string& text) {

    std::vector<std::string> result;

    std::string current;

    bool in_double_quotes = false;
    bool in_single_quotes = false;

    for (char c : text) {

        // Toggle double-quote mode so spaces inside quotes stay attached.
        if (c == '"' && !in_single_quotes) {
            in_double_quotes = !in_double_quotes;
            continue;
        }

        // Toggle single-quote mode so spaces inside quotes stay attached.
        if (c == '\'' && !in_double_quotes) {
            in_single_quotes = !in_single_quotes;
            continue;
        }

        // Split only on spaces that are not wrapped in quotes.
        if (c == ' ' && !in_double_quotes && !in_single_quotes) {

            if (!current.empty()) {
                result.push_back(current);
                current.clear();
            }

        } else {
            current += c;
        }
    }

    if (in_double_quotes) {
        throw std::runtime_error("Missing closing double quote");
    }

    if (in_single_quotes) {
        throw std::runtime_error("Missing closing single quote");
    }

    if (!current.empty()) {
        result.push_back(current);
    }

    return result;
}

// Wait for user input
void pause() {
    std::cout << "\n\nPress any button and enter to continue";

    std::string temp;
    std::getline(std::cin >> std::ws, temp);
}

// Success message
void show_success() {
    std::cout << "\n\nstatus: success\n\n";
    pause();
}

// Failure message
void show_fail() {
    std::cout << "\n\nstatus: fail\n\n";
    pause();
}

// Display exception message
void show_error(const std::exception& e) {
    std::cout << "\n\nError:\n";
    std::cout << e.what();
}

void clear_screen() {
#ifdef _WIN32
    std::system("cls");
#else
    std::system("clear");
#endif
}