#include <argus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands/remote.h"
#include "colors.h"


ARGUS_OPTIONS(
    remote_prune_options,
    HELP_OPTION(),
    OPTION_FLAG('n', "dry-run", HELP("Report what will be pruned, but do not actually prune")),
    POSITIONAL_STRING(
        "name",
        HELP("Name of the remote to prune")
    ),
)


int remote_prune_handler(argus_t *argus, void *data) {
    (void)data;
    
    const char *name = argus_get(argus, "name").as_string;
    bool dry_run = argus_get(argus, "dry-run").as_bool;
    
    if (dry_run) {
        printf(COLOR_YELLOW("Dry run mode: would prune stale references from '") COLOR_CYAN("%s") "'\n", name);
        printf(COLOR_YELLOW("Would prune:") "\n");
        printf("  * " COLOR_YELLOW("[would prune]") " " COLOR_CYAN("%s/feature-old-branch") "\n", name);
        printf("  * " COLOR_YELLOW("[would prune]") " " COLOR_CYAN("%s/hotfix-123") "\n", name);
    } else {
        printf("Pruning " COLOR_CYAN("%s") "\n", name);
        printf("URL: " COLOR_BLUE("https://github.com/user/repo.git") "\n");
        printf(" * " COLOR_GREEN("[pruned]") " " COLOR_CYAN("%s/feature-old-branch") "\n", name);
        printf(" * " COLOR_GREEN("[pruned]") " " COLOR_CYAN("%s/hotfix-123") "\n", name);
    }
    
    return 0;
}