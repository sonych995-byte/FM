# Changelog
 
## v.0.5.0-alpha
 
### Changed
- Refactored entire project from a single `main.cpp` into a multi-file structure:
  - `src/utils.cpp` — utility functions (`home`, `split`, `pause`, `show_success`, `show_fail`, `show_error`, `clear_screen`)
  - `src/cp.cpp`, `src/rn.cpp`, `src/rm.cpp`, `src/cd.cpp`, `src/mk.cpp`, `src/info.cpp`, `src/ls.cpp`, `src/pwd.cpp`, `src/oscmd.cpp`, `src/help.cpp` — one file per command
  - `include/common.hpp` — shared includes (`iostream`, `string`, `vector`, `filesystem`, `fstream`, `exception`) and `namespace fs = std::filesystem`
  - `include/utils.hpp` — declarations for all utility functions
  - `include/commands.hpp` — declarations for all command functions
  - `main.cpp` — entry point only; includes headers and dispatches commands
- `main.cpp` is now minimal: only the main loop, `split()` call, and command dispatcher remain
- `exit` is now checked first in the dispatcher (before all other commands) with a standalone `if` instead of being embedded at the end of the `if/else if` chain
- Dispatcher reformatted from `} else if {` to `else if {` on a new line for consistency
## v.0.4.1-alpha
 
### Added
- `clear_screen()` utility function that calls `cls` on Windows and `clear` on other platforms via preprocessor (`#ifdef _WIN32`), replacing all hardcoded `\033[2J\033[H` ANSI escape sequences
- `cmd_cp` now uses `fs::equivalent()` to detect when `from` and `to` refer to the same file via different paths (e.g. symlinks or different relative/absolute forms), not just string equality
- `cmd_oscmd` self-launch guard extended to also block `"fm.exe"` (Windows executable name), in addition to `"fm"` and `"./fm"`
- `cmd_oscmd` argument check now has a proper `return` after the usage error message (fixes previous undefined behavior from missing `return`)
- `cmd_oscmd` self-launch guard now has a proper `return` after the warning (fixes previous fallthrough bug)
- `cmd_ls` now handles the case where `target` is a regular file: prints its name as `[FILE]` and pauses, instead of throwing a `filesystem_error`
- `cmd_mk` usage error for invalid subcommand now has a proper `return` (previously fell off the end of the function)
- `cmd_info` error path now has a proper `return` after `pause()` inside the catch block, fixing the double-pause bug
- `cmd_ls` error path now has a proper `return` after `pause()` inside the catch block, fixing the double-pause bug
- More inline comments added throughout explaining intent
### Changed
- `home()` no longer calls `clear_screen()` itself; the main loop calls `clear_screen()` then `home()` separately
- All `\033[2J\033[H` escape sequences replaced with `clear_screen()` calls for cross-platform compatibility
- Fixed all remaining `"Useage"` typos to `"Usage"` across all commands (`cp`, `rn`, `cd`, `mk`)
### Fixed
- `cmd_oscmd`: missing `return` after usage error — previously could read `args[1]` out of bounds
- `cmd_oscmd`: missing `return` after self-launch guard — previously the warning was shown but `std::system` could still be called
- `cmd_info`: double-pause on error path
- `cmd_ls`: double-pause on error path
- `cmd_ls`: calling `ls` on a regular file no longer throws a `filesystem_error`
## v.0.4.0-alpha
 
### Added
- New `oscmd [command]` command to run operating system commands via `std::system`, with a confirmation prompt
- Single-quote support in command parsing (in addition to double quotes)
- New detailed `cmd_help()` screen describing syntax, examples, and output for every command (replaces the short list)
- `oscmd` added to the help screen
### Changed
- `split()` rewritten to track double-quote and single-quote state separately, throwing distinct errors for unmatched quotes
- `help` command now calls `cmd_help()` and shows a full reference screen
## v.0.3.0-alpha
 
### Added
- New `ls [path]` command to list contents of a directory (defaults to current path if no argument given)
- New `pwd` command to print the current working directory
- `ls` and `pwd` added to the help menu
### Changed
- Main loop now clears the screen explicitly before calling `home()`
- Fixed typo "Unknown commamd" -> "Unknown command"
## v.0.2.0-alpha
 
### Added
- New `info [path]` command to display detailed information about a file or folder:
  - Name, absolute path, and type (File/Folder)
  - For files: extension, size in bytes, and owner permissions (r/w/x)
  - For folders: total size of all contents (recursive) and a one-level listing of contents
- `info` command added to the help menu
- Section comment headers and inline comments added throughout
### Changed
- `pause()` message changed from "Press Enter to continue" to "Press any button and enter to continue"
- Restored the `command: ` prompt before reading input in the main loop
- General code cleanup and formatting
## v.0.1.2-alpha
 
### Added
- Quoted argument support in command parsing (e.g. `mk file "my file.txt"`), allowing names with spaces
- Confirmation prompt (`Continue? [Y/n]`) before `rm` deletes a file or folder
- `rm` checks for existence before deletion; uses `fs::remove_all` for directory support
- `cp` same-source/destination check
- `mk file` existence check before creation
- Refactored command handling into separate functions: `cmd_cp`, `cmd_rn`, `cmd_rm`, `cmd_cd`, `cmd_mk`
- Shared helper functions: `pause()`, `show_success()`, `show_fail()`, `show_error()`
### Changed
- Replaced whitespace-only command splitting with a quote-aware parser
- `rm` usage changed from `rm [filename]` to `rm [file/dir]` (now supports folders)
- Standardized all status messages and "Press Enter to continue" prompts
### Fixed
- Input handling using `std::getline` now properly skips leading whitespace (`std::cin >> std::ws`)
- Inconsistent spacing/newlines in status and usage messages
## v.0.1.0
 
### Added
- Basic interactive file manager shell
- `cp [from] [to]` — copy files/directories recursively, overwrite existing
- `rm [filename]` — remove a file
- `rn [oldname] [newname]` — rename a file or directory
- `cd [path]` — change current directory
- `help` — show list of available commands
- `exit` — quit the program
- Directory listing on startup with `[DIR]` / `[FILE]` prefixes
