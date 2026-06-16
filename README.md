# FM - Simple Command-Line File Manager
 
FM is a lightweight, terminal-based file manager written in C++ using `std::filesystem`. It provides a simple interactive shell for browsing directories and performing basic file operations.
 
## Features
 
- Display current directory contents (files and folders)
- Copy files or directories (`cp`)
- Remove files or directories with confirmation (`rm`)
- Rename files or directories (`rn`)
- Move files or directories (`mv`)
- Change directory (`cd`)
- Create new files or directories (`mk`)
- View file contents (`cat`)
- Search for files recursively by name (`find`)
- Open a file with an external editor (`edit`)
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
    ├── mv.cpp
    ├── cd.cpp
    ├── mk.cpp
    ├── cat.cpp
    ├── find.cpp
    ├── edit.cpp
    ├── info.cpp
    ├── ls.cpp
    ├── pwd.cpp
    ├── oscmd.cpp
    └── help.cpp
```
 
## Building
 
**Linux / macOS**
```bash
g++ -std=c++17 -o fm main.cpp src/utils.cpp src/cp.cpp src/rn.cpp src/rm.cpp src/mv.cpp src/cd.cpp src/mk.cpp src/cat.cpp src/find.cpp src/edit.cpp src/info.cpp src/ls.cpp src/pwd.cpp src/oscmd.cpp src/help.cpp
```
 
**Windows**
```bash
g++ -std=c++17 -o fm.exe main.cpp src/utils.cpp src/cp.cpp src/rn.cpp src/rm.cpp src/mv.cpp src/cd.cpp src/mk.cpp src/cat.cpp src/find.cpp src/edit.cpp src/info.cpp src/ls.cpp src/pwd.cpp src/oscmd.cpp src/help.cpp
```
 
**Termux (Android)**
```bash
pkg install clang
clang++ -std=c++17 -o fm main.cpp src/utils.cpp src/cp.cpp src/rn.cpp src/rm.cpp src/mv.cpp src/cd.cpp src/mk.cpp src/cat.cpp src/find.cpp src/edit.cpp src/info.cpp src/ls.cpp src/pwd.cpp src/oscmd.cpp src/help.cpp
```
 
## Running `fm` From Anywhere
 
After building the binary, you can install it so the `fm` command works in any directory, without typing `./fm` or the full path.
 
### Linux / macOS / WSL
 
Move the compiled binary to a directory that's already in your `PATH`, such as `/usr/local/bin`:
 
```bash
sudo mv fm /usr/local/bin/fm
sudo chmod +x /usr/local/bin/fm
```
 
Now you can run `fm` from any folder:
 
```bash
fm
```
 
Alternatively, without `sudo`, create a personal bin folder and add it to `PATH`:
 
```bash
mkdir -p ~/bin
mv fm ~/bin/fm
echo 'export PATH="$HOME/bin:$PATH"' >> ~/.bashrc   # or ~/.zshrc on macOS
source ~/.bashrc
```
 
### Termux (Android)
 
Termux already has `$PREFIX/bin` in `PATH`, so just move the binary there:
 
```bash
mv fm $PREFIX/bin/fm
chmod +x $PREFIX/bin/fm
```
 
Then run from anywhere with:
 
```bash
fm
```
 
### Windows
 
1. Build `fm.exe` as shown above.
2. Create a folder for your tools, e.g. `C:\tools`, and copy `fm.exe` into it.
3. Add that folder to your `PATH`:
   - Press `Win + R`, type `sysdm.cpl`, press Enter.
   - Go to **Advanced** → **Environment Variables**.
   - Under **User variables**, select `Path` → **Edit** → **New**, then add `C:\tools`.
   - Click OK on all dialogs.
4. Open a new Command Prompt / PowerShell window and run:
```bash
fm
```
 
> Tip: Verify the install by running `fm` from any directory (e.g. your Desktop). If the file manager starts up, the installation worked correctly.
 
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
| `rn [oldname] [newname]` | Rename a file or directory |
| `mv [source] [destination]` | Move or rename a file or directory |
| `rm [file/folder]` | Remove a file or directory (asks for confirmation) |
| `cat [filename]` | Display the contents of a text file |
| `find [path] [filename]` | Search recursively for files by name |
| `edit [tool] [filename]` | Open a file with an external editor tool |
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
- `find /` searches the entire filesystem (all drives on Windows, root on Unix/macOS). Use with caution on large systems.
- `edit` requires an external editor tool (e.g. `nano`, `vim`, `code`) to be available in your `PATH`.

## License
 
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
EOF
