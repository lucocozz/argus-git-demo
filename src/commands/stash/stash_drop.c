#include <argus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands/stash.h"
#include "colors.h"

ARGUS_OPTIONS(
    stash_drop_options,
    HELP_OPTION(),
    OPTION_FLAG(
        'q', "quiet",
        HELP("Be quiet, only report errors")
    ),
    POSITIONAL_STRING(
        "stash",
        HELP("The stash entry to drop"),
        FLAGS(FLAG_OPTIONAL)
    ),
)

int stash_drop_handler(argus_t *argus, void *data) {
    (void)data;
    
    const char *stash = argus_is_set(argus, "stash") 
        ? argus_get(argus, "stash").as_string : "stash@{0}";
    bool quiet = argus_get(argus, "quiet").as_bool;
    
    if (!quiet) printf("Dropped " COLOR_BLUE("%s") " (was " COLOR_YELLOW("abc1234") ")\n", stash);
    
    return 0;
}