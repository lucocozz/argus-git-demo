#include <argus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands/stash.h"
#include "colors.h"

ARGUS_OPTIONS(
    stash_branch_options,
    HELP_OPTION(),
    POSITIONAL_STRING(
        "branchname",
        HELP("Name of the new branch")
    ),
    POSITIONAL_STRING(
        "stash",
        HELP("The stash entry to use"),
        FLAGS(FLAG_OPTIONAL)
    ),
)

int stash_branch_handler(argus_t *argus, void *data) {
    (void)data;
    
    const char *branchname = argus_get(argus, "branchname").as_string;
    const char *stash = argus_is_set(argus, "stash") 
        ? argus_get(argus, "stash").as_string : "stash@{0}";
    
    printf("Creating branch '" COLOR_GREEN("%s") "' from " COLOR_BLUE("%s") "...\n", branchname, stash);
    printf("Switched to a new branch '" COLOR_GREEN("%s") "'\n", branchname);
    printf("Applying " COLOR_BLUE("%s") "...\n", stash);
    printf("On branch " COLOR_GREEN("%s") "\n", branchname);
    printf(COLOR_YELLOW("Changes not staged for commit:") "\n");
    printf("  (use \"git add <file>...\" to update what will be committed)\n");
    printf("  (use \"git restore <file>...\" to discard changes in working directory)\n");
    printf("\t" COLOR_YELLOW("modified:   src/main.c") "\n");
    printf("\t" COLOR_YELLOW("modified:   src/utils.c") "\n");
    printf("Dropped " COLOR_BLUE("%s") " (was " COLOR_YELLOW("abc1234") ")\n", stash);
    
    return 0;
}