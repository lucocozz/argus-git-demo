#include <argus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands/remote.h"
#include "colors.h"


ARGUS_OPTIONS(
    remote_add_options,
    HELP_OPTION(),
    OPTION_FLAG(
        'f', "fetch",
        HELP("Immediately fetch from the new remote")
    ),
    OPTION_FLAG(
        0, "tags",
        HELP("Import every tag from the remote with --fetch")
    ),
    OPTION_FLAG(
        0, "no-tags",
        HELP("Do not import tags from the remote with --fetch")
    ),
    OPTION_ARRAY_STRING(
        't', NULL,
        HELP("Instead of the default glob refspec for the remote, track only branches matching the pattern"),
        HINT("branch")
    ),
    OPTION_STRING(
        'm', NULL,
        HELP("Set the default branch for the remote"),
        HINT("master")
    ),
    POSITIONAL_STRING(
        "name",
        HELP("Name of the remote")
    ),
    POSITIONAL_STRING(
        "url", 
        HELP("URL of the remote repository")
    ),
)

int remote_add_handler(argus_t *argus, void *data) {
    (void)data;
    
    const char *name = argus_get(argus, "name").as_string;
    const char *url = argus_get(argus, "url").as_string;
    bool fetch = argus_get(argus, "fetch").as_bool;
    bool tags = argus_get(argus, "tags").as_bool;
    bool no_tags = argus_get(argus, "no-tags").as_bool;
    
    printf("Adding remote '" COLOR_CYAN("%s") "' with URL '" COLOR_BLUE("%s") "'\n", name, url);
    
    if (argus_is_set(argus, "m")) {
        printf("Setting default branch to: " COLOR_GREEN("%s") "\n", argus_get(argus, "m").as_string);
    }
    
    if (argus_is_set(argus, "t")) {
        argus_array_it_t it = argus_array_it(argus, "t");
        printf(COLOR_BLUE("Tracking branches:") "\n");
        while (argus_array_next(&it)) {
            printf("  - " COLOR_GREEN("%s") "\n", it.value.as_string);
        }
    }
    
    if (tags) {
        printf(COLOR_BLUE("Will import all tags") "\n");
    } else if (no_tags) {
        printf(COLOR_BLUE("Will not import tags") "\n");
    }
    
    if (fetch) {
        printf("Fetching from " COLOR_CYAN("%s") "...\n", name);
        printf("From " COLOR_BLUE("%s") "\n", url);
        printf(" * " COLOR_GREEN("[new branch]") "      " COLOR_GREEN("main") "     -> " COLOR_CYAN("%s/main") "\n", name);
        printf(" * " COLOR_GREEN("[new branch]") "      " COLOR_GREEN("develop") "  -> " COLOR_CYAN("%s/develop") "\n", name);
    }
    
    return 0;
}