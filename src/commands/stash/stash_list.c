#include <argus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands/stash.h"
#include "colors.h"
#include "git_types.h"
#include "mock_data.h"

ARGUS_OPTIONS(
    stash_list_options,
    HELP_OPTION(),
    OPTION_FLAG(0, "oneline", HELP("Show stash entries in oneline format")),
    OPTION_STRING(0, "pretty", HELP("Pretty-print format")),
)

int stash_list_handler(argus_t *argus, void *data) {
    (void)data;
    
    bool oneline = argus_get(argus, "oneline").as_bool;
    
    int stash_count;
    const git_stash_entry_t *stashes = get_mock_stashes(&stash_count);
    
    for (int i = 0; i < stash_count; i++) {
        const git_stash_entry_t *stash = &stashes[i];
        
        if (oneline)
            printf(COLOR_BLUE("stash@{%d}:") " %s\n", stash->index, stash->description);
        else
            printf(COLOR_BLUE("stash@{%d}:") " %s\n", stash->index, stash->description);
    }
    
    return 0;
}