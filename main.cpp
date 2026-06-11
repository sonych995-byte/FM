
#include <iostream>
#include <sstream>
#include <vector>
#include <filesystem>
#include <string>

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
      std::cout << "rm [filenane]\n";
      std::cout << "rn [oldname] [newname]\n";
      std::cout << "cd [path]\n";
      std::cout << "exit\n";
      std::cout << "\nPress enter to continue";
      std::cin.get();

      
    } else if (command.starts_with("cp")) {
      /*
      cp (copy)
      */
      std::vector<std::string> args = split(command);

      if (args.size() != 3) {
        std::cout << "Usege: cp [from] [to]\n";
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
        std::cout << "\nstatus: success\n\n";
        std::cout << "Press enter to continue";
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
        std::cout << "Usege: rm [filename]\n";
        std::cout << "\n Press enter to continue";
        std::cin.get();
        continue;
        
      }
      try {
        
        fs::remove(args[1]);
        std::cout << "\nstatus: success\n\n";
        std::cout << "Press enter to coutinue";
        std::cin.get();
          
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
        std::cout << "Usege: rm [oldname] [newname]\n";
        std::cout << "\n Press enter to continue";
        std::cin.get();
        continue;
      }
      try {
        fs::rename(
          args[1],
          args[2]
        );
        std::cout << "\nstatus: success\n\n";
        std::cout << "Press enter to coutinue";
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
        std::cout << "Usege: cd [path]\n";
        std::cout << "\n Press enter to continue";
        std::cin.get();
        continue;
      }
      try {
        
        fs::current_path(args[1]);
        
      } catch (const fs::filesystem_error& e) {
        std::cout << "Error:\n";
        std::cout << e.what();
        std::cout << "\nPress Enter to continue";
        std::cin.get();
      }
    } else if (command == "exit") {
      break;
    } else {
      std::cout << "unknow command";
      std::cout << "\n\nPress Enter to continue";
      std::cin.get();
    }
    }
  return 0;
}
