#include <argus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands/stash.h"
#include "colors.h"
#include "stash_utils.h"

ARGUS_OPTIONS(
    stash_apply_options,
    HELP_OPTION(),
    OPTION_FLAG(
        0, "index",
        HELP("Try to reinstate not only the working tree's changes, but also the index's ones")
    ),
    OPTION_FLAG(
        'q', "quiet",
        HELP("Be quiet, only report errors")
    ),
    POSITIONAL_STRING(
        "stash",
        HELP("The stash entry to apply"),
        FLAGS(FLAG_OPTIONAL)
    ),
)

int stash_apply_handler(argus_t *argus, void *data) {
    (void)data;
    
    const char *stash = get_stash_param(argus, "stash");
    bool index = argus_get(argus, "index").as_bool;
    bool quiet = argus_get(argus, "quiet").as_bool;
    
    if (!quiet) {
        printf("Applying " COLOR_BLUE("%s") "...\n", stash);
        printf("On branch " COLOR_GREEN("main") "\n");
        printf(COLOR_YELLOW("Changes not staged for commit:") "\n");
        printf("  (use \"git add <file>...\" to update what will be committed)\n");
        printf("  (use \"git restore <file>...\" to discard changes in working directory)\n");
        printf("\t" COLOR_YELLOW("modified:   src/main.c") "\n");
        printf("\t" COLOR_YELLOW("modified:   src/utils.c") "\n");
    }
    
    if (index && !quiet) printf("Restoring index state...\n");
    
    print_stash_operation_result("apply", stash, quiet);
    
    return 0;
}