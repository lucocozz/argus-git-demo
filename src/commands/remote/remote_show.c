#include <argus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands/remote.h"
#include "colors.h"
#include "git_types.h"
#include "mock_data.h"

ARGUS_OPTIONS(
    remote_show_options,
    HELP_OPTION(),
    OPTION_FLAG('n', NULL, HELP("Do not query remotes, only use cached information")),
    POSITIONAL_STRING("name", HELP("Name of the remote to show"), DEFAULT("origin"), FLAGS(FLAG_OPTIONAL)),
)

int remote_show_handler(argus_t *argus, void *data) {
    (void)data;
    
    bool no_query = argus_get(argus, "n").as_bool;
    const char *name = argus_get(argus, "name").as_string;
    
    int remote_count;
    const git_remote_t *remotes = get_mock_remotes(&remote_count);
    const git_remote_t *remote = NULL;
    
    for (int i = 0; i < remote_count; i++) {
        if (strcmp(remotes[i].name, name) == 0) {
            remote = &remotes[i];
            break;
        }
    }
    
    if (!remote) {
        printf(COLOR_RED("error: ") "No such remote '%s'\n", name);
        return 1;
    }
    
    printf("* remote " COLOR_CYAN("%s") "\n", remote->name);
    printf("  Fetch URL: " COLOR_BLUE("%s") "\n", remote->url);
    printf("  Push  URL: " COLOR_BLUE("%s") "\n", remote->push_url);
    printf("  HEAD branch: " COLOR_GREEN("main") "\n");
    
    if (!no_query) {
        int branch_count;
        const git_branch_t *branches = get_mock_remote_branches(&branch_count);
        
        printf(COLOR_BLUE("  Remote branches:") "\n");
        for (int i = 0; i < branch_count && i < 3; i++) {
            printf("    " COLOR_GREEN("%s") " tracked\n", strchr(branches[i].name, '/') + 1);
        }
        
        printf(COLOR_BLUE("  Local branches configured for 'git pull':") "\n");
        printf("    " COLOR_GREEN("main") " merges with remote " COLOR_GREEN("main") "\n");
        printf(COLOR_BLUE("  Local refs configured for 'git push':") "\n");
        printf("    " COLOR_GREEN("main") " pushes to " COLOR_GREEN("main") " (up to date)\n");
    }
    
    return 0;
}