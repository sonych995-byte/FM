# FM Command Reference (In-Depth)
 
This document explains every command in FM in detail, including the internal C++ implementation, source file location, edge cases, error handling, and notable behavior in the current version.
 
---
 
## Project Structure
 
Since v0.5.0-alpha, the codebase is split into multiple files:
 
```
fm/
├── main.cpp               Entry point and command dispatcher
├── include/
│   ├── common.hpp         Shared includes and namespace alias (fs)
│   ├── utils.hpp          Declarations for utility functions
│   └── commands.hpp       Declarations for all command functions
└── src/
    ├── utils.cpp          home(), split(), pause(), show_success(), show_fail(), show_error(), clear_screen()
    ├── cp.cpp             cmd_cp()
    ├── rn.cpp             cmd_rn()
    ├── rm.cpp             cmd_rm()
    ├── cd.cpp             cmd_cd()
    ├── mk.cpp             cmd_mk()
    ├── info.cpp           cmd_info()
    ├── ls.cpp             cmd_ls()
    ├── pwd.cpp            cmd_pwd()
    ├── oscmd.cpp          cmd_oscmd()
    └── help.cpp           cmd_help()
```
 
Each command `.cpp` file includes `../include/common.hpp` and `../include/utils.hpp`. `main.cpp` includes `include/common.hpp`, `include/utils.hpp`, and `include/commands.hpp`.
 
---
 
## General Notes
 
### Input Parsing — `split()` (`src/utils.cpp`)
 
Before any command runs, the raw input line is passed through `split()`, which:
 
- Splits on spaces, **except** inside quotes.
- Supports **both** double quotes (`"`) and single quotes (`'`).
- Quote types do not nest: a `'` inside a `"..."` block is treated as a literal character, because each toggle checks the opposite flag (`!in_single_quotes` / `!in_double_quotes`).
- If a quote is opened but never closed, `split()` throws:
  - `std::runtime_error("Missing closing double quote")`
  - `std::runtime_error("Missing closing single quote")`
- These exceptions are caught in `main()`, printed via `show_error()`, followed by `pause()`, and the loop `continue`s.
- Quote characters themselves are stripped from the resulting argument (e.g. `mk file "my file.txt"` → arg is `my file.txt`).
- Empty input (after parsing, `args.empty()`) is silently ignored; the loop redraws `home()`.
### Screen Clearing — `clear_screen()` (`src/utils.cpp`)
 
```cpp
void clear_screen() {
#ifdef _WIN32
    std::system("cls");
#else
    std::system("clear");
#endif
}
```
 
Called at the start of every main loop iteration and inside `cmd_info` and `cmd_help`. Works on Windows (`cls`) and Unix/macOS (`clear`).
 
### Helper Functions (`src/utils.cpp`)
 
- **`home()`** — prints the banner, current path (`fs::current_path()`), and a flat `[DIR]`/`[FILE]` listing of the current directory. Catches `fs::filesystem_error` and prints it inline without pausing.
- **`pause()`** — prints `"\n\nPress any button and enter to continue"`, then reads a full line via `std::getline(std::cin >> std::ws, temp)`. The `std::ws` skips leftover newlines from previous input.
- **`show_success()`** — prints `"\n\nstatus: success\n\n"` then calls `pause()`.
- **`show_fail()`** — prints `"\n\nstatus: fail\n\n"` then calls `pause()`.
- **`show_error(e)`** — prints `"\n\nError:\n"` followed by `e.what()`. Does **not** call `pause()` itself; every call site must do so.
### Main Loop (`main.cpp`)
 
- Calls `clear_screen()` then `home()` at the start of each iteration.
- Reads input with `std::getline`, parses with `split()`.
- `exit` is checked first with a standalone `if`, then all other commands follow in an `if/else if` chain.
- Unknown commands print `"\n\nUnknown command"` and call `pause()`.
- Dispatch is case-sensitive and exact-match only (`CP`, `Exit`, etc. are all unknown).
---
 
## Command Details
 
### 1. `cp [from] [to]` — Copy
 
**Source:** `src/cp.cpp`
 
- **Argument check:** requires exactly 3 tokens. If not, prints `"\n\nUsage: cp [from] [to]"` and pauses.
- **String equality check:** if `from == to` (raw string comparison), calls `show_fail()` immediately.
- **Filesystem equivalence check:** if both paths exist and `fs::equivalent(from, to)` returns true (same inode via different paths, symlinks, or relative/absolute forms), calls `show_fail()`. More thorough than string comparison alone.
- **Copy operation:**
```cpp
  fs::copy(from, to,
      fs::copy_options::recursive |
      fs::copy_options::overwrite_existing);
```
  - `recursive`: copies directory trees completely.
  - `overwrite_existing`: silently overwrites `to` if it already exists.
- **On success:** `show_success()`.
- **On `fs::filesystem_error`:** `show_error(e)` then `pause()`.
**Notes:**
- No explicit check that `from` exists before calling `fs::copy`; the underlying exception handles it.
- Copying a directory into itself (where paths are not string-equal but not yet filesystem-equivalent) may throw a `filesystem_error` caught by the generic handler.
---
 
### 2. `rn [oldname] [newname]` — Rename
 
**Source:** `src/rn.cpp`
 
- **Argument check:** requires exactly 3 tokens. If not, prints `"\n\nUsage: rn [oldname] [newname]"` and pauses.
- **Operation:** `fs::rename(oldname, newname)`.
  - Works on both files and directories.
  - Can move a file across directories if `newname` includes a path.
- **On success:** `show_success()`.
- **On `fs::filesystem_error`:** `show_error(e)` then `pause()`.
**Notes:**
- No existence check; relies entirely on the exception from `fs::rename`.
- `rn a a` is attempted as-is — typically a no-op or an error depending on the OS.
---
 
### 3. `rm [file/folder]` — Remove
 
**Source:** `src/rm.cpp`
 
- **Argument check:** requires exactly 2 tokens. If not, prints `"\n\nUsage: rm [file/folder]"` and pauses.
- **Existence check:** if `fs::exists(target)` is false, prints `"\n\nError: not found"`, pauses, and returns.
- **Confirmation prompt:**
```cpp
  std::getline(std::cin >> std::ws, conti);
```
  Accepts `"Y"` or `"y"` to proceed. Any other input (including plain Enter) cancels — the function returns **silently with no message**.
- **Deletion:** `fs::remove_all(target)`.
  - Recursively removes non-empty directories.
  - Return value (number of removed items) is **not checked**; success is assumed if no exception is thrown.
- **On success:** `show_success()`.
- **On `fs::filesystem_error`:** `show_error(e)` then `pause()`.
---
 
### 4. `cd [path]` — Change Directory
 
**Source:** `src/cd.cpp`
 
- **Argument check:** requires exactly 2 tokens. If not, prints `"\n\nUsage: cd [folder/path]"` and pauses.
- **Operation:** `fs::current_path(path)` — changes the process's working directory. Takes effect on the next iteration's `home()` call.
- **On success:** returns silently.
- **On `fs::filesystem_error`:** `show_error(e)` then `pause()`.
**Notes:**
- No explicit check that the target is a directory; relies on the exception from `fs::current_path`.
- `cd ..` and absolute paths both work as expected.
---
 
### 5. `mk file [name]` / `mk dir [name]` — Make File or Directory
 
**Source:** `src/mk.cpp`
 
- **Argument check:** requires exactly 3 tokens. If not, prints `"\n\nUsage: mk [file/dir] [name]"`, pauses, and returns.
#### `mk file [name]`
 
- **Existence check:** if `fs::exists(name)` is true, prints `"\n\nstatus: fail (File already exists)"` and pauses (distinct wording from `show_fail()`).
- **Creation:** opens `std::ofstream file(name)`.
  - If open succeeds: closes the file (creating an empty file) and calls `show_success()`.
  - If open fails (invalid path, permission denied): calls `show_fail()` — **no error details are shown**.
- Parent directories are **not** created automatically; if the parent doesn't exist, the ofstream simply fails.
#### `mk dir [name]`
 
- **Creation:** `fs::create_directories(name)` — creates all intermediate directories (`mkdir -p` behavior).
  - Returns `true` if at least one new directory was created → `show_success()`.
  - Returns `false` if the full path already exists → `show_fail()` (indistinguishable from other failures).
- **On `fs::filesystem_error`:** `show_error(e)` then `pause()`.
#### Invalid subcommand
 
- Prints `"\n\nUsage: mk [file/dir] [name]"`, pauses, and returns.
---
 
### 6. `info [path]` — File/Folder Information
 
**Source:** `src/info.cpp`
 
- **Argument check:** requires exactly 2 tokens. If not, prints `"\n\nUsage: info [path]"` and pauses.
- **Existence check:** if `fs::exists(p)` is false, prints `"\n\nError: not found"`, pauses, and returns.
- **Display:** calls `clear_screen()` then prints `"==== INFO ===="` followed by:
  - **Name:** `p.filename().string()`
  - **Path:** `fs::absolute(p).string()`
#### If `p` is a regular file
 
- **Type:** `File`
- **Extension:** `p.extension().string()` (includes dot; empty string if no extension)
- **Size:** `fs::file_size(p)` in bytes
- **Permission:** owner bits only (`owner_read`, `owner_write`, `owner_exec`), displayed as `r`/`w`/`x` or `-`. Group and other bits are not shown.
#### If `p` is a directory
 
- **Type:** `Folder`
- **Total size:** recursive sum of `fs::file_size()` for all regular files via `fs::recursive_directory_iterator`. Per-entry exceptions are silently swallowed with `catch (...)`.
- **Inside (1 level):** immediate children listed as `" - <filename>"` via `fs::directory_iterator`.
#### Error handling
 
- `fs::filesystem_error` in the try block: `show_error(e)` → `pause()` → `return`. The final unconditional `pause()` is not reached on error.
- Symlinks are followed (`fs::status`, not `fs::symlink_status`).
- Special files (sockets, FIFOs, devices) fall through both `is_regular_file` and `is_directory` checks, showing only Name and Path.
---
 
### 7. `ls [path]` — List Directory
 
**Source:** `src/ls.cpp`
 
- **Argument handling:**
  - 1 token (`ls`): target = `fs::current_path()`.
  - 2 tokens (`ls path`): target = given path.
  - 3+ tokens: prints `"\n\nUsage: ls [path]"`, pauses, and returns.
- **Existence check:** if `fs::exists(target)` is false, prints `"\n\nError: path not found"`, pauses, and returns.
#### If target is a regular file
 
```cpp
if (fs::is_regular_file(target)) {
    std::cout << "\nListing: " << fs::absolute(target).string() << "\n\n";
    std::cout << "[FILE] " << target.filename().string() << '\n';
    pause();
    return;
}
```
 
Displays the file as a single `[FILE]` entry instead of throwing a `filesystem_error`.
 
#### If target is a directory
 
- Prints `"\nListing: <absolute path>\n\n"`.
- Iterates `fs::directory_iterator(target)`, printing `"[DIR] "` or `"[FILE] "` followed by the filename.
- **On `fs::filesystem_error`:** `show_error(e)` → `pause()` → `return`.
---
 
### 8. `pwd` — Print Working Directory
 
**Source:** `src/pwd.cpp`
 
- No argument check; extra tokens after `pwd` are silently ignored (the function takes no parameters).
- Prints `"\n\ncurrent path: " << fs::current_path() << "\n\n"`.
- Calls `pause()`.
---
 
### 9. `oscmd [command]` — Run OS Command
 
**Source:** `src/oscmd.cpp`
 
- **Argument check:** requires exactly 2 tokens. If not, prints `"\n\nUsage: oscmd [command]"`, pauses, and **returns** (bug in previous version where `return` was missing has been fixed).
- **Self-launch guard:** blocks `"fm"`, `"./fm"`, and `"fm.exe"`. Prints `"\n\ncannot launch fm inside fm"`, pauses, and **returns** (bug in previous version where `return` was missing has been fixed).
- **Confirmation prompt:**
```cpp
  std::cin >> conti;
```
  Uses `std::cin >>` (not `getline`) — reads only the first whitespace-delimited word. Accepts `"Y"` or `"y"`; any other input cancels silently.
- **Execution:** `std::system(args[1].c_str())`.
  - FM's `split()` strips the quote characters before this point, so `oscmd 'pkg install git'` passes `pkg install git` (no quotes) to the system shell.
  - The exit code from `std::system` is **not checked**; `show_success()` is always called.
**Security notes:**
- `std::system` spawns the OS default shell — any command the current user can run in the terminal can be run here.
- `cd` inside `oscmd` runs in a child process and has **no effect** on FM's own working directory.
- The self-launch guard only matches the three hardcoded strings; other paths to the binary (e.g. `../fm`, `/usr/bin/fm`) are not blocked.
- The use of `std::cin >>` for the confirmation prompt (rather than `getline`) may leave a newline in the input buffer, which `std::ws` in subsequent `pause()` calls will consume.
---
 
### 10. `help` — Detailed Help Screen
 
**Source:** `src/help.cpp`
 
- Calls `clear_screen()`, then prints a large raw string literal (`R"(...)"`) containing numbered descriptions (1–9) for every command: `cp`, `rn`, `rm`, `cd`, `mk`, `info`, `ls`, `pwd`, `oscmd`.
- Each entry includes **Syntax**, **Example**, and **Output** sections.
- **Known artifact:** the `mk` example line contains a stray `"Program"` prefix:
```
         Program  - mk file src.cpp
```
  This is a copy-paste leftover with no functional effect.
- The help text is a hardcoded string — any new command added to the dispatcher must also be manually added here.
---
 
### 11. `exit` — Quit
 
**Source:** `main.cpp` (dispatcher)
 
- Checked first in the main loop dispatcher with a standalone `if` (not part of the `else if` chain).
- Breaks out of the `while (true)` loop. `main()` returns `0`.
- No confirmation, no cleanup.
---
 
### 12. Unknown Commands
 
**Source:** `main.cpp` (dispatcher)
 
- Any `args[0]` not matching a known command prints `"\n\nUnknown command"` and calls `pause()`.
- Matching is exact and case-sensitive.
---
 
## Building (Multi-file)
 
**Linux / macOS**
```bash
g++ -std=c++17 -o fm main.cpp src/utils.cpp src/cp.cpp src/rn.cpp src/rm.cpp src/cd.cpp src/mk.cpp src/info.cpp src/ls.cpp src/pwd.cpp src/oscmd.cpp src/help.cpp
```
 
**Windows**
```bash
g++ -std=c++17 -o fm.exe main.cpp src/utils.cpp src/cp.cpp src/rn.cpp src/rm.cpp src/cd.cpp src/mk.cpp src/info.cpp src/ls.cpp src/pwd.cpp src/oscmd.cpp src/help.cpp
```
