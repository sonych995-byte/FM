# FM Command Reference (In-Depth)

This document explains every command in FM in detail: internal C++
implementation, source file location, edge cases, error handling, and
notable behavior in the current version.

**This revision covers v0.7.0**, in which the C++ core stopped being an
interactive terminal program and became a headless process driven by the
Flask/Python bridge. If you are looking for the pre-bridge, interactive
behavior (`pause()`, `show_success()`, on-screen prompts typed directly into
the core's console), see the changelog for v0.6.0-alpha and earlier — that
model no longer applies.

---

## Architecture Change in v0.7.0

Before v0.7.0, `main()` read commands from `std::cin`, printed results
straight to the console, and used `pause()` / `show_success()` /
`show_fail()` / `show_error()` / `clear_screen()` to drive an interactive
loop with on-screen prompts and confirmations.

As of v0.7.0, `core` is launched by `run.py` and no longer talks to a
terminal at all:

- `main.cpp` calls `bridge.receive("request")` in a loop instead of
  `std::getline(std::cin, ...)`.
- Every command function sends its result with `bridge.send(text,
  "response")` instead of printing to `std::cout`.
- `home()`, `pause()`, `show_success()`, `show_fail()`, `show_error()`, and
  `clear_screen()` have been **removed** from `utils.cpp` / `utils.hpp`.
  The only utility function that remains is `split()`.
- Confirmation prompts that used to read from `std::cin` (e.g. `rm`'s
  `Y/n`, `oscmd`'s `Y/n`) are gone from the core entirely. Confirmation is
  now the GUI's job (see `fm-gui.html`'s modal), and the confirmation itself
  is passed to the core as an extra command-line token (e.g. `rm path Y`).
- The request/response channel is a pair of JSON files (`request.json` /
  `response.json`) inside `runtime/`, written atomically (write to a temp
  file, then rename). See `Bridge.py` and `CORE/include/Bridge.hpp`.
- Only one request/response round trip is in flight at a time; `run.py`
  serializes calls with a lock.

---

## Project Structure

```
FM/
├── main.cpp                 C++ core entry point (bridge loop + dispatcher)
├── run.py                   Flask server and core launcher
├── Bridge.py                Python side of the file bridge
├── templates/
│   └── fm-gui.html          Browser interface
└── CORE/
    ├── include/
    │   ├── Bridge.hpp        C++ side of the file bridge
    │   ├── commands.hpp      Declarations for all command functions
    │   ├── common.hpp        Shared includes, `namespace fs = std::filesystem`, `extern Bridge bridge;`
    │   └── utils.hpp         Declaration for split() only
    └── src/
        ├── utils.cpp         split() — the only remaining utility function
        ├── cp.cpp            cmd_cp()
        ├── rn.cpp            cmd_rn()
        ├── rm.cpp            cmd_rm()
        ├── mv.cpp            cmd_mv()
        ├── cd.cpp            cmd_cd()
        ├── mk.cpp            cmd_mk()
        ├── cat.cpp           cmd_cat()
        ├── find.cpp          cmd_find() (search_file() also present but unused — see Known Issues)
        ├── edit.cpp          cmd_edit()
        ├── info.cpp          cmd_info()
        ├── ls.cpp            cmd_ls()
        ├── pwd.cpp           cmd_pwd()
        ├── oscmd.cpp         cmd_oscmd()
        └── help.cpp          cmd_help()
```

Each command `.cpp` file includes `../include/common.hpp` and
`../include/utils.hpp`. `main.cpp` includes `CORE/include/common.hpp`,
`CORE/include/utils.hpp`, and `CORE/include/commands.hpp`, and defines the
global `Bridge bridge(std::filesystem::absolute("runtime"));` instance that
every command function writes to.

---

## General Notes

### Input Parsing — `split()` (`CORE/src/utils.cpp`)

Unchanged in behavior from earlier versions. Before any command runs, the
raw request string (received via `bridge.receive("request")`) is passed
through `split()`, which:

- Splits on spaces, **except** inside quotes.
- Supports **both** double quotes (`"`) and single quotes (`'`).
- Quote types do not nest: a `'` inside a `"..."` block is treated as a
  literal character, because each toggle checks the opposite flag
  (`!in_single_quotes` / `!in_double_quotes`).
- If a quote is opened but never closed, `split()` throws:
  - `std::runtime_error("Missing closing double quote")`
  - `std::runtime_error("Missing closing single quote")`
- Quote characters themselves are stripped from the resulting argument
  (e.g. `mk file "my file.txt"` → arg is `my file.txt`).

**What changed:** these exceptions are no longer caught with `show_error()`
+ `pause()` + `continue`. They are caught in `main()` and sent back over the
bridge as `"Error: " + e.what()`, then the loop `continue`s to wait for the
next request. Empty input (`args.empty()` after parsing) is silently
`continue`d with **no response sent at all** for that request — in
practice this path is unreachable through the GUI/API, because Flask's
`/api/run` already rejects an empty `"command"` field with a 400 before it
ever reaches the bridge.

### Removed Helpers

`home()`, `pause()`, `show_success()`, `show_fail()`, `show_error()`, and
`clear_screen()` no longer exist anywhere in the codebase. Every command
function is now responsible for building its own response string and
sending it once via `bridge.send(text, "response")`.

### Main Loop (`main.cpp`)

- Loops on `bridge.receive("request")` instead of `std::getline(std::cin, ...)`.
- Parses with `split()`; a parse error is reported over the bridge (see
  above) and the loop continues.
- `exit` is checked first with a standalone `if`. It sends `"Core shutting
  down"` as the response, then `break`s out of the loop — `main()` returns
  `0` and the process exits, which is separately caught by `run.py`'s
  `atexit`-registered terminator (a no-op at that point, since the process
  already exited on its own).
- All other commands are dispatched through an `if/else if` chain exactly
  as before.
- Unknown commands now send `"Invalid command"` (previously `"Unknown
  command"` in the interactive version) via the bridge.
- Dispatch is still case-sensitive and exact-match only (`CP`, `Exit`, etc.
  are all unknown).
- Any other exception escaping a command handler is caught at the very
  outer `try` in `main()` and only logged to `std::cerr` on the core's own
  console — **no response is sent to the bridge in that case**, so the
  in-flight request will time out on the Python side after
  `BRIDGE_TIMEOUT_SECONDS` (15s, see `run.py`).

---

## Command Details

### 1. `cp [source] [destination]` — Copy

**Source:** `CORE/src/cp.cpp`

- **Argument check:** requires exactly 3 tokens. If not, the intended
  message is `"Usage: cp [source] [destination]"` — see **Known Issues**
  below, this call is currently broken.
- **String equality check:** if `args[1] == args[2]` (raw string
  comparison), sends `"Source and destination are the same"`.
- **Filesystem equivalence check:** if both paths exist and
  `fs::equivalent(args[1], args[2])` is true (same inode via different
  paths, symlinks, or relative/absolute forms), also sends `"Source and
  destination are the same"`.
- **Copy operation:**
```cpp
  fs::copy(args[1], args[2],
      fs::copy_options::recursive |
      fs::copy_options::overwrite_existing);
```
- **On success:** sends `"File copied successfully"`.
- **On `fs::filesystem_error`:** sends the generic message `"Error copying
  file"` (the exception's own `what()` text is not forwarded).

**Notes:**
- No explicit check that `args[1]` exists before calling `fs::copy`; the
  underlying exception handles it.
- Copying a directory into itself, where paths are not string-equal but not
  yet filesystem-equivalent, may throw a `filesystem_error` caught by the
  generic handler above.

---

### 2. `rn [oldname] [newname]` — Rename

**Source:** `CORE/src/rn.cpp`

- **Argument check:** requires exactly 3 tokens. If not, sends `"Usage: rn
  [from] [to]"`.
- **Operation:** `fs::rename(args[1], args[2])`. Works on both files and
  directories, and can move a file across directories if `newname`
  includes a path.
- **On success:** sends `"File or directory renamed successfully"`.
- **On `fs::filesystem_error`:** sends `"Error: Unable to rename file or
  directory"`.

**Notes:**
- No existence check; relies entirely on the exception from `fs::rename`.

---

### 3. `mv [source] [destination]` — Move/Rename

**Source:** `CORE/src/mv.cpp`

- **Argument check:** requires exactly 3 tokens. If not, sends `"Usage: mv
  [from] [to]"`.
- **Operation:** `fs::rename(args[1], args[2])` — functionally identical to
  `rn`; both use `fs::rename` internally.
- **On success:** sends `"File/directory moved successfully"`.
- **On `fs::filesystem_error`:** sends `"Error: Unable to move file or
  directory"`.

---

### 4. `rm [path] [Y]` — Remove

**Source:** `CORE/src/rm.cpp`

- **Argument check:** requires exactly 2 **or** 3 tokens. Otherwise sends
  `"Usage: rm [path] [Y]"`.
- **Existence check:** if `fs::exists(args[1])` is false, sends `"Error:
  Path does not exist"`.
- **No interactive confirmation prompt anymore.** Confirmation is now
  expressed as a third command-line token:
  - 2 tokens (`rm path`): sends `"Error: confirmation required"` and stops
    — nothing is deleted.
  - 3 tokens, `args[2]` is `"Y"` or `"y"`: proceeds to delete.
  - 3 tokens, anything else: sends `"Delete cancelled"`.
- **Deletion:** `fs::remove_all(args[1])`. Recursively removes non-empty
  directories; the return value (count removed) is not checked.
- **On success:** sends `"File or directory removed successfully"`.
- **On `fs::filesystem_error`:** sends `"Error: Unable to remove file or
  directory"`.

**Notes:**
- The GUI's delete confirmation modal (`fm-gui.html`) is what supplies the
  `Y` token — it sends `rm "<path>" Y` directly once the user confirms in
  the browser, rather than the core prompting for it.

---

### 5. `cd [path]` — Change Directory

**Source:** `CORE/src/cd.cpp`

- **Argument check:** requires exactly 2 tokens. If not, sends `"Usage: cd
  [folder/path]"`.
- **Operation:** `fs::current_path(args[1])` — changes the process's
  working directory.
- **On success:** sends `"PATH:" + fs::current_path().string()`. This
  `PATH:` prefix is a small protocol addition consumed by `fm-gui.html`,
  which updates its own `state.path` whenever a response starts with
  `PATH:`.
- **On `fs::filesystem_error`:** sends `"Error: Unable to change
  directory"`.

**Notes:**
- No explicit check that the target is a directory; relies on the
  exception from `fs::current_path`.

---

### 6. `mk file [name]` / `mk dir [name]` — Make File or Directory

**Source:** `CORE/src/mk.cpp`

- **Argument check:** requires exactly 3 tokens. Otherwise sends `"Usage:
  mk [file/dir] [name]"`.

#### `mk file [name]`

- **Existence check:** if `fs::exists(args[2])` is true, sends `"Error:
  File already exists"`.
- **Creation:** opens `std::ofstream file(args[2])`.
  - If open succeeds: closes the file (creating an empty file) and sends
    `"File created successfully"`.
  - If open fails (invalid path, permission denied): sends `"Error: Unable
    to create file"` — no exception details are included.
- Parent directories are **not** created automatically.

#### `mk dir [name]`

- **Creation:** `fs::create_directories(args[2])`.
  - Returns `true` if at least one new directory was created → sends
    `"Directory created successfully"`.
  - Returns `false` if the full path already exists → sends `"Error:
    Unable to create directory"` (indistinguishable from other failures).
- **On `fs::filesystem_error`:** sends the same `"Error: Unable to create
  directory"`.

#### Invalid subcommand

- Sends `"Usage: mk [file/dir] [name]"`.

---

### 7. `cat [filename]` — Display File Contents

**Source:** `CORE/src/cat.cpp`

- **Argument check:** requires exactly 2 tokens. Otherwise sends `"Usage:
  cat [filename]"`.
- Opens `args[1]` with `std::ifstream`.
  - If open fails: sends `"Error: Unable to open file"`.
- Reads every line via a `std::getline` loop, appending `line + '\n'` to a
  single `output` string, then sends the whole string in **one**
  `bridge.send` call (previously each line was printed and then a single
  `show_success()` followed).
- **On `std::exception`:** sends `"Error: Unable to display file
  contents"` (generic message, not `e.what()`).

**Notes:**
- No binary detection; `cat`-ing a binary file sends garbled text with no
  error.
- Very large files are sent in full, in one message, with no paging.

---

### 8. `find [path] [name]` — Search Files

**Source:** `CORE/src/find.cpp`

- **Argument check:** requires exactly 3 tokens. Otherwise sends `"Usage:
  find [path] [name]"`.
- No `"Searching..."` progress message is sent anymore — the whole search
  runs synchronously and only the final result is sent.

#### Path resolution — **changed from earlier versions**

| `args[1]` | Resolved search root |
|---|---|
| `.` | `fs::current_path()` |
| anything else (including `..` and `/`) | used as-is, i.e. `fs::path(args[1])` |

The special-case handling that previously treated `..` as "current
directory's parent" and `/` as "search the whole filesystem" **no longer
exists**. Only `.` is special-cased.

- If the resolved root does not exist, sends `"Error: Path does not
  exist"`.
- Otherwise iterates with `fs::recursive_directory_iterator(root,
  fs::directory_options::skip_permission_denied)`, matching files whose
  `entry.path().filename() == args[2]` (exact, case-sensitive), and
  appends each match's full path + `"\n"` to an `output` string.
- **Result:** sends `output` if non-empty, otherwise sends `"No matches
  found"`. There is no `"Found N file(s)"` count message — the response is
  either the raw newline-separated list of matching paths, or the
  no-matches string.
- **On `std::exception`:** sends `"Error: " + e.what()`.

---

### 9. `edit [tool] [filename]` — Open File in External Editor

**Source:** `CORE/src/edit.cpp`

- **Argument check:** requires exactly 3 tokens. If not, the intended
  message is `"Usage: edit [tool] [filename]"` — see **Known Issues**,
  this call is currently broken the same way as `cp`'s usage message.
- **Editor allow-list — new in this version.** `tool` must be exactly one
  of `"nano"`, `"vim"`, `"gedit"`, or `"code"`. Any other value throws
  `std::runtime_error("Editor not allowed")`. (Earlier versions allowed
  any tool available on `PATH`.)
- **Existence check:** if `fs::exists(filename)` is false, throws
  `std::runtime_error("File does not exist")`.
- **Filename character check — new in this version:** if `filename`
  contains a `"` or a newline character, throws
  `std::runtime_error("Filename contains unsupported characters")`.
- **Command construction:** builds `tool + " \"" + filename + "\""` and
  runs it via `system()`.
- **Exit code check:** if `system()` returns non-zero, throws
  `std::runtime_error("Failed to open editor")`.
- **On any caught `std::exception`:** sends `e.what()` as the response.
- **On success: nothing is sent.** There is no `bridge.send` call outside
  the `catch` block, so a successful edit produces no response at all —
  see **Known Issues**.

---

### 10. `info [path]` — File/Folder Information

**Source:** `CORE/src/info.cpp`

This command was significantly simplified compared to earlier versions —
the old permission-bit display, recursive directory size, and one-level
child listing are all gone.

- **Argument check:** requires exactly 2 tokens. Otherwise sends `"Usage:
  info [path]"`.
- **Existence check:** if `fs::exists(p)` is false, sends `"Error: Path
  does not exist"`.
- **Fields sent, for every path type:**
  - `Path:` — `fs::absolute(p).string()`
  - `Type:` — `File` (via `fs::is_regular_file`), `Directory` (via
    `fs::is_directory`), or `Other` for anything else (symlinks that don't
    resolve to one of those, sockets, FIFOs, devices, etc.)
  - `Size:` — **files only** — `fs::file_size(p)` in bytes.
  - `Last Modified:` — computed from `fs::last_write_time(p)`, converted to
    a `std::time_t` and formatted with `std::asctime`/`std::localtime`.
    This line is included for **every** path type (file, directory, or
    other), not just files.
- **On `std::exception`:** sends `"Error: " + e.what()`.

**Notes:**
- No owner/group/other permission bits are reported anymore.
- No recursive size total for directories.
- No listing of a directory's immediate children.
- Symlinks are followed (`fs::status`-style calls), consistent with
  earlier versions.

---

### 11. `ls [path]` — List Directory

**Source:** `CORE/src/ls.cpp`

- **Argument handling:**
  - 1 token (`ls`): target = `fs::current_path()`.
  - 2 tokens (`ls path`): target = `args[1]`.
  - 3+ tokens: sends `"Usage: ls [path]"`.
- **Existence check:** if `fs::exists(target)` is false, sends `"Error:
  Path does not exist"`.
- **Regular file target — changed behavior:** if `fs::is_regular_file(target)`
  is true, sends `"Error: Path is not a directory"` and returns. (Earlier
  versions displayed the file as a single `[FILE]` entry instead of
  treating it as an error.)
- **Directory target:** iterates `fs::directory_iterator(target)`,
  building one `"[DIR] name\n"` or `"[FILE] name\n"` line per entry, then
  sends the whole listing in one message. There is no separate `"Listing:
  <path>"` header line in the response anymore — `run.py`'s `/api/list`
  endpoint reconstructs the `entries` array itself by parsing these
  `[DIR] ` / `[FILE] ` prefixes.
- **On `std::exception`:** sends `"Error: Unable to list directory
  contents: " + e.what()`.

---

### 12. `pwd` — Print Working Directory

**Source:** `CORE/src/pwd.cpp`

- No argument check; extra tokens after `pwd` are silently ignored (the
  function takes no parameters).
- Sends `fs::current_path().string()` directly — no `"current path: "`
  label text is prepended anymore.
- **On `fs::filesystem_error`:** sends `"Error: Unable to get current
  working directory"`.

---

### 13. `oscmd [command]` — Run OS Command

**Source:** `CORE/src/oscmd.cpp`

- **Argument check:** requires exactly 2 tokens. Otherwise sends `"Usage:
  oscmd [command]"`.
- **Self-launch guard — expanded in this version:** blocks `"fm"`,
  `"./fm"`, `"fm.exe"`, and now also `"core"`, `"./core"`, `"core.exe"`
  (the renamed binary introduced in v0.7.0). Any of these sends `"Error:
  Cannot launch FM inside FM"`.
- **No interactive confirmation prompt anymore.** The core executes the
  command immediately; confirmation is handled client-side by
  `fm-gui.html`'s modal before it ever calls `/api/run`.
- **Execution:** `std::system(args[1].c_str())`.
  - The exit code **is** checked (unlike earlier versions): non-zero sends
    `"Error: Command failed with exit code " + std::to_string(result)`;
    zero sends `"Command executed successfully"`.
- **On `std::exception`:** sends `"Error: Exception occurred while
  executing command"`.

**Security notes (unchanged):**
- `std::system` spawns the OS default shell — any command the current user
  can run in a terminal can be run here.
- `cd` inside `oscmd` runs in a child process and has no effect on the
  core's own working directory.
- The self-launch guard only matches the six hardcoded strings above;
  other paths to the binary (e.g. `../fm`, `/usr/bin/core`) are not
  blocked.

---

### 14. `help` — Help Screen

**Source:** `CORE/src/help.cpp`

- Sends one static multi-line string over the bridge — a short one-line
  syntax summary for every command (`cp`, `rn`, `mv`, `rm`, `cd`, `mk`,
  `info`, `ls`, `cat`, `find`, `edit`, `oscmd`, `help`, `exit`).
- This is a simplification from earlier versions: there are no longer
  separate **Syntax / Example / Output** subsections per command, and
  nothing is printed to a console (`clear_screen()` is gone) — the whole
  thing is a single response string, intended to be rendered by the GUI or
  whatever client called `/api/run`.
- The help text is hardcoded; any new command added to the dispatcher must
  still be added here manually.

---

### 15. `exit` — Quit

**Source:** `main.cpp` (dispatcher)

- Checked first, with a standalone `if` (not part of the `else if`
  chain).
- Sends `"Core shutting down"` as the response, then `break`s out of the
  `while (true)` loop. `main()` returns `0`.
- No confirmation, no cleanup beyond the normal process exit. `run.py`'s
  `atexit`-registered `_stop_core()` will find the process already
  terminated at that point and do nothing.

---

### 16. Unknown Commands

**Source:** `main.cpp` (dispatcher)

- Any `args[0]` not matching a known command sends `"Invalid command"`
  (renamed from `"Unknown command"` in the interactive version).
- Matching is exact and case-sensitive.

---

## Known Issues

These are real inconsistencies present in the current source, not
intentional behavior — listed here so they're not mistaken for spec.

1. **`cp` and `edit` usage-error calls have their `bridge.send` arguments
   reversed.** `Bridge::send(const std::string& data, const std::string&
   name)` expects the message body first and the channel name (`"response"`)
   second. The usage-check branches in `cp.cpp` and `edit.cpp` instead call:
   ```cpp
   bridge.send("response", "Usage: cp [source] [destination]");
   ```
   This writes a file literally named `Usage: cp [source] [destination].json`
   inside `runtime/` instead of `response.json`, with the string
   `"response"` as its contents. `Bridge.py`'s `receive("response")` never
   finds `response.json` for that call, so `run.py`'s `call_core()` blocks
   until `BRIDGE_TIMEOUT_SECONDS` (15s) and the browser sees a `504`/timeout
   instead of the intended usage message. Every other `bridge.send` call in
   the codebase uses the correct `(data, name)` order.

2. **`edit` sends no response at all on success.** `cmd_edit()` only calls
   `bridge.send(...)` from inside its `catch` block. When the whitelisted
   editor opens and exits with code `0`, no message is ever sent back, so a
   successful edit also produces a bridge timeout on the Python side, even
   though the file was opened correctly.

3. **`find.cpp` still defines `search_file()`, but `cmd_find()` no longer
   calls it.** The current `cmd_find()` implements its own inline
   `recursive_directory_iterator` loop. `search_file()` (which sends a
   `bridge.send` per match and increments a `count` reference) is dead
   code left over from the previous implementation.

---

## Building (Multi-file)

Build from the repository root; the compiled output must be named `core`
(`core.exe` on Windows), matching the filename `run.py` looks for.

**Linux / macOS**
```bash
g++ -std=c++17 -pthread -I. -o core \
  main.cpp \
  CORE/src/utils.cpp CORE/src/cp.cpp CORE/src/rn.cpp CORE/src/rm.cpp \
  CORE/src/mv.cpp CORE/src/cd.cpp CORE/src/mk.cpp CORE/src/cat.cpp \
  CORE/src/find.cpp CORE/src/edit.cpp CORE/src/info.cpp CORE/src/ls.cpp \
  CORE/src/pwd.cpp CORE/src/oscmd.cpp CORE/src/help.cpp
```

**Windows**
```powershell
g++ -std=c++17 -pthread -I. -o core.exe `
  main.cpp `
  CORE/src/utils.cpp CORE/src/cp.cpp CORE/src/rn.cpp CORE/src/rm.cpp `
  CORE/src/mv.cpp CORE/src/cd.cpp CORE/src/mk.cpp CORE/src/cat.cpp `
  CORE/src/find.cpp CORE/src/edit.cpp CORE/src/info.cpp CORE/src/ls.cpp `
  CORE/src/pwd.cpp CORE/src/oscmd.cpp CORE/src/help.cpp
```

Run the whole application (Flask server + core) with:

```bash
python3 run.py
```
