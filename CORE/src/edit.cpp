#include "../include/common.hpp"
#include "../include/utils.hpp"
#include <algorithm>

void cmd_edit(const std::vector<std::string>& args) {

    if (args.size() != 3) {

        bridge.send(
            "response",
            "Usage: edit [tool] [filename]"
        );

        return;
    }


    std::string tool = args[1];
    std::string filename = args[2];


    try {

        std::vector<std::string> allowed = {
            "nano",
            "vim",
            "gedit",
            "code"
        };


        if (std::find(
                allowed.begin(),
                allowed.end(),
                tool
            ) == allowed.end()) {

            throw std::runtime_error(
                "Editor not allowed"
            );
        }


        if (!fs::exists(filename)) {

            throw std::runtime_error(
                "File does not exist"
            );
        }


        if (filename.find('"') != std::string::npos || filename.find('\n') != std::string::npos) {
            throw std::runtime_error("Filename contains unsupported characters");
        }
        std::string command = tool + " \"" + filename + "\"";


        int result =
            system(command.c_str());


        if (result != 0) {

            throw std::runtime_error(
                "Failed to open editor"
            );
        }


    }
    catch (const std::exception& e) {

        bridge.send(e.what(), "response");

    }
}