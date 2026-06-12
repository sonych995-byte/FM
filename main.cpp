#include <iostream>
#include <sstream>
#include <vector>
#include <filesystem>
#include <string>
#include <fstream>
#include <exception>
namespace fs = std::filesystem;

// util

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
            if (entry.is_directory()) {
    std::cout << "[DIR] ";
}
else {
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

std::vector<std::string> split(const std::string& text) {
    std::vector<std::string> result;

    std::string current;
    bool in_quotes = false;

    for (char c : text) {

        if (c == '"') {
            in_quotes = !in_quotes;
            continue;
        }

        if (c == ' ' && !in_quotes) {

            if (!current.empty()) {
                result.push_back(current);
                current.clear();
            }

        } else {
            current += c;
        }
    }

    if (in_quotes) {
        throw std::runtime_error("Missing closing quote");
    }

    if (!current.empty()) {
        result.push_back(current);
    }

    return result;
  }

void pause() {
    std::cout << "\n\nPress Enter to continue";

    std::string temp;
    std::getline(std::cin >> std::ws, temp);
}

void show_success() {
  std::cout << "\n\nstatus: success" << "\n\n";
  pause();
}

void show_fail() {
  std::cout << "\n\nstatus: fail" << "\n\n";
  pause();
}

void show_error(const std::exception& e) {
    std::cout << "\n\nError:\n";
    std::cout << e.what();
}

// commamd

void cmd_cp(const std::vector<std::string>& args ) {
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

void cmd_rm(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        std::cout << "\n\nUsage: rm [file/folder]";
        pause();
        return;
    }

    if (!fs::exists(args[1])) {
        std::cout << "\n\nError: not found";
        pause();
        return;
    }

    std::string conti;

    std::cout << "\n\nContinue? [Y/n]: ";

    std::getline(std::cin >> std::ws, conti);

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

void cmd_mk(const std::vector<std::string>& args) {
  if (args.size() != 3) {
    std::cout << "\n\nUseage: mk [file/dir] [name]";
    pause();
    return;
  }
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

int main() {

    while (true) {
    home();

    std::string input;
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
      std::cout << "\n\nAll command\n";
      std::cout << "cp [from] [to]\n";
      std::cout << "rn [oldname] [newname]\n";
      std::cout << "rm [file/dir]\n";
      std::cout << "cd [path/full path]\n";
      std::cout << "mk [file/dir] [name]\n";
      std::cout << "exit";
      pause();
    } else if (args[0] == "exit") {
      break;
    } else {
      std::cout << "\n\nUnknown command";
      pause();
    }
    }
  return 0;
}
