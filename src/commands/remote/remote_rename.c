#include <argus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands/remote.h"
#include "colors.h"


ARGUS_OPTIONS(
    remote_rename_options,
    HELP_OPTION(),
    POSITIONAL_STRING(
        "old",
        HELP("Current name of the remote")
    ),
    POSITIONAL_STRING(
        "new",
        HELP("New name for the remote")
    ),
)


int remote_rename_handler(argus_t *argus, void *data) {
    (void)data;
    
    const char *old_name = argus_get(argus, "old").as_string;
    const char *new_name = argus_get(argus, "new").as_string;
    
    printf("Renaming remote '" COLOR_CYAN("%s") "' to '" COLOR_CYAN("%s") "'\n", old_name, new_name);
    printf(COLOR_BLUE("Updating remote-tracking branches:") "\n");
    printf("  " COLOR_CYAN("%s/main") " -> " COLOR_CYAN("%s/main") "\n", old_name, new_name);
    printf("  " COLOR_CYAN("%s/develop") " -> " COLOR_CYAN("%s/develop") "\n", old_name, new_name);
    
    return 0;
}