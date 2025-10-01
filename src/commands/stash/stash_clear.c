#include <argus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands/stash.h"
#include "colors.h"

ARGUS_OPTIONS(
    stash_clear_options,
    HELP_OPTION(),
)

int stash_clear_handler(argus_t *argus, void *data) {
    (void)data;
    (void)argus;
    
    printf(COLOR_BLUE("Clearing all stash entries...") "\n");
    printf(COLOR_GREEN("Removed 3 stash entries") "\n");
    
    return 0;
}