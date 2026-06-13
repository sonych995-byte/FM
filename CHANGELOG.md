# Changelog
 
## [v.0.4.0-alpha]
 
### Added
- New `oscmd [command]` command (`cmd_oscmd`) to run operating system commands via `std::system`, with a confirmation prompt before execution
- Single-quote support in command parsing (in addition to double quotes), required for `oscmd` arguments
- New detailed `cmd_help()` screen describing syntax, examples, and output for every command (replaces the short command list)
- `oscmd` added to the help screen
### Changed
- `split()` rewritten to track double-quote and single-quote state separately, throwing distinct errors for unmatched double or single quotes
- `help` command now calls `cmd_help()` and shows a full reference screen instead of a short list
 
## [v.0.3.0-alpha]
 
### Added
- New `ls [path]` command (`cmd_ls`) to list contents of a directory (defaults to current path if no argument given)
- New `pwd` command (`cmd_pwd`) to print the current working directory
- `ls` and `pwd` added to the `help` menu
### Changed
- Main loop now clears the screen explicitly before calling `home()`
- Fixed typo "Unknown commamd" -> "Unknown command"

## V.0.2.0 alpha

### Added
- New `info [path]` command (`cmd_info`) to display detailed information about a file or folder:
  - Name, absolute path, and type (File/Folder)
  - For files: extension, size in bytes, and owner permissions (r/w/x)
  - For folders: total size of all contents (recursive) and a one-level listing of contents
- `info` command added to the `help` menu
- Section comment headers added throughout the file (Utility Functions, Commands, Main Program)
- Inline comments explaining logic in `home()`, `split()`, and each command function

### Changed
- `pause()` message changed from "Press Enter to continue" to "Press any button and enter to continue"
- Restored the `command: ` prompt before reading input in the main loop
- General code cleanup and formatting (spacing, blank lines) for readability across all functions

---

## v.0.1.2 alpha

### Added
- Quoted argument support in command parsing (e.g. `mk file "my file.txt"`), allowing names with spaces
- Confirmation prompt (`Continue? [Y/n]`) before `rm` deletes a file or folder
- `rm` now checks if the target exists before attempting deletion and reports "not found" if missing
- `rm` now uses `fs::remove_all` to support deleting non-empty directories
- `cp` now checks if source and destination are the same and reports failure instead of attempting the copy
- `mk file` checks if the file already exists and reports "fail (File already exists)" instead of overwriting
- Refactored command handling into separate functions: `cmd_cp`, `cmd_rn`, `cmd_rm`, `cmd_cd`, `cmd_mk`
- Added shared helper functions: `pause()`, `show_success()`, `show_fail()`, `show_error()`
- `<exception>` header added for exception handling

### Changed
- Replaced whitespace-only command splitting with a quote-aware parser that throws on unmatched quotes
- `help` command output reformatted and updated to reflect current commands
- `rm` usage changed from `rm [filename]` to `rm [file/dir]` (now supports folders)
- `cd` usage description updated to `cd [path/full path]`
- Standardized all "Press Enter to continue" prompts via the new `pause()` helper
- Standardized all success/fail/error messages via `show_success()`, `show_fail()`, `show_error()`

### Fixed
- Fixed input handling so prompts using `std::getline` properly skip leading whitespace (`std::cin >> std::ws`)
- Corrected inconsistent spacing/newlines in status and usage messages

---

## v.0.1.0

### Added
- Basic interactive file manager shell
- `cp [from] [to]` - copy files/directories recursively, overwrite existing
- `rm [filename]` - remove a file
- `rn [oldname] [newname]` - rename a file or directory
- `cd [path]` - change current directory
- `help` - show list of available commands
- `exit` - quit the program
- Directory listing on startup, marking entries as `[DIR]` or `[FILE]`
