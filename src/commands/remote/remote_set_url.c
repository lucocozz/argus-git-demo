#include <argus.h>
#include <argus/regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands/remote.h"
#include "colors.h"

ARGUS_OPTIONS(
    remote_set_url_options,
    HELP_OPTION(),
    OPTION_FLAG(0, "push", HELP("Set push URL instead of fetch URL")),
    OPTION_FLAG(0, "add", HELP("Instead of changing existing URLs, add new URL")),
    OPTION_FLAG(0, "delete", HELP("Instead of changing existing URLs, delete matching URLs")),
    POSITIONAL_STRING(
        "name",
        HELP("Name of the remote")
    ),
    POSITIONAL_STRING("newurl", HELP("New URL for the remote"), VALIDATOR(V_REGEX(ARGUS_RE_URL))),
    POSITIONAL_STRING("oldurl", HELP("Old URL to replace (for --delete)"), VALIDATOR(V_REGEX(ARGUS_RE_URL)), FLAGS(FLAG_OPTIONAL)),
)

int remote_set_url_handler(argus_t *argus, void *data) {
    (void)data;
    
    const char *name = argus_get(argus, "name").as_string;
    const char *newurl = argus_get(argus, "newurl").as_string;
    bool push = argus_get(argus, "push").as_bool;
    bool add = argus_get(argus, "add").as_bool;
    bool delete = argus_get(argus, "delete").as_bool;
    
    if (add) printf("Adding URL '" COLOR_BLUE("%s") "' to remote '" COLOR_CYAN("%s") "'\n", newurl, name);
    else if (delete) printf("Deleting URL '" COLOR_BLUE("%s") "' from remote '" COLOR_CYAN("%s") "'\n", newurl, name);
    else printf("Setting " COLOR_BLUE("%s") " URL for remote '" COLOR_CYAN("%s") "' to '" COLOR_BLUE("%s") "'\n", 
               push ? "push" : "fetch", name, newurl);
    
    return 0;
}