#include <argus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands/stash.h"
#include "colors.h"

ARGUS_OPTIONS(
    stash_push_options,
    HELP_OPTION(),
    
    OPTION_STRING('m', "message", HELP("Stash description message")),
    OPTION_FLAG('u', "include-untracked", HELP("Include untracked files")),
    OPTION_FLAG('k', "keep-index", HELP("Keep the index")),
    POSITIONAL_STRING("pathspec", HELP("Limit stash to pathspec"), FLAGS(FLAG_OPTIONAL)),
)

int stash_push_handler(argus_t *argus, void *data)
{
    (void)data;
    
    const char *message = argus_is_set(argus, "message") 
                          ? argus_get(argus, "message").as_string : NULL;
    bool include_untracked = argus_get(argus, "include-untracked").as_bool;
    bool keep_index = argus_get(argus, "keep-index").as_bool;
    
    if (argus_is_set(argus, "pathspec")) {
        const char *pathspec = argus_get(argus, "pathspec").as_string;
        printf("Stashing changes in: " COLOR_BLUE("%s") "\n", pathspec);
    }
    
    printf(COLOR_BLUE("Stashing working directory changes...") "\n");
    
    if (include_untracked)
        printf(COLOR_BLUE("Including untracked files") "\n");
    if (keep_index)
        printf(COLOR_BLUE("Keeping index unchanged") "\n");
    
    const char *stash_msg = message ? message : "WIP on main: abc1234 Add new feature";
    printf(COLOR_GREEN("Saved working directory and index state") " \"%s\"\n", stash_msg);
    
    return 0;
}