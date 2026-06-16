#include "../include/common.hpp"
#include "../include/utils.hpp"

void cmd_edit(const std::vector<std::string>& args) {

    if (args.size() != 3) {

        std::cout
            << "\n\nUsage: edit [tool] [filename]";

        pause();

        return;
    }

    std::string tool = args[1];

    std::string filename = args[2];

    try {

        if (!fs::exists(filename)) {

            throw std::runtime_error(
                "File not found"
            );
        }

        std::string command =

            tool + " \"" +

            filename + "\"";

        int result =

            system(command.c_str());

        if (result != 0) {

            throw std::runtime_error(

                "Failed to open file"

            );
        }

        show_success();

    }

    catch (const std::exception& e) {

        show_error(e);

        pause();
    }
}