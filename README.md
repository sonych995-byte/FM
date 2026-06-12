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
- Quoted argument support for names containing spaces
- Built-in help command

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
| `help` | Show the list of available commands |
| `cp [from] [to]` | Copy a file or directory (recursive, overwrites existing) |
| `rm [file/dir]` | Remove a file or directory (asks for confirmation) |
| `rn [oldname] [newname]` | Rename a file or directory |
| `cd [path]` | Change the current directory |
| `mk file [name]` | Create a new empty file |
| `mk dir [name]` | Create a new directory |
| `info [path]` | Show details about a file or folder (size, type, permissions, contents) |
| `exit` | Exit the program |

Names containing spaces can be wrapped in double quotes, e.g. `mk file "my file.txt"`.

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

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
