#include <argus.h>
#include <argus/regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands/remote.h"
#include "colors.h"
#include "mock_data.h"

ARGUS_OPTIONS(
    remote_add_options,
    HELP_OPTION(),
    OPTION_FLAG('f', "fetch", HELP("Fetch from remote after adding")),
    OPTION_FLAG(0, "tags", HELP("Import all tags")),
    POSITIONAL_STRING("name", HELP("Remote name")),
    POSITIONAL_STRING("url", HELP("Remote URL"), VALIDATOR(V_REGEX(ARGUS_RE_URL))),
)

int remote_add_handler(argus_t *argus, void *data) {
    (void)data;
    
    const char *name = argus_get(argus, "name").as_string;
    const char *url = argus_get(argus, "url").as_string;
    bool fetch = argus_get(argus, "fetch").as_bool;
    bool tags = argus_get(argus, "tags").as_bool;
    
    printf("Adding remote '" COLOR_CYAN("%s") "' with URL '" COLOR_BLUE("%s") "'\n", name, url);
    
    if (fetch) {
        printf("Fetching from " COLOR_CYAN("%s") "...\n", name);
        printf("From " COLOR_BLUE("%s") "\n", url);
        
        int branch_count;
        const git_branch_t *branches = get_mock_remote_branches(&branch_count);
        
        for (int i = 0; i < branch_count && i < 3; i++) {
            printf(" * " COLOR_GREEN("[new branch]") "      " COLOR_GREEN("%s") " -> " COLOR_CYAN("%s/%s") "\n", 
                   strchr(branches[i].name, '/') + 1, name, strchr(branches[i].name, '/') + 1);
        }
        
        if (tags)
            printf(" * [new tag]         v1.0.0 -> v1.0.0\n");
    }
    
    return 0;
}