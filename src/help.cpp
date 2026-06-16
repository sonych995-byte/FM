#include "../include/common.hpp"
#include "../include/utils.hpp"

void cmd_help() {
    clear_screen();
    std::cout << "\n\n" << R"(
        All Command:

        1. cp (copy)

        Syntax:
         - cp [from] [to]

        Example:
         - cp test.cpp main.cpp

        Output:
        Copy a file or directory from [from] to [to].

        2. rn (rename)

        Syntax:
         - rn [oldname] [newname]

        Example:
         - rn test.cpp main.cpp

        Output:
        Rename or move [oldname] to [newname].

        3. mv (move)

        Syntax:
         - mv [source] [destination]

        Example:
         - mv temp.txt backup.txt

        Output:
        Move or rename [source] to [destination].

        4. rm (remove)

        Syntax:
         - rm [filename]

        Example:
         - rm useless.cpp

        Output:
        Delete the file or directory named [filename].

        5. cd

        Syntax:
         - cd [path]

        Example:
         - cd storage/myfolder

        Output:
         Change the current directory to [path].

        6. mk (make)

        Syntax:
         - mk file [name]
         - mk dir [name]

        Example:
         - mk file src.cpp
         - mk dir myfolder

        Output:
        Create an empty file or directory named [name].

        7. info

        Syntax:
         - info [file name/dir name]

        Example:
         - info src

        Output:
        View detailed information about a file or folder.

        8. ls (list)

        Syntax:
         - ls
         - ls [path]

        Example:
         - ls
         - ls storage/myfolder

        Output:
        List contents of a directory or inspect a single file.

        9. cat

        Syntax:
         - cat [filename]

        Example:
         - cat README.md

        Output:
        Display the contents of a text file.

        10. find

        Syntax:
         - find [path] [filename]

        Example:
         - find . main.cpp

        Output:
        Search recursively for files by name.

        11. edit

        Syntax:
         - edit [tool] [filename]

        Example:
         - edit code main.cpp

        Output:
        Open [filename] with the specified external editor tool.

        12. pwd

        Syntax:
         - pwd

        Example:
         - pwd

        Output:
        Print the current working directory.

        13. oscmd

        Syntax:
         - oscmd [os command]

        Example:
         - oscmd 'pkg install git'

        Output:
        Run a shell command after confirmation.

        14. help

        Syntax:
         - help

        Example:
         - help

        Output:
        Display this help message with all available commands.)";
    pause();
}
