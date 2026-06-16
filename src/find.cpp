#include "../include/common.hpp"
#include "../include/utils.hpp"

// =====================================
// Search function
// =====================================

bool search_file(
    const fs::path& path,

    const std::string& target,

    int& count
) {
    bool found = false;

    try {

        for (const auto& entry :

             fs::recursive_directory_iterator(

                path,

                fs::directory_options::
                skip_permission_denied
             ))
        {
            if (entry.path().filename()
                == target)
            {
                found = true;

                ++count;

                std::cout

                    << entry.path()

                    << '\n';
            }
        }
    }

    catch (...) {}

    return found;
}

// =====================================
// Command
// =====================================

void cmd_find(
    const std::vector<std::string>& args
) {

    if (args.size() != 3) {

        std::cout

            << "\n\nUsage: find [path] [filename]";

        pause();

        return;
    }

    std::string target = args[2];

    int count = 0;

    bool found = false;

    std::cout

        << "\nSearching...\n\n";

    try {

        // ==========================
        // Search entire computer
        // ==========================

        if (args[1] == "/") {

#if defined(_WIN32)

            for (
                char drive = 'A';

                drive <= 'Z';

                ++drive
            ) {

                std::string drive_path =

                    std::string(1, drive)

                    + ":\\";

                if (!fs::exists(
                        drive_path
                    ))
                {
                    continue;
                }

                if (

                    search_file(

                        drive_path,

                        target,

                        count
                    )
                ) {

                    found = true;
                }
            }

#else

            if (

                search_file(

                    "/",

                    target,

                    count
                )
            ) {

                found = true;
            }

#endif

        }

        // ==========================
        // Search custom path
        // ==========================

        else {

            fs::path search_path;

            if (args[1] == ".") {

                search_path =

                    fs::current_path();
            }

            else if (
                args[1] == ".."
            ) {

                search_path =

                    fs::current_path()

                    .parent_path();
            }

            else {

                search_path =

                    args[1];
            }

            // ======================
            // Check path
            // ======================

            if (

                !fs::exists(

                    search_path
                )
            ) {

                std::cout

                    << "\nPath not found";

                pause();

                return;
            }

            if (

                search_file(

                    search_path,

                    target,

                    count
                )
            ) {

                found = true;
            }
        }

        // ==========================
        // Result
        // ==========================

        if (!found) {

            std::cout

                << "\nFile not found\n";
        }

        else {

            std::cout

                << "\nFound "

                << count

                << " file(s)\n";
        }

        show_success();

        pause();

        return;
    }

    catch (
        const fs::filesystem_error& e
    ) {

        show_error(e);

        pause();

        return;
    }
}