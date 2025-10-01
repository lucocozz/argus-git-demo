#include <argus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands/remote.h"
#include "colors.h"


ARGUS_OPTIONS(
    remote_remove_options,
    HELP_OPTION(),
    POSITIONAL_STRING(
        "name",
        HELP("Name of the remote to remove")
    ),
)

int remote_remove_handler(argus_t *argus, void *data) {
    (void)data;
    
    const char *name = argus_get(argus, "name").as_string;
    
    printf("Removing remote '" COLOR_CYAN("%s") "'\n", name);
    printf(COLOR_YELLOW("Note: all remote-tracking branches for '") COLOR_CYAN("%s") COLOR_YELLOW("' will be deleted") "\n", name);
    
    return 0;
}