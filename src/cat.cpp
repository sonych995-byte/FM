#include "../include/common.hpp"
#include "../include/utils.hpp"

void cmd_cat(const std::vector<std::string>& args) {

    if (args.size() != 2) {
        std::cout << "\n\nUsage: cat [filename]";
        pause();
        return;
    }

    try {
        std::ifstream file(args[1]);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file");
        }

        std::string line;
        while (std::getline(file, line)) {
            std::cout << line << std::endl;
        }

        file.close();
        show_success();
        return;

    } catch (const std::exception& e) {
        show_error(e);
        pause();
        return;
    }
}