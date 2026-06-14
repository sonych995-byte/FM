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
        Create a new file named [to] with contents belonging to [from].

        2. rn (rename)

        Syntax:
         - rn [oldname] [newname]

        Example:
         - rn test.cpp main.cpp

        Output:
        Rename the file [oldname] to [newname].

        3. rm (remove)

        Syntax:
         - rm [filename]

        Example:
         - rm useless.cpp

        Output:
        Delete the file named [filename].

        4. cd
        Syntax:
         - cd [path]

        Example:
         - cd storage/myfolder
         
        Output:
         Go to path [path]
         
        5. mk (make)
        
        Syntax:
         - mk file [name]
         - mk dir [name]
         
        Example:
       Program  - mk file src.cpp
         - mk dir myfolder
         
        Output:
        If the command is `mk file [name]`, it will create an empty file named [name]. If the command is `mk dir [name]`, it will create an empty folder named [name].
        
        6. info
        
        Syntax:
         - info [file name/dir name]
         
        Example:
         - info src
         
        Output:
        View detailed information about a file or folder
        
        7. ls (list)
        
        Syntax:
         - ls
         - ls [path]
         
        Example:
         - ls
         - ls storage/myfolder
         
        Output:
        List contents of a directory
        
        8. pwd
        
        Syntax:
         - pwd
         
        Example:
         - pwd
         
        Output:
        Print the current working directory
        
        9. oscmd
        
        Syntax:
         - oscmd [os command]
         
        Example:
         - oscmd 'pkg install git'
         
        Output:
        Use operating system commands, which require single or double quotes, as shown in the example.)";
        pause();
    }