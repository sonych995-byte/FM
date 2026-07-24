#include "../include/common.hpp"
#include "../include/utils.hpp"
#include <cctype>

// ==================================================
// Utility Functions
// ==================================================

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
        if (std::isspace(static_cast<unsigned char>(c)) && !in_double_quotes && !in_single_quotes) {

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