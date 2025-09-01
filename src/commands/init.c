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

    GROUP_START("General options"),
        OPTION_FLAG('q', "quiet", HELP("Only print error and warning messages")),
        OPTION_FLAG(0, "bare", HELP("Create a bare repository")),
    GROUP_END(),
    
    GROUP_START("Configuration options"),
        OPTION_STRING('b', "initial-branch", HELP("Use the specified name for the initial branch"), 
                      DEFAULT("main")),
        OPTION_STRING(0, "template", HELP("Directory from which templates will be used")),
        OPTION_STRING(0, "separate-git-dir", HELP("Create a text file containing path to actual repository")),
    GROUP_END(),

    POSITIONAL_STRING("directory", HELP("Directory to initialize as git repository"),
                      DEFAULT("."), FLAGS(FLAG_OPTIONAL)),
)

int init_handler(argus_t *argus, void *data)
{
    (void)data;

    bool quiet = argus_get(argus, "quiet").as_bool;
    bool bare = argus_get(argus, "bare").as_bool;
    const char *initial_branch = argus_get(argus, "initial-branch").as_string;
    const char *directory = argus_get(argus, "directory").as_string;

    if (!quiet)
        printf(COLOR_GREEN("Initialized empty Git repository") " in %s/%s\n", 
               directory, bare ? "" : ".git");

    const char *template_dir = argus_get(argus, "template").as_string;
    if (template_dir)
        printf("Using template from: %s\n", template_dir);

    const char *separate_git_dir = argus_get(argus, "separate-git-dir").as_string;
    if (separate_git_dir)
        printf("Separate git directory: %s\n", separate_git_dir);

    printf("Initial branch: " COLOR_BLUE("%s") "\n", initial_branch);

    return 0;
}