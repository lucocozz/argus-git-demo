#include <argus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands/remote.h"
#include "colors.h"


ARGUS_OPTIONS(
    remote_get_url_options,
    HELP_OPTION(),
    OPTION_FLAG(0, "push", HELP("Query push URLs rather than fetch URLs")),
    OPTION_FLAG(0, "all", HELP("List all URLs for the remote")),
    POSITIONAL_STRING(
        "name",
        HELP("Name of the remote")
    ),
)


int remote_get_url_handler(argus_t *argus, void *data) {
    (void)data;
    
    const char *name = argus_get(argus, "name").as_string;
    bool push = argus_get(argus, "push").as_bool;
    bool all = argus_get(argus, "all").as_bool;
    
    (void)name;
    
    if (all) {
        printf(COLOR_BLUE("https://github.com/user/repo.git") "\n");
        printf(COLOR_BLUE("git@github.com:user/repo.git") "\n");
    } else {
        if (push) printf(COLOR_BLUE("https://github.com/user/repo.git") "\n");
        else printf(COLOR_BLUE("https://github.com/user/repo.git") "\n");
    }
    
    return 0;
}