#include "../include/common.hpp"
#include "../include/utils.hpp"

void cmd_help() {

    std::string help_text = R"(
Available commands:
  cp [source] [destination]   - Copy a file or directory
  rn [old_name] [new_name]     - Rename a file or directory
  mv [source] [destination]   - Move a file or directory
  rm [path]                    - Remove a file or directory
  cd [path]                    - Change the current directory
  mk [file/dir] [name]         - Create a file or directory
  info [path]                  - Display information about a file or directory
  ls [path]                    - List the contents of a directory
  cat [file]                   - Display the contents of a file
  find [path] [name]           - Search for a file or directory by name
  edit [tool] [file]           - Open a file with an external editor
  oscmd [command]              - Run an operating system command
  help                         - Display this help message
  exit                         - Exit the program
)";
    bridge.send(help_text, "response");
}
