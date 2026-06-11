
#include <iostream>
#include <sstream>
#include <vector>
#include <filesystem>
#include <string>
#include <fstream>

namespace fs = std::filesystem;

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

  std::istringstream iss(text);
  std::string word;

  while (iss >> word) {
    result.push_back(word);
  }
  return result;
}

int main() {

    while (true) {
    home();

    std::string command;

    std::cout << "\ncommand: ";
    std::getline(std::cin, command);

    if (command == "help") {
      /*
      help
      */
      std::cout << "\033[2J\033[H";
      std::cout << "all command:\n";
      std::cout << "cp [from] [to]\n";
      std::cout << "rm [filename]\n";
      std::cout << "rn [oldname] [newname]\n";
      std::cout << "cd [path]\n";
      std::cout << "mk [file/dir] [name]";
      std::cout << "exit\n";
      std::cout << "\nPress enter to continue";
      std::cin.get();

      
    } else if (command.starts_with("cp")) {
      /*
      cp (copy)
      */
      std::vector<std::string> args = split(command);

      if (args.size() != 3) {
        std::cout << "Usage: cp [from] [to]\n";
        std::cout << "\n Press enter to continue";
        std::cin.get();
        continue;
        
      }
      try {
        fs::copy(
        args[1],
        args[2],
        fs::copy_options::recursive |
        fs::copy_options::overwrite_existing
    );
          std::cout << "\n\nstatus: success";
          std::cout << "\n\nPress Enter to coutinue";
          std::cin.get();
        
      } catch (const fs::filesystem_error& e) {
        std::cout << "Error:\n";
        std::cout << e.what();
        std::cout << "\nPress Enter to continue";
        std::cin.get();
      }
      
    } else if (command.starts_with("rm")) {
      /*
      rm (remove)
      */
      std::vector<std::string> args = split(command);

      if (args.size() != 2) {
        std::cout << "Usage: rm [filename]\n";
        std::cout << "\n Press enter to continue";
        std::cin.get();
        continue;
        
      }
      try {
        
        bool status = fs::remove(args[1]);
        if (status) {
          std::cout << "\n\nstatus: success";
          std::cout << "\n\nPress Enter to coutinue";
          std::cin.get();
          
        } else {
          std::cout << "\n\nstatus: fail";
          std::cout << "\n\nPress Enter to coutinue";
          std::cin.get();
        }
        
          
      } catch (const fs::filesystem_error& e) {
        std::cout << "Error:\n";
        std::cout << e.what();
        std::cout << "\nPress Enter to continue";
        std::cin.get();
      }
    } else if (command.starts_with("rn")) {
      /*
      rn (rename)
      */
      std::vector<std::string> args = split(command);

      if (args.size() != 3) {
        std::cout << "Usage: rm [oldname] [newname]\n";
        std::cout << "\n Press enter to continue";
        std::cin.get();
        continue;
      }
      try {
        fs::rename(
          args[1],
          args[2]
        );
          std::cout << "\n\nstatus: success";
          std::cout << "\n\nPress Enter to coutinue";
          std::cin.get();
        
      } catch (const fs::filesystem_error& e) {
        std::cout << "Error:\n";
        std::cout << e.what();
        std::cout << "\nPress Enter to continue";
        std::cin.get();
      }
    } else if (command.starts_with("cd")) {
      /*
      cd
      */
      std::vector<std::string> args = split(command);

      if (args.size() != 2) {
        std::cout << "Usage: cd [path]\n";
        std::cout << "\n Press enter to continue";
        std::cin.get();
        continue;
      }
      try {
        
        fs::current_path(args[1]);
        
      } catch (const fs::filesystem_error& e) {
        std::cout << "\nError:\n";
        std::cout << e.what();
        std::cout << "\nPress Enter to continue";
        std::cin.get();
      }
    } else if (command == "exit") {
      break;
    } else if (command.starts_with("mk")) {
      /*
      mk (make)
      */
      std::vector<std::string> args = split(command);
      if (args.size() != 3) {
        std::cout << "Usage: mk [file/dir] [name]\n";
        std::cout << "\n Press enter to continue";
        std::cin.get();
        continue;
      }
      if (args[1] == "file") {
        std::ofstream file(args[2]);
        if (file.is_open()) {
          file.close();
          std::cout << "\n\nstatus: success";
          std::cout << "\n\nPress Enter to coutinue";
          std::cin.get();
          
        } else {
          std::cout << "\n\nstatus: fail";
          std::cout << "\n\nPress Enter to coutinue";
          std::cin.get();
        }
      } else if (args[1] == "dir") {
        try {
          bool status = fs::create_directories(args[2]);
          if (status) {
          std::cout << "\n\nstatus: success";
          std::cout << "\n\nPress Enter to coutinue";
          std::cin.get();
          
        } else {
          std::cout << "\n\nstatus: fail (Directory already exists)";
          std::cout << "\n\nPress Enter to coutinue";
          std::cin.get();
          }
        } catch (fs::filesystem_error& e) {
          std::cout << "\nError:\n";
          std::cout << e.what();
          std::cout << "\nPress Enter to continue";
          std::cin.get();
        }
      } else {
        std::cout << "Usage: mk [file/dir] [name]\n";
        std::cout << "\nPress Enter to continue";
        std::cin.get();
      }
    } else {
      std::cout << "unknown command";
      std::cout << "\n\nPress Enter to continue";
      std::cin.get();
    }
    }
  return 0;
}
