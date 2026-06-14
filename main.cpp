#include "include/common.hpp"
#include "include/utils.hpp"
#include "include/commands.hpp"

int main() {

    while (true) {

        clear_screen();
        home();

        std::string input;

        std::cout << "\ncommand: ";
        std::getline(std::cin, input);

        std::vector<std::string> args;

        try {

            args = split(input);

        } catch (const std::exception& e) {

            show_error(e);
            pause();
            continue;
        }

        if (args.empty()) {
            continue;
        }

        if (args[0] == "exit") {
            break;
        }

        if (args[0] == "cp") {
            cmd_cp(args);
        }
        else if (args[0] == "rn") {
            cmd_rn(args);
        }
        else if (args[0] == "rm") {
            cmd_rm(args);
        }
        else if (args[0] == "cd") {
            cmd_cd(args);
        }
        else if (args[0] == "mk") {
            cmd_mk(args);
        }
        else if (args[0] == "info") {
            cmd_info(args);
        }
        else if (args[0] == "ls") {
            cmd_ls(args);
        }
        else if (args[0] == "pwd") {
            cmd_pwd();
        }
        else if (args[0] == "help") {
            cmd_help();
        }
        else if (args[0] == "oscmd") {
            cmd_oscmd(args);
        }
        else {

            std::cout << "\n\nUnknown command";
            pause();
        }
    }

    return 0;
}