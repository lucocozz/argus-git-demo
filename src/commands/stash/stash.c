#include <argus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands/git.h"
#include "commands/stash.h"
#include "colors.h"
#include "git_types.h"

ARGUS_OPTIONS(
    stash_options,
    HELP_OPTION(),

    OPTION_STRING('m', "message", HELP("Use the given <msg> as the stash message")),
    OPTION_FLAG('u', "include-untracked", HELP("Include untracked files in the stash")),
    OPTION_FLAG('a', "all", HELP("Include ignored files in the stash")),
    OPTION_FLAG('k', "keep-index", HELP("Keep the index")),
    OPTION_FLAG('p', "patch", HELP("Interactively select hunks from the diff between HEAD and the working tree")),

    SUBCOMMAND("push", stash_push_options,
        HELP("Save your local modifications to a new stash entry"),
        ACTION(stash_push_handler)),
    SUBCOMMAND("pop", stash_pop_options,
        HELP("Remove and apply a single stashed state from the stash list"),
        ACTION(stash_pop_handler)),
    SUBCOMMAND("apply", stash_apply_options,
        HELP("Apply a single stashed state on top of the current working tree state"),
        ACTION(stash_apply_handler)),
    SUBCOMMAND("drop", stash_drop_options,
        HELP("Remove a single stash entry from the list of stash entries"),
        ACTION(stash_drop_handler)),
    SUBCOMMAND("list", stash_list_options,
        HELP("List the stash entries that you currently have"),
        ACTION(stash_list_handler)),
    SUBCOMMAND("show", stash_show_options,
        HELP("Show the changes recorded in the stash entry"),
        ACTION(stash_show_handler)),
    SUBCOMMAND("clear", stash_clear_options,
        HELP("Remove all the stash entries"),
        ACTION(stash_clear_handler)),
    SUBCOMMAND("branch", stash_branch_options,
        HELP("Create and check out a new branch starting from the commit at which the stash entry was originally created"),
        ACTION(stash_branch_handler)),
)

int stash_handler(argus_t *argus, void *data)
{
    (void)data;
    
    if (argus_has_command(argus))
        return argus_exec(argus, NULL);
    
    const char *message = argus_get(argus, "message").as_string;
    bool include_untracked = argus_get(argus, "include-untracked").as_bool;
    bool all = argus_get(argus, "all").as_bool;
    bool keep_index = argus_get(argus, "keep-index").as_bool;
    bool patch = argus_get(argus, "patch").as_bool;
    
    if (patch) {
        printf("Interactively selecting hunks to stash...\n");
        printf("diff --git a/src/main.c b/src/main.c\n");
        printf("index abc1234..def5678 100644\n");
        printf(COLOR_RED("---") " a/src/main.c\n");
        printf(COLOR_GREEN("+++") " b/src/main.c\n");
        printf("@@ -10,6 +10,9 @@ int main() {\n");
        printf("     printf(\"Hello World\\n\");\n");
        printf("+    // TODO: Add more functionality\n");
        printf("+    printf(\"Debug: Starting application\\n\");\n");
        printf("+\n");
        printf("     return 0;\n");
        printf(" }\n");
        printf("Stash this hunk [y,n,q,a,d,e,?]? y\n");
    }
    
    printf(COLOR_BLUE("Saving current work to stash...") "\n");
    
    if (include_untracked)
        printf(COLOR_BLUE("Including untracked files in stash") "\n");
    
    if (all)
        printf(COLOR_BLUE("Including ignored files in stash") "\n");
    
    if (keep_index)
        printf(COLOR_BLUE("Keeping index unchanged") "\n");
    
    const char *stash_msg = message ? message : "WIP on main: abc1234 Add new feature";
    printf(COLOR_GREEN("Saved working directory and index state") " \"%s\"\n", stash_msg);
    
    return 0;
}