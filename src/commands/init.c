#include <argus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "commands/git.h"
#include "colors.h"
#include "git_types.h"
#include "mock_data.h"

ARGUS_OPTIONS(
    init_options,
    HELP_OPTION(),

    GROUP_START("General options"),
        OPTION_FLAG('q', "quiet", HELP("Only print error and warning messages")),
        OPTION_FLAG(0, "bare", HELP("Create a bare repository")),
    GROUP_END(),
    
    GROUP_START("Branch options"),
        OPTION_STRING('b', "initial-branch", HELP("Use the specified name for the initial branch"), 
                      DEFAULT("main")),
    GROUP_END(),

    POSITIONAL_STRING("directory", HELP("Directory to initialize as git repository"),
                      DEFAULT("."), FLAGS(FLAG_OPTIONAL)),
)

static void display_initialization_message(argus_t *argus, const char *directory)
{
    bool quiet = argus_get(argus, "quiet").as_bool;
    bool bare = argus_get(argus, "bare").as_bool;
    
    if (quiet)
        return;
    
    if (bare) {
        printf(COLOR_GREEN("Initialized empty Git repository") " in %s/\n", directory);
    } else {
        printf(COLOR_GREEN("Initialized empty Git repository") " in %s/.git/\n", directory);
    }
}

static void display_initial_branch_info(argus_t *argus, const char *initial_branch)
{
    bool quiet = argus_get(argus, "quiet").as_bool;
    
    if (quiet)
        return;
    
    printf("\n");
    printf("Using '" COLOR_GREEN("%s") "' as the name for the initial branch. ", initial_branch);
    printf("This default branch name\n");
    printf("can be changed via 'git config init.defaultBranch <name>'\n");
}

static int handle_bare_repository(argus_t *argus, const char *directory, const char *initial_branch)
{
    bool bare = argus_get(argus, "bare").as_bool;
    bool quiet = argus_get(argus, "quiet").as_bool;
    
    if (bare) {
        display_initialization_message(argus, directory);
        
        if (!quiet) {
            printf("\nBare repository initialized with no working directory.\n");
            printf("Initial branch: " COLOR_BLUE("%s") "\n", initial_branch);
        }
        return 0;
    }
    return -1;
}

static void initialize_standard_repository(argus_t *argus, const char *directory, const char *initial_branch)
{
    display_initialization_message(argus, directory);
    display_initial_branch_info(argus, initial_branch);
}

int init_handler(argus_t *argus, void *data)
{
    (void)data;

    const char *initial_branch = argus_get(argus, "initial-branch").as_string;
    const char *directory = argus_get(argus, "directory").as_string;
    
    int result;
    
    if ((result = handle_bare_repository(argus, directory, initial_branch)) != -1)
        return result;
    
    initialize_standard_repository(argus, directory, initial_branch);
    return 0;
}