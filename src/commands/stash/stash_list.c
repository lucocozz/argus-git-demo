#include <argus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands/stash.h"
#include "colors.h"
#include "git_types.h"


ARGUS_OPTIONS(
    stash_list_options,
    HELP_OPTION(),
    
    // Display options
    OPTION_FLAG(0, "oneline", HELP("Show stash entries in oneline format")),
    OPTION_STRING(0, "pretty", HELP("Pretty-print format"), HINT("format")),
)


int stash_list_handler(argus_t *argus, void *data) 
{
    (void)data;
    
    // Parse options
    bool oneline = argus_get(argus, "oneline").as_bool;
    
    const git_stash_entry_t stashes[] = {
        {0, "WIP on main: abc1234 Add new feature for user authentication", "main", "2024-01-15 10:30:00"},
        {1, "On feature-branch: def5678 Fix bug in payment processing", "feature-branch", "2024-01-14 15:45:00"},
        {2, "WIP on main: ghi9012 Update documentation for API endpoints", "main", "2024-01-13 09:15:00"}
    };
    int stash_count = 3;
    
    for (int i = 0; i < stash_count; i++) {
        const git_stash_entry_t *stash = &stashes[i];
        
        if (oneline) {
            printf(COLOR_BLUE("stash@{%d}:") " %s\n", stash->index, stash->description);
        } else {
            printf(COLOR_BLUE("stash@{%d}:") " %s\n", stash->index, stash->description);
        }
    }
    
    return 0;
}