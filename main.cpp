#include <iostream>
#include <sstream>
#include <vector>
#include <filesystem>
#include <string>
#include <fstream>
#include <cstdlib>
#include <exception>

namespace fs = std::filesystem;

// ==================================================
// Utility Functions
// ==================================================

// Display current directory and file list
void home() {
    std::cout << "\033[2J\033[H";
    std::cout << "=========================\n\n";
    std::cout << "file manager\n\n";
    std::cout << "=========================\n\n";

    std::cout << "current path: " << fs::current_path() << "\n\n";

    std::cout << "Files:\n";

    fs::path path = ".";

    try {
        for (const auto& entry : fs::directory_iterator(path)) {

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

        // Double quote
        if (c == '"' && !in_single_quotes) {
            in_double_quotes = !in_double_quotes;
            continue;
        }

        // Single quote
        if (c == '\'' && !in_double_quotes) {
            in_single_quotes = !in_single_quotes;
            continue;
        }

        // Split only outside quotes
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

// ==================================================
// Commands
// ==================================================

// Copy file or directory
void cmd_cp(const std::vector<std::string>& args) {

    if (args.size() != 3) {
        std::cout << "\n\nUseage: cp [from] [to]";
        pause();
        return;
    }

    if (args[1] == args[2]) {
        show_fail();
        return;
    }

    try {
        fs::copy(
            args[1],
            args[2],
            fs::copy_options::recursive |
            fs::copy_options::overwrite_existing
        );

        show_success();
        return;

    } catch (const fs::filesystem_error& e) {
        show_error(e);
        pause();
        return;
    }
}

// Rename file or directory
void cmd_rn(const std::vector<std::string>& args) {

    if (args.size() != 3) {
        std::cout << "\n\nUseage: rn [oldname] [newname]";
        pause();
        return;
    }

    try {
        fs::rename(
            args[1],
            args[2]
        );

        show_success();
        return;

    } catch (const fs::filesystem_error& e) {
        show_error(e);
        pause();
        return;
    }
}

// Remove file or directory
void cmd_rm(const std::vector<std::string>& args) {

    if (args.size() != 2) {
        std::cout << "\n\nUsage: rm [file/folder]";
        pause();
        return;
    }

    // Check target exists
    if (!fs::exists(args[1])) {
        std::cout << "\n\nError: not found";
        pause();
        return;
    }

    std::string conti;

    std::cout << "\n\nContinue? [Y/n]: ";
    std::getline(std::cin >> std::ws, conti);

    // Cancel deletion
    if (conti != "Y" && conti != "y") {
        return;
    }

    try {

        fs::remove_all(args[1]);

        show_success();
        return;

    } catch (const fs::filesystem_error& e) {
        show_error(e);
        pause();
        return;
    }
}

// Change current working directory
void cmd_cd(const std::vector<std::string>& args) {

    if (args.size() != 2) {
        std::cout << "\n\nUseage: cd [folder/path]";
        pause();
        return;
    }

    try {

        fs::current_path(args[1]);

        return;

    } catch (const fs::filesystem_error& e) {
        show_error(e);
        pause();
        return;
    }
}

// Create file or directory
void cmd_mk(const std::vector<std::string>& args) {

    if (args.size() != 3) {
        std::cout << "\n\nUseage: mk [file/dir] [name]";
        pause();
        return;
    }

    // Create file
    if (args[1] == "file") {

        if (fs::exists(args[2])) {

            std::cout << "\n\nstatus: fail (File already exists)";
            pause();
            return;

        } else {

            std::ofstream file(args[2]);

            if (file.is_open()) {

                file.close();

                show_success();
                return;

            } else {

                show_fail();
                return;
            }
        }

    // Create directory
    } else if (args[1] == "dir") {

        try {

            bool status = fs::create_directories(args[2]);

            if (status) {
                show_success();
                return;
            } else {
                show_fail();
                return;
            }

        } catch (const fs::filesystem_error& e) {

            show_error(e);
            pause();
            return;
        }

    } else {

        std::cout << "\n\nUseage: mk [file/dir] [name]";
        pause();
    }
}

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
        std::cout << "\033[2J\033[H";
        std::cout << "\n\n==== INFO ====\n";
        std::cout << "Name: " << p.filename().string() << "\n";
        std::cout << "Path: " << fs::absolute(p).string() << "\n";

        if (fs::is_regular_file(p)) {

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
    }

    pause();
}

void cmd_ls(const std::vector<std::string>& args) {

    fs::path target;

    // no argument → current path
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

    try {
        std::cout << "\nListing: " << fs::absolute(target).string() << "\n\n";

        for (const auto& entry : fs::directory_iterator(target)) {

            // type
            if (entry.is_directory()) {
                std::cout << "[DIR]  ";
            } else {
                std::cout << "[FILE] ";
            }

            // name
            std::cout << entry.path().filename().string() << "\n";
        }

    } catch (const fs::filesystem_error& e) {
        show_error(e);
        pause();
    }

    pause();
}

void cmd_pwd() {
  std::cout << "\n\ncurrent path: " << fs::current_path() << "\n\n";
  pause();
}

void cmd_oscmd(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        std::cout << "\n\nUsage: oscmd [command]";
        pause();
    }

    if (args[1] == "./fm" || args[1] == "fm") {
        std::cout << "\n\ncannot launch fm inside fm";
        pause();
    }
    
    std::string conti;
    std::cout << "\n\nSome commands are dangerous";
    std::cout << "\nContinue? [Y/n]: ";
    std::cin >> conti;

    if (conti == "Y" || conti == "y") {
        std::system(args[1].c_str());
        show_success();
        pause();
    } else {
        return;
    }

}

void cmd_help() {
        std::cout << "\033[2J\033[H";
        std::cout << "\n\n" << R"(
        All Command:
        
        1. cp (copy)

        Syntax:
         - cp [from] [to]
        
        Example:
         - cp test.cpp main.cpp

        Output:
        Create a new file named [to] with contents belonging to [from].

        2. rn (rename)

        Syntax:
         - rn [oldname] [newname]

        Example:
         - rn test.cpp main.cpp

        Output:
        Rename the file [oldname] to [newname].

        3. rm (remove)

        Syntax:
         - rm [filename]

        Example:
         - rm useless.cpp

        Output:
        Delete the file named [filename].

        4. cd
        Syntax:
         - cd [path]

        Example:
         - cd storage/myfolder
         
        Output:
         Go to path [path]
         
        5. mk (make)
        
        Syntax:
         - mk file [name]
         - mk dir [name]
         
        Example:
       Program  - mk file src.cpp
         - mk dir myfolder
         
        Output:
        If the command is `mk file [name]`, it will create an empty file named [name]. If the command is `mk dir [name]`, it will create an empty folder named [name].
        
        6. info
        
        Syntax:
         - info [file name/dir name]
         
        Example:
         - info src
         
        Output:
        View detailed information about a file or folder
        
        7. ls (list)
        
        Syntax:
         - ls
         - ls [path]
         
        Example:
         - ls
         - ls storage/myfolder
         
        Output:
        List contents of a directory
        
        8. pwd
        
        Syntax:
         - pwd
         
        Example:
         - pwd
         
        Output:
        Print the current working directory
        
        9. oscmd
        
        Syntax:
         - oscmd [os command]
         
        Example:
         - oscmd 'pkg install git'
         
        Output:
        Use operating system commands, which require single or double quotes, as shown in the example.)";
        pause();
    }


// ==================================================
// Main 
// ==================================================

int main() {

    while (true) {
        std::cout << "\033[2J\033[H";
        home();

        std::string input;

        std::cout << "\ncommand: ";
        std::getline(std::cin, input);

        std::vector<std::string> args;

        // Parse command line
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

        // Command dispatcher
        if (args[0] == "cp") {

            cmd_cp(args);

        } else if (args[0] == "rn") {

            cmd_rn(args);

        } else if (args[0] == "rm") {

            cmd_rm(args);

        } else if (args[0] == "cd") {

            cmd_cd(args);

        } else if (args[0] == "mk") {

            cmd_mk(args);

        } else if (args[0] == "help") {

            cmd_help();

        } else if (args[0] == "exit") {

            break;

        } else if (args[0] == "info") {
          
          cmd_info(args);
          
        } else if (args[0] == "ls") {
          
          cmd_ls(args);
          
        } else if (args[0] == "pwd") {

          cmd_pwd();

        } else if (args[0] == "oscmd") {
            
          cmd_oscmd(args);

        } else {
            std::cout << "\n\nUnknown command";
            pause();
        }
    }

    return 0;
}
