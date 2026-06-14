# FM Command Reference (In-Depth)
 
This document explains every command in FM in detail, including the internal C++ implementation, edge cases, error handling, and notable behavior in the current version.
 
---
 
## General Notes
 
### Input Parsing - `split()`
 
Before any command runs, the raw input line is passed through `split()`, which:
 
- Splits on spaces, **except** inside quotes.
- Supports **both** double quotes (`"`) and single quotes (`'`).
- Quote types do not nest: a `'` inside a `"..."` block is treated as a literal character and vice versa, because each toggle checks the opposite flag (`!in_single_quotes` / `!in_double_quotes`).
- If a quote is opened but never closed, `split()` throws:
  - `std::runtime_error("Missing closing double quote")`
  - `std::runtime_error("Missing closing single quote")`
- These exceptions are caught in `main()`, printed via `show_error()`, and followed by `pause()`. The loop then `continue`s.
- Quote characters themselves are stripped from the resulting argument (e.g. `mk file "my file.txt"` → arg is `my file.txt`).
- Empty input (after parsing, `args.empty()`) is silently ignored and the loop redraws `home()`.
### Screen Clearing - `clear_screen()`
 
```cpp
void clear_screen() {
#ifdef _WIN32
    std::system("cls");
#else
    std::system("clear");
#endif
}
```
 
Called at the start of every main loop iteration and inside `cmd_info` and `cmd_help`. Replaces the previously hardcoded ANSI escape `\033[2J\033[H` with a cross-platform approach that works on Windows (`cls`) and Unix/macOS (`clear`).
 
### Helper Functions
 
- **`pause()`** — prints `"\n\nPress any button and enter to continue"`, then reads a full line via `std::getline(std::cin >> std::ws, temp)`. The `std::ws` skips leftover whitespace/newlines from previous input.
- **`show_success()`** — prints `"\n\nstatus: success\n\n"` then calls `pause()`.
- **`show_fail()`** — prints `"\n\nstatus: fail\n\n"` then calls `pause()`.
- **`show_error(e)`** — prints `"\n\nError:\n"` followed by `e.what()`. Does **not** call `pause()` itself; every call site must call `pause()` afterward.
### Main Loop Behavior
 
- Each iteration calls `clear_screen()` then `home()`, which prints the banner, current path, and a flat (non-recursive) directory listing with `[DIR]` / `[FILE]` prefixes.
- `home()` catches `fs::filesystem_error` if the directory iterator fails and prints the error inline without pausing.
- The prompt `"\ncommand: "` is shown and a line is read with `std::getline`.
- The command name is `args[0]`; dispatch is a chain of case-sensitive `if/else if` string comparisons.
- Anything not matching a known command prints `"\n\nUnknown command"` and pauses.
---
 
## Command Details
 
### 1. `cp [from] [to]` — Copy
 
**Function:** `cmd_cp`
 
- **Argument check:** requires exactly 3 tokens (`cp`, `from`, `to`). If not, prints `"\n\nUsage: cp [from] [to]"` and pauses.
- **String equality check:** if `from == to` as raw strings, calls `show_fail()` immediately.
- **Filesystem equivalence check:** if both paths exist and `fs::equivalent(from, to)` is true (i.e. they resolve to the same inode via different paths, symlinks, or relative/absolute forms), calls `show_fail()`. This check is new and more thorough than string comparison alone.
- **Copy operation:**
```cpp
  fs::copy(from, to,
      fs::copy_options::recursive |
      fs::copy_options::overwrite_existing);
```
  - `recursive`: copies directory contents completely.
  - `overwrite_existing`: silently overwrites `to` if it already exists.
- **On success:** `show_success()`.
- **On `fs::filesystem_error`:** `show_error(e)` then `pause()`.
---
 
### 2. `rn [oldname] [newname]` — Rename
 
**Function:** `cmd_rn`
 
- **Argument check:** requires exactly 3 tokens. If not, prints `"\n\nUsage: rn [oldname] [newname]"` and pauses.
- **Operation:** `fs::rename(oldname, newname)`.
  - Works on both files and directories. Can move across directories if `newname` includes a path.
- **On success:** `show_success()`.
- **On `fs::filesystem_error`:** `show_error(e)` then `pause()`.
**Notes:**
- No existence check before renaming; relies entirely on the exception from `fs::rename`.
- No same-name check; `rn a a` is attempted as-is (typically a no-op, possibly an error).
---
 
### 3. `rm [file/folder]` — Remove
 
**Function:** `cmd_rm`
 
- **Argument check:** requires exactly 2 tokens. If not, prints `"\n\nUsage: rm [file/folder]"` and pauses.
- **Existence check:** if `fs::exists(target)` is false, prints `"\n\nError: not found"`, pauses, and returns.
- **Confirmation prompt:**
```cpp
  std::getline(std::cin >> std::ws, conti);
```
  Accepts `"Y"` or `"y"` to proceed. Any other input (including plain Enter) cancels - the function returns **silently with no message**.
- **Deletion:** `fs::remove_all(target)`.
  - Recursively removes non-empty directories.
  - Return value (number of removed items) is **not checked** - success is assumed if no exception is thrown.
- **On success:** `show_success()`.
- **On `fs::filesystem_error`:** `show_error(e)` then `pause()`.
---
 
### 4. `cd [path]` — Change Directory
 
**Function:** `cmd_cd`
 
- **Argument check:** requires exactly 2 tokens. If not, prints `"\n\nUsage: cd [folder/path]"` and pauses.
- **Operation:** `fs::current_path(path)` — changes the process's working directory.
  - Accepts relative paths, absolute paths, and paths with spaces (if quoted in the input).
- **On success:** returns silently; the next loop iteration's `home()` shows the new directory.
- **On `fs::filesystem_error`:** `show_error(e)` then `pause()`.
**Notes:**
- No explicit check that the target is a directory (rather than a file); relies on the exception from `fs::current_path`.
---
 
### 5. `mk file [name]` / `mk dir [name]` — Make File or Directory
 
**Function:** `cmd_mk`
 
- **Argument check:** requires exactly 3 tokens. If not, prints `"\n\nUsage: mk [file/dir] [name]"` and pauses.
#### `mk file [name]`
 
- **Existence check:** if `fs::exists(name)` is true, prints `"\n\nstatus: fail (File already exists)"` and pauses (distinct from the generic `show_fail()` message).
- **Creation:** opens `std::ofstream file(name)`.
  - If open succeeds: closes the file (creating an empty file) and calls `show_success()`.
  - If open fails (e.g. invalid path, permission denied): calls `show_fail()` — **no error details are shown**.
#### `mk dir [name]`
 
- **Creation:** `fs::create_directories(name)`.
  - Creates all intermediate directories (`mkdir -p` behavior).
  - Returns `true` only if at least one new directory was created.
  - Returns `false` if the directory (and all its parents) already exist → `show_fail()` is called, indistinguishable from other failure types.
- **On `fs::filesystem_error`:** `show_error(e)` then `pause()`.
#### Invalid subcommand (neither `file` nor `dir`)
 
- Prints `"\n\nUsage: mk [file/dir] [name]"`, pauses, and returns.
---
 
### 6. `info [path]` — File/Folder Information
 
**Function:** `cmd_info`
 
- **Argument check:** requires exactly 2 tokens. If not, prints `"\n\nUsage: info [path]"` and pauses.
- **Existence check:** if `fs::exists(p)` is false, prints `"\n\nError: not found"`, pauses, and returns.
- **Display:** calls `clear_screen()` and prints a header `"==== INFO ===="`, followed by:
  - **Name:** `p.filename().string()`
  - **Path:** `fs::absolute(p).string()`
#### If `p` is a regular file
 
- **Type:** `File`
- **Extension:** `p.extension().string()` (includes the dot, e.g. `.cpp`; empty string if no extension)
- **Size:** `fs::file_size(p)` in bytes
- **Permission:** owner bits only, displayed as `rwx` or `-` for each:
```cpp
  auto perm = fs::status(p).permissions();
  // checks owner_read, owner_write, owner_exec
```
  Group and other bits are not shown.
 
#### If `p` is a directory
 
- **Type:** `Folder`
- **Total size:** sums `fs::file_size()` of all regular files via `fs::recursive_directory_iterator`. Per-entry exceptions are silently caught with `catch (...)` — broken symlinks or inaccessible files are simply skipped.
- **Inside (1 level):** non-recursive listing of immediate children, printed as `" - <filename>"`.
#### Error handling
 
- `fs::filesystem_error` in the try block: `show_error(e)` then `pause()`, then `return`. The unconditional `pause()` at the end of the function is not reached (fixed in this version).
- Symlinks are followed (`fs::status` not `fs::symlink_status`).
- Special files (sockets, FIFOs, etc.) fall through both the `is_regular_file` and `is_directory` branches, showing only Name and Path.
---
 
### 7. `ls [path]` — List Directory
 
**Function:** `cmd_ls`
 
- **Argument handling:**
  - 1 token (`ls`): target = `fs::current_path()`.
  - 2 tokens (`ls path`): target = the given path.
  - 3+ tokens: prints `"\n\nUsage: ls [path]"`, pauses, and returns.
- **Existence check:** if `fs::exists(target)` is false, prints `"\n\nError: path not found"`, pauses, and returns.
#### If target is a regular file (new behavior)
 
```cpp
if (fs::is_regular_file(target)) {
    std::cout << "\nListing: " << fs::absolute(target).string() << "\n\n";
    std::cout << "[FILE] " << target.filename().string() << '\n';
    pause();
    return;
}
```
 
Previously, calling `ls` on a file would throw a `filesystem_error`. Now it displays the file as a single `[FILE]` entry.
 
#### If target is a directory
 
- Prints `"\nListing: <absolute path>\n\n"`.
- Iterates `fs::directory_iterator(target)`, printing `"[DIR] "` or `"[FILE] "` followed by the filename.
- **On `fs::filesystem_error`:** `show_error(e)` then `pause()`, then `return` (no double-pause).
---
 
### 8. `pwd` — Print Working Directory
 
**Function:** `cmd_pwd`
 
- No argument check; extra tokens after `pwd` are silently ignored.
- Prints `"\n\ncurrent path: " << fs::current_path() << "\n\n"`.
- Calls `pause()`.
---
 
### 9. `oscmd [command]` — Run OS Command
 
**Function:** `cmd_oscmd`
 
- **Argument check:** requires exactly 2 tokens. If not, prints `"\n\nUsage: oscmd [command]"`, pauses, and returns. The missing `return` bug from the previous version is now fixed.
- **Self-launch guard:** blocks `"fm"`, `"./fm"`, and `"fm.exe"` (Windows), prints `"\n\ncannot launch fm inside fm"`, pauses, and returns. The missing `return` bug from the previous version is now fixed.
- **Confirmation prompt:**
```cpp
  std::cin >> conti;
```
  Uses `std::cin >>` (not `getline`) — reads only the first whitespace-delimited word. Accepts `"Y"` or `"y"`. Any other input cancels silently.
- **Execution:** `std::system(args[1].c_str())`.
  - Quote characters are stripped by FM's `split()` before reaching `std::system`. For example, `oscmd 'pkg install git'` passes `pkg install git` (no quotes) to the shell.
  - The exit code of `std::system` is **not checked**; `show_success()` is always called afterward.
**Security notes:**
- `std::system` spawns the OS default shell; any command the user can run in the terminal can be run here.
- `cd` inside `oscmd` runs in a child process and has **no effect** on FM's working directory.
- The self-launch guard only checks for the three hardcoded strings (`fm`, `./fm`, `fm.exe`); other paths to the executable are not blocked.
---
 
### 10. `help` — Detailed Help Screen
 
**Function:** `cmd_help`
 
- Calls `clear_screen()`, then prints a raw string literal (`R"(...)"`) with numbered entries (1–9) covering every command: `cp`, `rn`, `rm`, `cd`, `mk`, `info`, `ls`, `pwd`, `oscmd`.
- Each entry includes **Syntax**, **Example**, and **Output** sections.
- **Known artifact:** the `mk` example line contains a stray `"Program"` prefix in the source string:
```
       Program  - mk file src.cpp
```
  This appears to be a copy-paste leftover and has no functional effect.
- This is a hardcoded string — newly added commands must be manually added here.
---
 
### 11. `exit` — Quit
 
- Handled directly in `main()` via `break`.
- No confirmation, no cleanup. Immediate exit with `return 0`.
---
 
### 12. Unknown Commands
 
- Any `args[0]` not matching a known command prints `"\n\nUnknown command"` and calls `pause()`.
- Matching is exact and case-sensitive (`CP`, `Cp`, `Exit` etc. are all unknown).
