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
- List contents of a directory or inspect a single file (`ls`)
- Print the current working directory (`pwd`)
- Run operating system commands with confirmation (`oscmd`)
- Quoted argument support (double and single quotes) for names and commands containing spaces
- Cross-platform screen clearing (Windows `cls` / Unix `clear`)
- Detailed built-in help command with syntax, examples, and output for every command
## Requirements
 
- A C++17 (or later) compatible compiler (e.g. g++, clang++)
## Project Structure
 
```
fm/
├── main.cpp
├── include/
│   ├── common.hpp
│   ├── utils.hpp
│   └── commands.hpp
└── src/
    ├── utils.cpp
    ├── cp.cpp
    ├── rn.cpp
    ├── rm.cpp
    ├── cd.cpp
    ├── mk.cpp
    ├── info.cpp
    ├── ls.cpp
    ├── pwd.cpp
    ├── oscmd.cpp
    └── help.cpp
```
 
## Building
 
**Linux / macOS**
```bash
g++ -std=c++17 -o fm main.cpp src/utils.cpp src/cp.cpp src/rn.cpp src/rm.cpp src/cd.cpp src/mk.cpp src/info.cpp src/ls.cpp src/pwd.cpp src/oscmd.cpp src/help.cpp
```
 
**Windows**
```bash
g++ -std=c++17 -o fm.exe main.cpp src/utils.cpp src/cp.cpp src/rn.cpp src/rm.cpp src/cd.cpp src/mk.cpp src/info.cpp src/ls.cpp src/pwd.cpp src/oscmd.cpp src/help.cpp
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
| `mk dir [name]` | Create a new directory (including intermediate directories) |
| `info [path]` | Show details about a file or folder (size, type, permissions, contents) |
| `ls [path]` | List contents of a directory, or inspect a single file (current directory if no path given) |
| `pwd` | Print the current working directory |
| `oscmd [command]` | Run an OS command (asks for confirmation; some commands can be dangerous) |
| `exit` | Exit the program |
 
Names containing spaces can be wrapped in double or single quotes, e.g. `mk file "my file.txt"` or `oscmd 'pkg install git'`.
 
## Example
 
```
=========================
 
file manager
 
=========================
 
current path: /home/user/project
 
Files:
[DIR] src
[FILE] main.cpp
[FILE] README.md
 
command: info main.cpp
```
 
## Notes
 
- `oscmd` executes raw OS commands via `std::system` and prompts for confirmation before running. Running `fm`, `./fm`, or `fm.exe` through `oscmd` is blocked to prevent recursive launching.
- Screen clearing works cross-platform: `cls` on Windows, `clear` on Linux/macOS.
## License
 
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
