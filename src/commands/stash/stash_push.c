#include <argus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands/stash.h"
#include "colors.h"


ARGUS_OPTIONS(
    stash_push_options,
    HELP_OPTION(),
    
    // Message option
    OPTION_STRING('m', "message", HELP("Use the given message as stash description"), HINT("msg")),
    
    // Include options
    OPTION_FLAG('u', "include-untracked", HELP("Include untracked files in the stash")),
    OPTION_FLAG('a', "all",               HELP("Include ignored files in the stash")),
    
    // Behavior options
    OPTION_FLAG('k', "keep-index", HELP("Keep the index")),
    OPTION_FLAG('p', "patch",      HELP("Interactively select hunks")),
    OPTION_FLAG('S', "staged",     HELP("Stash only currently staged changes")),
    
    // Positional arguments
    POSITIONAL_STRING("pathspec", HELP("Limit stash to given pathspec"), FLAGS(FLAG_OPTIONAL)),
)


int stash_push_handler(argus_t *argus, void *data)
{
    (void)data;
    

    const char *message         = argus_is_set(argus, "message") 
                                  ? argus_get(argus, "message").as_string : NULL;
    bool        include_untracked = argus_get(argus, "include-untracked").as_bool;
    bool        all             = argus_get(argus, "all").as_bool;
    bool        keep_index      = argus_get(argus, "keep-index").as_bool;
    bool        patch           = argus_get(argus, "patch").as_bool;
    bool        staged          = argus_get(argus, "staged").as_bool;
    

    // Handle pathspec limitation
    if (argus_is_set(argus, "pathspec")) {
        const char *pathspec = argus_get(argus, "pathspec").as_string;
        printf("Stashing changes in: " COLOR_BLUE("%s") "\n", pathspec);
    }
    
    // Display stash operation type
    if (staged)
        printf(COLOR_BLUE("Stashing only staged changes...") "\n");
    else if (patch)
        printf(COLOR_BLUE("Interactively selecting hunks to stash...") "\n");
    else
        printf(COLOR_BLUE("Stashing working directory changes...") "\n");
    
    // Display additional options
    if (include_untracked)
        printf(COLOR_BLUE("Including untracked files") "\n");
    if (all)
        printf(COLOR_BLUE("Including ignored files") "\n");
    if (keep_index)
        printf(COLOR_BLUE("Keeping index unchanged") "\n");
    

    // Display simulated Git results and cleanup
    const char *stash_msg = message ? message : "WIP on main: abc1234 Add new feature";
    printf(COLOR_GREEN("Saved working directory and index state") " \"%s\"\n", stash_msg);
    
    return 0;
}