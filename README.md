# FM - Simple Command-Line File Manager
 
FM is a lightweight, terminal-based file manager written in C++ using `std::filesystem`. It provides a simple interactive shell for browsing directories and performing basic file operations.
 
## Features
 
- Display current directory contents (files and folders)
- Copy files or directories (`cp`)
- Remove files or directories with confirmation (`rm`)
- Rename files or directories (`rn`)
- Change directory (`cd`)
- Create new files or directories (`mk`)
- View detailed information about a file or folder (`info`)
- List contents of a directory (`ls`)
- Print the current working directory (`pwd`)
- Run operating system commands with confirmation (`oscmd`)
- Quoted argument support (double and single quotes) for names and commands containing spaces
- Detailed built-in help command with syntax, examples, and output for every command
## Requirements
 
- A C++17 (or later) compatible compiler (e.g. g++, clang++)
## Building
 
```bash
g++ -std=c++17 -o fm main.cpp
```
 
## Usage
 
Run the compiled program:
 
```bash
./fm
```
 
You will see the current directory and its contents, followed by a command prompt.
 
### Available Commands
 
| Command | Description |
|---|---|
| `help` | Show detailed help with syntax and examples for all commands |
| `cp [from] [to]` | Copy a file or directory (recursive, overwrites existing) |
| `rm [file/dir]` | Remove a file or directory (asks for confirmation) |
| `rn [oldname] [newname]` | Rename a file or directory |
| `cd [path]` | Change the current directory |
| `mk file [name]` | Create a new empty file |
| `mk dir [name]` | Create a new directory |
| `info [path]` | Show details about a file or folder (size, type, permissions, contents) |
| `ls [path]` | List contents of a directory (current directory if no path given) |
| `pwd` | Print the current working directory |
| `oscmd [command]` | Run an OS command (asks for confirmation; some commands can be dangerous) |
| `exit` | Exit the program |
 
Names containing spaces can be wrapped in double or single quotes, e.g. `mk file "my file.txt"` or `oscmd 'pkg install git'`.
 
## Example
 
```
=========================
 
file manager (v.
 
=========================
 
current path: /home/user/project
 
Files:
[DIR] src
[FILE] main.cpp
[FILE] README.md
 
command: oscmd 'ls -la'
```
 
## Notes
 
- `oscmd` executes raw OS commands via `std::system` and will prompt for confirmation before running, as some commands can be destructive. Running `fm` or `./fm` through `oscmd` is blocked.
## License
 
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
 
