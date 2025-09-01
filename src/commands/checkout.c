#include <argus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "commands/git.h"
#include "colors.h"
#include "git_types.h"

ARGUS_OPTIONS(
    checkout_options,
    HELP_OPTION(),
    
    GROUP_START("Branch options"),
        OPTION_FLAG('b', NULL, HELP("Create and checkout a new branch")),
        OPTION_FLAG('B', NULL, HELP("Create/reset and checkout a branch")),
        OPTION_STRING('\0', "orphan", HELP("Create a new orphan branch")),
        OPTION_FLAG('l', NULL, HELP("Create reflog for new branch")),
        OPTION_FLAG('\0', "detach", HELP("Detach HEAD at named commit")),
        OPTION_STRING('t', "track",
            HELP("Set upstream info for new branch"),
            VALIDATOR(V_CHOICE_STR("direct", "inherit")),
            FLAGS(FLAG_OPTIONAL)),
        OPTION_FLAG('\0', "no-track", HELP("Do not set upstream info")),
        OPTION_FLAG('\0', "guess", HELP("If <branch> is not found but there does exist a tracking branch")),
        OPTION_FLAG('\0', "no-guess", HELP("Do not try to match remote branches")),
    GROUP_END(),
    
    GROUP_START("File options"),
        OPTION_FLAG('f', "force", HELP("Force checkout (throw away local modifications)")),
        OPTION_FLAG('\0', "overwrite-ignore", HELP("Overwrite ignored files")),
        OPTION_FLAG('m', "merge", HELP("Perform a 3-way merge with the new branch")),
        OPTION_STRING('\0', "conflict",
            HELP("Same as --merge, but change conflicted hunks display"),
            VALIDATOR(V_CHOICE_STR("merge", "diff3", "zdiff3")),
            ),
        OPTION_FLAG('p', "patch", HELP("Interactively select hunks in the difference")),
        OPTION_FLAG('\0', "ignore-skip-worktree-bits", HELP("Do not limit pathspecs to sparse entries only")),
        OPTION_FLAG('\0', "pathspec-from-stdin", HELP("Read pathspec from stdin")),
    GROUP_END(),
    
    GROUP_START("Display options"),
        OPTION_FLAG('q', "quiet", HELP("Suppress feedback messages")),
        OPTION_FLAG('\0', "progress", HELP("Force progress reporting")),
        OPTION_FLAG('\0', "no-progress", HELP("Do not show progress")),
        OPTION_STRING('\0', "recurse-submodules",
            HELP("Control recursive updating of submodules"),
            VALIDATOR(V_CHOICE_STR("yes", "no")),
            FLAGS(FLAG_OPTIONAL),
            ),
        OPTION_FLAG('\0', "no-recurse-submodules", HELP("Disable recursive updating of submodules")),
        OPTION_FLAG('\0', "overlay", HELP("Use overlay mode")),
        OPTION_FLAG('\0', "no-overlay", HELP("Use no-overlay mode")),
    GROUP_END(),
    
    POSITIONAL_STRING("tree-ish", HELP("Tree-ish to checkout from"), FLAGS(FLAG_OPTIONAL)),
    POSITIONAL_MANY_STRING("pathspec", HELP("Paths to checkout"), FLAGS(FLAG_OPTIONAL)),
)

static int handle_patch_mode(argus_t *argus)
{
    bool patch = argus_get(argus, "patch").as_bool;
    
    if (patch) {
        printf("Applying patch to working tree...\n");
        printf("diff --git a/src/main.c b/src/main.c\n");
        printf("Apply this hunk to working tree [y,n,q,a,d,e,?]? y\n");
        return 0;
    }
    return -1;
}

static int handle_orphan_branch(argus_t *argus, const char *orphan)
{
    bool quiet = argus_get(argus, "quiet").as_bool;
    
    if (orphan) {
        if (!quiet)
            printf("Switched to a new branch '" COLOR_GREEN("%s") "'\n", orphan);
        return 0;
    }
    return -1;
}

static int handle_branch_creation(argus_t *argus, const char *tree_ish)
{
    bool create_branch = argus_get(argus, "b").as_bool;
    bool force_create = argus_get(argus, "B").as_bool;
    bool quiet = argus_get(argus, "quiet").as_bool;
    const char *track_mode = argus_get(argus, "track").as_string;
    
    if (create_branch || force_create) {
        if (!tree_ish) {
            printf(COLOR_RED("error: ") "option '-b' requires a value\n");
            return 1;
        }
        
        if (force_create) {
            if (!quiet) {
                printf("Reset branch '" COLOR_GREEN("%s") "'\n", tree_ish);
                printf("Your branch is now at " COLOR_YELLOW("abc1234") " " COLOR_BLUE("Initial commit") "\n");
            }
        } else {
            if (!quiet)
                printf("Switched to a new branch '" COLOR_GREEN("%s") "'\n", tree_ish);
        }
        
        if (track_mode && !quiet)
            printf("Branch '" COLOR_GREEN("%s") "' set up to track remote branch 'main' from 'origin' by %s.\n", 
                   tree_ish, track_mode);
        
        return 0;
    }
    return -1;
}

static int handle_detached_head(argus_t *argus, const char *tree_ish)
{
    bool detach = argus_get(argus, "detach").as_bool;
    bool quiet = argus_get(argus, "quiet").as_bool;
    
    if (detach && tree_ish) {
        if (!quiet) {
            printf("Note: switching to '" COLOR_YELLOW("%s") "'.\n\n", tree_ish);
            printf("You are in 'detached HEAD' state. You can look around, make experimental\n");
            printf("changes and commit them, and you can discard any commits you make in this\n");
            printf("state without impacting any branches by switching back to a branch.\n\n");
            printf("HEAD is now at " COLOR_YELLOW("abc1234") " " COLOR_BLUE("Add new feature") "\n");
        }
        return 0;
    }
    return -1;
}

static int handle_file_checkout(argus_t *argus)
{
    bool merge = argus_get(argus, "merge").as_bool;
    bool force = argus_get(argus, "force").as_bool;
    bool quiet = argus_get(argus, "quiet").as_bool;
    bool progress = argus_get(argus, "progress").as_bool;
    const char *conflict_style = argus_get(argus, "conflict").as_string;
    
    if (argus_is_set(argus, "pathspec")) {
        if (merge && !quiet)
            printf("Merging changes to files...\n");
        
        if (conflict_style && !quiet)
            printf("Using conflict style: %s\n", conflict_style);
        
        argus_array_it_t it = argus_array_it(argus, "pathspec");
        int file_count = 0;
        
        while (argus_array_next(&it)) {
            const char *pathspec = it.value.as_string;
            file_count++;
            
            if (!quiet || progress) {
                if (force)
                    printf("Checked out '%s' (" COLOR_YELLOW("local modifications overwritten") ")\n", pathspec);
                else
                    printf("Updated '%s'\n", pathspec);
            }
        }
        
        if (!quiet && file_count > 0)
            printf("Updated %d file(s)\n", file_count);
        
        return 0;
    }
    return -1;
}

static int switch_branch(argus_t *argus, const char *tree_ish)
{
    bool quiet = argus_get(argus, "quiet").as_bool;
    bool progress = argus_get(argus, "progress").as_bool;
    
    if (tree_ish) {
        if (!quiet) {
            printf("Switched to branch '" COLOR_GREEN("%s") "'\n", tree_ish);
            printf("Your branch is up to date with 'origin/%s'.\n", tree_ish);
        }
        
        if (progress)
            printf("Updating files: 100%% (25/25), done.\n");
        
        return 0;
    }
    return -1;
}

static void show_current_status(void)
{
    printf("On branch " COLOR_GREEN("main") "\n");
    printf("Your branch is up to date with '" COLOR_CYAN("origin/main") "'.\n\n");
    printf("nothing to commit, working tree clean\n");
}

int checkout_handler(argus_t *argus, void *data)
{
    (void)data;
    
    const char *tree_ish = argus_get(argus, "tree-ish").as_string;
    const char *orphan = argus_get(argus, "orphan").as_string;
    
    int result;
    
    result = handle_patch_mode(argus);
    if (result != -1) 
        return result;
    
    result = handle_orphan_branch(argus, orphan);
    if (result != -1) 
        return result;
    
    result = handle_branch_creation(argus, tree_ish);
    if (result != -1) 
        return result;
    
    result = handle_detached_head(argus, tree_ish);
    if (result != -1) 
        return result;
    
    result = handle_file_checkout(argus);
    if (result != -1) 
        return result;
    
    result = switch_branch(argus, tree_ish);
    if (result != -1) 
        return result;
    
    show_current_status();
    
    return 0;
}