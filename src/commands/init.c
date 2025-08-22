#include <argus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "commands/git.h"
#include "colors.h"
#include "git_types.h"


ARGUS_OPTIONS(
    init_options,
    HELP_OPTION(),

    // Behavior options
    OPTION_FLAG('q', "quiet", HELP("Only print error and warning messages")),
    OPTION_FLAG(0, "bare",    HELP("Create a bare repository")),

    // Configuration options
    OPTION_STRING(
        'b', "initial-branch",
        HELP("Use the specified name for the initial branch"), 
        DEFAULT("main"),
        HINT("branch-name")),
    OPTION_STRING(
        0, "template",
        HELP("Directory from which templates will be used"), 
        HINT("DIR")),
    OPTION_STRING(
        0, "separate-git-dir",
        HELP("Create a text file containing path to actual repository"), 
        HINT("git-dir")),

    // Positional arguments
    POSITIONAL_STRING(
        "directory",
        HELP("Directory to initialize as git repository"),
        DEFAULT("."),
        HINT("DIR"),
        FLAGS(FLAG_OPTIONAL)),
)


int init_handler(argus_t *argus, void *data)
{
    (void)data;

    bool        quiet          = argus_get(argus, "quiet").as_bool;
    bool        bare           = argus_get(argus, "bare").as_bool;
    const char *initial_branch = argus_get(argus, "initial-branch").as_string;
    const char *directory      = argus_get(argus, "directory").as_string;

    if (!quiet)
        printf(COLOR_GREEN("Initialized empty Git repository") " in %s/%s\n", 
               directory, bare ? "" : ".git");


    if (argus_is_set(argus, "template"))
        printf("Using template from: %s\n", argus_get(argus, "template").as_string);

    if (argus_is_set(argus, "separate-git-dir"))
        printf("Separate git directory: %s\n", argus_get(argus, "separate-git-dir").as_string);

    printf("Initial branch: " COLOR_BLUE("%s") "\n", initial_branch);

    return 0;
}