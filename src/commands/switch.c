#include <argus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "commands/git.h"
#include "colors.h"
#include "git_types.h"

ARGUS_OPTIONS(
    switch_options,
    HELP_OPTION(),
    
    GROUP_START("Branch creation"),
        OPTION_FLAG('c', "create", HELP("Create a new branch and switch to it")),
        OPTION_FLAG('C', "force-create", HELP("Similar to --create except if <new-branch> already exists, reset it")),
        OPTION_STRING('\0', "orphan", HELP("Create a new orphan branch")),
        OPTION_FLAG('\0', "ignore-other-worktrees", HELP("Do not check if another worktree is holding the given ref")),
    GROUP_END(),
    
    GROUP_START("Branch tracking"),
        OPTION_STRING('t', "track",
            HELP("Set upstream info for new branch"),
            VALIDATOR(V_CHOICE_STR("direct", "inherit")),
            FLAGS(FLAG_OPTIONAL)),
        OPTION_FLAG('\0', "no-track", HELP("Do not set upstream info")),
        OPTION_FLAG('\0', "guess", HELP("If <branch> is not found but there does exist a tracking branch")),
        OPTION_FLAG('\0', "no-guess", HELP("Do not try to match remote branches")),
    GROUP_END(),
    
    GROUP_START("Worktree options"),
        OPTION_FLAG('f', "force", HELP("Throw away local modifications")),
        OPTION_FLAG('\0', "discard-changes", HELP("Proceed even if index or working tree differs from HEAD")),
        OPTION_FLAG('m', "merge", HELP("Perform a 3-way merge between current branch, working tree and new branch")),
        OPTION_STRING('\0', "conflict",
            HELP("Same as --merge, but change conflicted hunks display"),
            VALIDATOR(V_CHOICE_STR("merge", "diff3", "zdiff3")),
            ),
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
    GROUP_END(),
    
    POSITIONAL_STRING("branch", HELP("Branch to switch to"), FLAGS(FLAG_OPTIONAL)),
    POSITIONAL_STRING("start-point", HELP("Start point for new branch"), FLAGS(FLAG_OPTIONAL)),
)

static int validate_branch_switch(argus_t *argus, const char *branch)
{
    (void)argus;
    if (!branch) {
        printf(COLOR_RED("error: ") "missing branch or commit argument\n");
        printf("\nusage: git switch [<options>] [<branch>]\n");
        printf("   or: git switch [<options>] --create <branch> [<start-point>]\n");
        printf("   or: git switch [<options>] --detach [<start-point>]\n");
        printf("   or: git switch [<options>] --orphan <new-branch>\n");
        return 1;
    }
    return -1;
}

static int handle_orphan_branch(argus_t *argus, const char *orphan)
{
    bool quiet = argus_get(argus, "quiet").as_bool;
    
    if (orphan) {
        if (!quiet) {
            printf("Switched to a new branch '" COLOR_GREEN("%s") "'\n", orphan);
            printf("You are now on an orphan branch. Your first commit will start a new history.\n");
        }
        return 0;
    }
    return -1;
}

static int create_new_branch(argus_t *argus, const char *branch)
{
    bool create = argus_get(argus, "create").as_bool;
    bool force_create = argus_get(argus, "force-create").as_bool;
    bool quiet = argus_get(argus, "quiet").as_bool;
    bool progress = argus_get(argus, "progress").as_bool;
    const char *track_mode = argus_get(argus, "track").as_string;
    
    if (create || force_create) {
        if (!branch) {
            printf(COLOR_RED("error: ") "option '--create' requires a value\n");
            return 1;
        }
        
        if (force_create) {
            if (!quiet) {
                printf("Reset branch '" COLOR_GREEN("%s") "' (was at " COLOR_YELLOW("abc1234") ")\n", branch);
                printf("Switched to branch '" COLOR_GREEN("%s") "'\n", branch);
            }
        } else {
            if (!quiet)
                printf("Switched to a new branch '" COLOR_GREEN("%s") "'\n", branch);
        }
        
        if (track_mode && !quiet)
            printf("Branch '" COLOR_GREEN("%s") "' set up to track remote branch 'main' from 'origin' by %s.\n", 
                   branch, track_mode);
        
        if (progress)
            printf("Updating files: 100%% (15/15), done.\n");
        
        return 0;
    }
    return -1;
}

static int handle_merge_conflicts(argus_t *argus)
{
    bool merge = argus_get(argus, "merge").as_bool;
    const char *conflict_style = argus_get(argus, "conflict").as_string;
    
    if (merge) {
        if (conflict_style)
            printf("Using conflict style: %s\n", conflict_style);
        printf("Auto-merging conflicted files...\n");
        printf("CONFLICT (content): Merge conflict in src/main.c\n");
        printf("Automatic merge failed; fix conflicts and then commit the result.\n");
        return 1;
    }
    return -1;
}

static int perform_branch_switch(argus_t *argus, const char *branch)
{
    bool quiet = argus_get(argus, "quiet").as_bool;
    bool force = argus_get(argus, "force").as_bool;
    bool discard_changes = argus_get(argus, "discard-changes").as_bool;
    bool progress = argus_get(argus, "progress").as_bool;
    bool guess = argus_get(argus, "guess").as_bool;
    bool no_guess = argus_get(argus, "no-guess").as_bool;
    
    bool branch_exists = strcmp(branch, "main") == 0 || 
                        strcmp(branch, "develop") == 0 || 
                        strcmp(branch, "feature") == 0;
    
    if (!branch_exists && !no_guess && (guess || !argus_is_set(argus, "no-guess"))) {
        if (!quiet) {
            printf("Branch '" COLOR_GREEN("%s") "' set up to track remote branch '" COLOR_CYAN("%s") "' from '" COLOR_CYAN("origin") "'.\n", 
                   branch, branch);
            printf("Switched to a new branch '" COLOR_GREEN("%s") "'\n", branch);
        }
        return 0;
    } else if (!branch_exists) {
        printf(COLOR_RED("error: ") "pathspec '%s' did not match any file(s) known to git\n", branch);
        return 1;
    }
    
    if (force || discard_changes) {
        if (!quiet)
            printf(COLOR_YELLOW("warning: ") "local modifications were discarded\n");
    }
    
    if (!quiet) {
        printf("Switched to branch '" COLOR_GREEN("%s") "'\n", branch);
        printf("Your branch is up to date with 'origin/%s'.\n", branch);
    }
    
    if (progress)
        printf("Updating files: 100%% (8/8), done.\n");
    
    return 0;
}

int switch_handler(argus_t *argus, void *data)
{
    (void)data;
    
    const char *branch = argus_get(argus, "branch").as_string;
    const char *orphan = argus_get(argus, "orphan").as_string;
    
    int result;
    
    result = handle_orphan_branch(argus, orphan);
    if (result != -1) 
        return result;
    
    result = create_new_branch(argus, branch);
    if (result != -1) 
        return result;
    
    result = validate_branch_switch(argus, branch);
    if (result != -1) 
        return result;
    
    result = handle_merge_conflicts(argus);
    if (result != -1) 
        return result;
    
    return perform_branch_switch(argus, branch);
}