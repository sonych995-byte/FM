# FM

FM is a local web-based file manager. The browser interface is served by
Flask, while the file operations are implemented in C++17. `run.py` starts
the Flask server and launches the C++ core; both processes communicate through
request and response files in the `runtime/` directory.

## Features

- Browse files and directories in a browser
- Copy, move, rename, and remove files or directories
- Create files and directories
- View file contents and file metadata
- Search recursively by filename
- Open files with an external editor
- Run supported FM commands from the command composer
- Use quoted paths containing spaces

## Requirements

- Python 3.9 or newer
- A C++17-compatible compiler, such as `g++` or `clang++`
- Flask
- A browser and `xdg-open` on Linux, or the platform equivalent

## Project Structure

```
FM/
├── main.cpp                 C++ core entry point
├── run.py                   Flask server and core launcher
├── Bridge.py                Python side of the file bridge
├── templates/
│   └── fm-gui.html          Browser interface
└── CORE/
    ├── include/
    │   ├── Bridge.hpp
    │   ├── commands.hpp
    │   ├── common.hpp
    │   └── utils.hpp
    └── src/
        ├── cat.cpp
        ├── cd.cpp
        ├── cp.cpp
        ├── edit.cpp
        ├── find.cpp
        ├── help.cpp
        ├── info.cpp
        ├── ls.cpp
        ├── mk.cpp
        ├── mv.cpp
        ├── oscmd.cpp
        ├── pwd.cpp
        ├── rm.cpp
        ├── rn.cpp
        └── utils.cpp
```

## Setup

Create a virtual environment and install the Python dependency:

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install Flask
```

On Windows, activate the environment with:

```powershell
.venv\Scripts\Activate.ps1
```

## Build

Build the C++ core from the repository root. The output must be named `core`
(`core.exe` on Windows), because that is the filename used by `run.py`.

### Linux / macOS

```bash
g++ -std=c++17 -pthread -I. -o core \
  main.cpp \
  CORE/src/utils.cpp CORE/src/cp.cpp CORE/src/rn.cpp CORE/src/rm.cpp \
  CORE/src/mv.cpp CORE/src/cd.cpp CORE/src/mk.cpp CORE/src/cat.cpp \
  CORE/src/find.cpp CORE/src/edit.cpp CORE/src/info.cpp CORE/src/ls.cpp \
  CORE/src/pwd.cpp CORE/src/oscmd.cpp CORE/src/help.cpp
```

### Windows

```powershell
g++ -std=c++17 -pthread -I. -o core.exe `
  main.cpp `
  CORE/src/utils.cpp CORE/src/cp.cpp CORE/src/rn.cpp CORE/src/rm.cpp `
  CORE/src/mv.cpp CORE/src/cd.cpp CORE/src/mk.cpp CORE/src/cat.cpp `
  CORE/src/find.cpp CORE/src/edit.cpp CORE/src/info.cpp CORE/src/ls.cpp `
  CORE/src/pwd.cpp CORE/src/oscmd.cpp CORE/src/help.cpp
```

Use the compiler's normal source-file separator for your shell if the
PowerShell command above is not supported by the installed toolchain.

## Run

Start the application from the repository root:

```bash
python3 run.py
```

Open [http://127.0.0.1:5000](http://127.0.0.1:5000) in a browser if it does
not open automatically. The server is bound to localhost only.

The first run creates `runtime/`, which is used for request and response
files between Flask and the C++ core. Do not run multiple instances from the
same directory, because they would share this bridge directory.

Stop the application with `Ctrl+C`. `run.py` terminates the core process when
the Python process exits normally.

## Command Reference

The web interface sends the following command forms to the C++ core:

| Command | Description |
|---|---|
| `help` | Show the available commands |
| `cp [from] [to]` | Copy a file or directory |
| `rn [oldname] [newname]` | Rename a file or directory |
| `mv [source] [destination]` | Move a file or directory |
| `rm [file/folder]` | Remove a file or directory |
| `cd [path]` | Change the current directory |
| `mk file [name]` | Create an empty file |
| `mk dir [name]` | Create a directory and missing parents |
| `cat [filename]` | Display a file's contents |
| `find [path] [filename]` | Search recursively by filename |
| `edit [tool] [filename]` | Open a file with an external editor |
| `info [path]` | Show file or directory information |
| `ls [path]` | List a directory or inspect a file |
| `pwd` | Print the current working directory |
| `oscmd [command]` | Run an operating-system command after confirmation |
| `exit` | Stop the C++ core |

Arguments containing spaces can be wrapped in single or double quotes, for
example `mk file "notes from work.txt"`.

## Local API

`run.py` exposes two localhost-only endpoints:

- `GET /api/list?path=.` lists a path through the C++ core.
- `POST /api/run` accepts a JSON body such as
  `{ "command": "pwd" }`.

The API is intended for the bundled interface and is not configured for
remote access.

## Current Development Status

The Flask/browser interface and C++ bridge are operational. The C++ core can
be built with the command above and the supported file-management commands can
be run through the local API or the bundled GUI.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for
details.
