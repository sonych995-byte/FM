# FM Command Reference (In-Depth)

This document explains every command in FM in detail, including the internal C++ implementation, edge cases, error handling, and quirks/bugs found in the current code.

---

## General Notes

### Input Parsing - `split()`

Before any command runs, the raw input line is passed through `split()`, which:

- Splits on spaces, **except** inside quotes.
- Supports **both** double quotes (`"`) and single quotes (`'`).
- Quote types do not nest with each other: a `'` inside a `"..."` block is treated as a literal character (and vice versa), because the quote-toggle checks `!in_single_quotes` / `!in_double_quotes`.
- If a quote is opened but never closed, `split()` throws:
  - `std::runtime_error("Missing closing double quote")`
  - `std::runtime_error("Missing closing single quote")`
- These exceptions are caught in `main()`, printed via `show_error()`, followed by `pause()`, and the loop `continue`s (the screen is **not** redrawn before showing the error, since `pause()` doesn't clear the screen).
- Quote characters themselves are stripped from the resulting argument (e.g. `mk file "my file.txt"` → arg is `my file.txt`, no quotes).
- Empty input (after parsing, `args.empty()`) is silently ignored and the loop redraws `home()`.

### Helper Functions

- **`pause()`** - prints `"\n\nPress any button and enter to continue"`, then reads a full line via `std::getline(std::cin >> std::ws, temp)`. The `std::ws` skips any leftover whitespace/newlines from previous input, so a single Enter press is consumed correctly.
- **`show_success()`** - prints `"\n\nstatus: success\n\n"` then calls `pause()`.
- **`show_fail()`** - prints `"\n\nstatus: fail\n\n"` then calls `pause()`.
- **`show_error(e)`** - prints `"\n\nError:\n"` followed by `e.what()`. **Note:** this function does *not* call `pause()` itself - every call site must call `pause()` separately afterward.

### Main Loop Behavior

- Each iteration starts by clearing the screen (`\033[2J\033[H`) and calling `home()`, which redraws the banner, current path, and a flat (non-recursive) listing of the current directory contents with `[DIR]` / `[FILE]` prefixes.
- `home()` catches `fs::filesystem_error` if the directory iterator fails (e.g. permission denied) and prints the error inline without pausing.
- After `home()`, the prompt `"\ncommand: "` is shown and a line is read with `std::getline`.
- The command name is `args[0]`; dispatch is a chain of `if/else if` string comparisons (case-sensitive, exact match).
- Anything not matching a known command prints `"\n\nUnknown command"` and pauses.

---

## Command Details

### 1. `cp [from] [to]` - Copy

**Function:** `cmd_cp`

- **Argument check:** requires exactly 3 tokens (`cp`, `from`, `to`). If not, prints `"\n\nUseage: cp [from] [to]"` (note: typo "Useage") and pauses.
- **Self-copy check:** if `from == to` (string comparison), immediately calls `show_fail()` without attempting any filesystem operation - this prevents `fs::copy` from throwing on identical source/destination.
- **Copy operation:** uses
  ```cpp
  fs::copy(from, to, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
  ```
  - `recursive`: if `from` is a directory, its entire contents are copied.
  - `overwrite_existing`: if `to` already exists, it is overwritten rather than causing an error.
- **On success:** `show_success()`.
- **On `fs::filesystem_error`:** `show_error(e)` then `pause()`.

**Edge cases / behavior notes:**
- Copying a directory into itself (e.g. `cp dir dir/sub`) is not specifically checked beyond the exact-string-equality check, and will likely throw a `filesystem_error` caught by the generic handler.
- No check that `from` exists before calling `fs::copy` - the underlying exception handles that case.

---

### 2. `rn [oldname] [newname]` - Rename

**Function:** `cmd_rn`

- **Argument check:** requires exactly 3 tokens. If not, prints `"\n\nUseage: rn [oldname] [newname]"` and pauses.
- **Operation:** `fs::rename(oldname, newname)`.
  - This is effectively a move - works across files and directories, and can move into a different directory if `newname` includes a path.
- **On success:** `show_success()`.
- **On `fs::filesystem_error`:** `show_error(e)` then `pause()`.

**Edge cases:**
- No existence check before renaming; relies on the exception from `fs::rename`.
- No same-name check (unlike `cp`), so `rn a a` will be attempted as-is (typically a no-op or error depending on the OS/filesystem).

---

### 3. `rm [file/folder]` - Remove

**Function:** `cmd_rm`

- **Argument check:** requires exactly 2 tokens (`rm`, `target`). If not, prints `"\n\nUsage: rm [file/folder]"` and pauses.
- **Existence check:** if `fs::exists(target)` is false, prints `"\n\nError: not found"` and pauses, then returns (no deletion attempted).
- **Confirmation prompt:**
  ```cpp
  std::cout << "\n\nContinue? [Y/n]: ";
  std::getline(std::cin >> std::ws, conti);
  ```
  - Accepts `"Y"` or `"y"` (exact string match) to proceed.
  - Any other input (including empty string from just pressing Enter) cancels - the function `return`s immediately **without any message and without calling `pause()`**. The screen will simply redraw on the next loop iteration.
- **Deletion:** `fs::remove_all(target)`.
  - Unlike a plain `fs::remove`, `remove_all` works on **non-empty directories**, recursively deleting everything inside.
- **On success:** `show_success()`.
- **On `fs::filesystem_error`:** `show_error(e)` then `pause()`.

**Edge cases:**
- There is no distinct return value check from `remove_all` (it returns the number of files removed) - success is assumed if no exception is thrown, even if the count is 0 (e.g. target vanished between the existence check and the removal).
- Deleting the current working directory or its parent could cause subsequent `home()` calls to fail (caught and printed inline by `home()`'s own try/catch).

---

### 4. `cd [path]` - Change Directory

**Function:** `cmd_cd`

- **Argument check:** requires exactly 2 tokens (`cd`, `path`). If not, prints `"\n\nUseage: cd [folder/path]"` and pauses.
- **Operation:** `fs::current_path(path)` - changes the process's working directory.
  - Accepts relative paths (`..`, `subfolder`), absolute paths, and (per the quoting support) paths with spaces if quoted.
- **On success:** returns silently - the next loop iteration's `home()` call will show the new path and its contents.
- **On `fs::filesystem_error`** (e.g. path doesn't exist, not a directory, permission denied): `show_error(e)` then `pause()`.

**Edge cases:**
- No explicit check that the target is a directory (vs. a file) - relying entirely on the exception from `fs::current_path`.
- `cd ..` works as expected via the underlying OS/filesystem semantics.

---

### 5. `mk file [name]` / `mk dir [name]` - Make File or Directory

**Function:** `cmd_mk`

- **Argument check:** requires exactly 3 tokens (`mk`, `file`|`dir`, `name`). If not, prints `"\n\nUseage: mk [file/dir] [name]"` and pauses.

#### `mk file [name]`

- **Existence check:** if `fs::exists(name)` is true, prints `"\n\nstatus: fail (File already exists)"` and pauses (does **not** call `show_fail()`, so the message text differs slightly from the generic fail message).
- **Creation:** opens `std::ofstream file(name)`.
  - If the stream opens successfully, it is immediately closed (creating an empty file) and `show_success()` is called.
  - If `file.is_open()` is false (e.g. invalid path, permission denied), `show_fail()` is called - note this path produces **no error details**, just a generic "fail".

#### `mk dir [name]`

- **Creation:** `fs::create_directories(name)`.
  - This creates all intermediate directories as needed (like `mkdir -p`), so `mk dir a/b/c` creates `a`, `a/b`, and `a/b/c` if they don't exist.
  - **Return value semantics:** `create_directories` returns `true` only if it actually created at least one new directory. If the directory (and all its parents) **already exist**, it returns `false`.
    - In that case, the code calls `show_fail()` - meaning **"directory already exists" is reported as a generic "fail"**, not distinguished from other failure types.
- **On `fs::filesystem_error`:** `show_error(e)` then `pause()`.

#### Invalid second argument (neither `file` nor `dir`)

- Prints `"\n\nUseage: mk [file/dir] [name]"` and pauses (same message as the wrong-arg-count case).

**Edge cases:**
- `mk file` does not create parent directories - if the path's parent directory doesn't exist, `std::ofstream` will simply fail to open, falling into the generic `show_fail()` branch with no details.
- `mk dir` on an already-existing path is indistinguishable (to the user) from other failure conditions, since both produce `"status: fail"`.

---

### 6. `info [path]` - File/Folder Information

**Function:** `cmd_info`

- **Argument check:** requires exactly 2 tokens (`info`, `path`). If not, prints `"\n\nUsage: info [path]"` and pauses.
- **Existence check:** if `fs::exists(p)` is false, prints `"\n\nError: not found"` and pauses, then returns.
- **Display:** clears the screen (`\033[2J\033[H`) and prints a header `"==== INFO ===="`, followed by:
  - **Name:** `p.filename().string()`
  - **Path:** `fs::absolute(p).string()` - the fully resolved absolute path.

#### If `p` is a regular file (`fs::is_regular_file`)

- **Type:** `File`
- **Extension:** `p.extension().string()` (includes the leading dot, e.g. `.cpp`; empty string if there is no extension)
- **Size:** `fs::file_size(p)` in bytes
- **Permission:** a 3-character string built from the **owner**'s permission bits only:
  ```cpp
  auto perm = fs::status(p).permissions();
  // r: owner_read, w: owner_write, x: owner_exec
  ```
  Each bit prints `r`/`w`/`x` if set, or `-` if not. Group and "other" permission bits are **not shown**.

#### If `p` is a directory (`fs::is_directory`)

- **Type:** `Folder`
- **Total size:** sums `fs::file_size(entry.path())` over every regular file found via `fs::recursive_directory_iterator(p)` (i.e. recursively through all subdirectories).
  - Any per-entry exception during this loop is caught by a bare `catch (...) {}` and **silently ignored** - so files that can't be stat'd (e.g. broken symlinks, permission issues) simply don't contribute to the total, with no warning to the user.
  - Directory entries themselves are not counted toward size (only `is_regular_file` entries).
- **Inside (1 level):** lists immediate children only (`fs::directory_iterator(p)`, non-recursive), printed as `" - <filename>"`.

#### Error handling

- If any `fs::filesystem_error` occurs during the above (e.g. `fs::absolute`, `fs::file_size` on the top-level path, or the directory iterators themselves throwing), it's caught: `show_error(e)` then `pause()`.
- **After the try/catch block, `pause()` is called again unconditionally** - meaning if an error occurred, the user is prompted to continue **twice** in a row (once inside the catch block, once after it).

**Edge cases:**
- Symlinks: `fs::is_regular_file` and `fs::is_directory` follow symlinks by default (using `fs::status`, not `fs::symlink_status`), so a symlink to a file is treated as that file.
- Special files (sockets, devices, FIFOs) would fall through both the `is_regular_file` and `is_directory` branches, resulting in just the Name/Path header being shown with no Type/Size info.

---

### 7. `ls [path]` - List Directory Contents

**Function:** `cmd_ls`

- **Argument handling:**
  - 1 token (just `ls`): target = `fs::current_path()`.
  - 2 tokens (`ls path`): target = the given path.
  - 3+ tokens: prints `"\n\nUsage: ls [path]"`, pauses, and returns.
- **Existence check:** if `fs::exists(target)` is false, prints `"\n\nError: path not found"` and pauses, then returns.
- **Listing:**
  - Prints `"\nListing: <absolute path>\n\n"`.
  - Iterates `fs::directory_iterator(target)`, printing `"[DIR]  "` (two spaces) or `"[FILE] "` (one space) followed by the entry's filename.
- **On `fs::filesystem_error`** during iteration: `show_error(e)` then `pause()`.
- **After the try/catch, `pause()` is called again unconditionally** - same double-pause behavior as `info` if an error occurs.

**Edge cases:**
- If `target` exists but is a **file** (not a directory), `fs::directory_iterator` on a non-directory path throws `fs::filesystem_error`, which is caught and reported - so `ls somefile.txt` results in an error message, not file info (use `info` for that).
- The `[DIR]` and `[FILE]` prefixes have different spacing (`"[DIR]  "` vs `"[FILE] "`) so filenames still align in a column.

---

### 8. `pwd` - Print Working Directory

**Function:** `cmd_pwd`

- Takes no arguments (the `args` vector isn't even checked/used).
- Prints `"\n\ncurrent path: " << fs::current_path() << "\n\n"`.
- Calls `pause()`.

**Edge cases:**
- Even if extra arguments are typed after `pwd` (e.g. `pwd foo bar`), they are silently ignored since `cmd_pwd()` takes no parameters and the dispatcher only checks `args[0] == "pwd"`.

---

### 9. `oscmd [command]` - Run OS Command

**Function:** `cmd_oscmd`

This is the most powerful and most dangerous command - it shells out to the operating system via `std::system`.

- **Argument check:** requires exactly 2 tokens (`oscmd`, `command`).
  - If not, prints `"\n\nUsage: oscmd [command]"` and calls `pause()`.
  - **Important quirk:** there is **no `return`** after this message. Execution **falls through** to the rest of the function regardless. If `args.size() != 2`, `args[1]` would be **out of bounds** - this is undefined behavior (likely a crash or garbage read) if the user types just `oscmd` with no argument, since `args` would only have 1 element.

- **Self-launch guard:** if `args[1]` is exactly `"./fm"` or `"fm"`, prints `"\n\ncannot launch fm inside fm"` and calls `pause()`.
  - **Same quirk as above:** no `return` here either - execution continues to the confirmation prompt and, if confirmed, **`std::system("fm")` or `std::system("./fm")` would still be executed anyway**, since the guard only prints a warning but doesn't stop the call to `std::system(args[1].c_str())` below.

- **Confirmation prompt:**
  ```cpp
  std::cout << "\n\nSome commands are dangerous";
  std::cout << "\nContinue? [Y/n]: ";
  std::cin >> conti;
  ```
  - Uses `std::cin >> conti` (whitespace-delimited single token), **not** `std::getline` - this differs from `rm`'s confirmation, which uses `getline`. This means leftover newline characters in the input buffer are consumed differently, and only the first whitespace-delimited word is read (e.g. typing "Yes please" would set `conti` to `"Yes"`).
  - Accepts `"Y"` or `"y"` to proceed.

- **Execution:** if confirmed, `std::system(args[1].c_str())` runs the given string as a shell command using the system's default shell.
  - The single quotes shown in the help example (`oscmd 'pkg install git'`) are consumed by FM's own `split()` function - by the time `std::system` receives the string, it's just `pkg install git` with **no quotes**, since `split()` strips quote characters. The quotes in the example are therefore for FM's argument parser, not for the underlying shell.
  - After execution, `show_success()` is called **unconditionally** - the actual exit code of `std::system` is **not checked**, so even if the OS command fails, FM reports `"status: success"`.

- **Cancellation:** if the confirmation is anything other than `"Y"`/`"y"`, the function `return`s with no message (similar to `rm`'s cancellation).

**Edge cases / risks:**
- Because `split()` strips quotes and splits on spaces, any `oscmd` command containing spaces **must** be wrapped in quotes when typed into FM, or it will be parsed as multiple arguments and fail the `args.size() != 2` check (falling into the undefined-behavior path described above).
- `cd` as an `oscmd` argument (e.g. `oscmd 'cd ..'`) would run in a subshell via `std::system` and have **no effect** on FM's own working directory, since `std::system` spawns a separate child process.
- This command is effectively a sandbox escape - any command the OS user can run, FM can run, including destructive operations (`rm -rf`, etc.) with only a single `y` confirmation.

---

### 10. `help` - Detailed Help Screen

**Function:** `cmd_help`

- Clears the screen (`\033[2J\033[H`).
- Prints a large raw string literal (`R"(...)"`) containing numbered descriptions (1-9) of every command: `cp`, `rn`, `rm`, `cd`, `mk`, `info`, `ls`, `pwd`, `oscmd`.
  - Each entry includes a **Syntax**, **Example**, and **Output** description.
  - The text is indented with literal spaces as part of the raw string (preserved exactly as written in the source, including some inconsistent indentation - e.g. the `mk` example line has a stray `"Program"` prefix: `"Program  - mk file src.cpp"`, which appears to be a leftover typo/artifact in the source).
- Calls `pause()` at the end.

**Note:** the help text does not auto-update if commands change - it's a hardcoded string, so any new command added to the dispatcher must be manually added here too.

---

### 11. `exit` - Quit

- No dedicated function; handled directly in `main()`'s dispatcher via `break`, which exits the `while (true)` loop and the program returns `0`.
- No confirmation, no cleanup - immediate exit.

---

### 12. Unknown Commands

- Any input whose first token doesn't match `cp`, `rn`, `rm`, `cd`, `mk`, `help`, `exit`, `info`, `ls`, `pwd`, or `oscmd` prints `"\n\nUnknown command"` and calls `pause()`.
- Matching is exact and case-sensitive (`CP`, `Cp`, ` cp` with leading space already trimmed by `split()`, etc. would not match `cp`).

---

## Summary Table of Known Quirks/Bugs

| Area | Issue |
|---|---|
| `cmd_oscmd` argument check | Missing `return` after usage error - can read out-of-bounds `args[1]` if no argument given |
| `cmd_oscmd` self-launch guard | Missing `return` - warning is shown but `fm`/`./fm` can still run if confirmed |
| `cmd_oscmd` confirmation input | Uses `std::cin >> conti` instead of `getline`, inconsistent with `rm`'s confirmation style |
| `cmd_oscmd` exit status | `std::system`'s return value is ignored; always reports success |
| `cmd_info` / `cmd_ls` error path | `pause()` is called both inside the catch block and again after it, causing a double prompt on error |
| `mk dir` on existing directory | Reported as generic `"status: fail"`, same as other unrelated failures |
| `mk file` open failure | Reported as generic `"status: fail"` with no underlying error message |
| `rm` / `oscmd` cancellation | Returns silently with no confirmation message that the action was cancelled |
| `cmd_help` text | Contains a stray `"Program"` artifact on the `mk` example line; hardcoded and must be manually kept in sync with the dispatcher |
| Quote handling | Double and single quotes don't nest inside each other; quotes are stripped before reaching `oscmd`'s underlying shell command |
