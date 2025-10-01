#include <argus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "commands/git.h"
#include "colors.h"
#include "git_types.h"
#include "mock_data.h"

ARGUS_OPTIONS(
    switch_options,
    HELP_OPTION(),
    
    GROUP_START("Branch creation"),
        OPTION_FLAG('c', "create", HELP("Create a new branch and switch to it")),
        OPTION_FLAG('C', "force-create", HELP("Similar to --create except if <new-branch> already exists, reset it")),
        OPTION_STRING('\0', "orphan", HELP("Create a new orphan branch")),
    GROUP_END(),
    
    GROUP_START("Branch tracking"),
        OPTION_STRING('t', "track",
            HELP("Set upstream info for new branch"),
            VALIDATOR(V_CHOICE_STR("direct", "inherit")),
            FLAGS(FLAG_OPTIONAL)),
        OPTION_FLAG('\0', "guess", HELP("If <branch> is not found but there does exist a tracking branch")),
        OPTION_FLAG('\0', "no-guess", HELP("Do not try to match remote branches")),
    GROUP_END(),
    
    GROUP_START("Worktree options"),
        OPTION_FLAG('f', "force", HELP("Throw away local modifications")),
        OPTION_FLAG('m', "merge", HELP("Perform a 3-way merge between current branch, working tree and new branch")),
        OPTION_STRING('\0', "conflict",
            HELP("Same as --merge, but change conflicted hunks display"),
            VALIDATOR(V_CHOICE_STR("merge", "diff3", "zdiff3")),
            FLAGS(FLAG_OPTIONAL)),
    GROUP_END(),
    
    GROUP_START("Display options"),
        OPTION_FLAG('q', "quiet", HELP("Suppress feedback messages")),
    GROUP_END(),
    
    POSITIONAL_STRING("branch", HELP("Branch to switch to"), FLAGS(FLAG_OPTIONAL)),
    POSITIONAL_STRING("start-point", HELP("Start point for new branch"), FLAGS(FLAG_OPTIONAL)),
)

static int validate_branch_argument(argus_t *argus, const char *branch)
{
    bool create = argus_get(argus, "create").as_bool;
    bool force_create = argus_get(argus, "force-create").as_bool;
    const char *orphan = argus_get(argus, "orphan").as_string;
    
    if (!branch && !orphan && !(create || force_create)) {
        printf(COLOR_RED("error: ") "missing branch or commit argument\n");
        printf("\nusage: git switch [<options>] [<branch>]\n");
        printf("   or: git switch [<options>] --create <branch> [<start-point>]\n");
        printf("   or: git switch [<options>] --orphan <new-branch>\n");
        return 1;
    }
    return -1;
}

static int handle_orphan_branch(argus_t *argus)
{
    const char *orphan = argus_get(argus, "orphan").as_string;
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

static int handle_branch_creation(argus_t *argus, const char *branch)
{
    bool create = argus_get(argus, "create").as_bool;
    bool force_create = argus_get(argus, "force-create").as_bool;
    bool quiet = argus_get(argus, "quiet").as_bool;
    const char *track_mode = argus_get(argus, "track").as_string;
    
    if (create || force_create) {
        if (!branch) {
            printf(COLOR_RED("error: ") "option '--create' requires a value\n");
            return 1;
        }
        
        int commit_count, remote_count;
        const git_commit_t *commits = get_mock_commits(&commit_count);
        const git_remote_t *remotes = get_mock_remotes(&remote_count);
        
        if (force_create && !quiet) {
            printf("Reset branch '" COLOR_GREEN("%s") "' (was at " COLOR_YELLOW("%s") ")\n", 
                   branch, commit_count > 0 ? commits[0].hash : "abc1234");
        }
        
        if (!quiet)
            printf("Switched to a new branch '" COLOR_GREEN("%s") "'\n", branch);
        
        if (track_mode && !quiet) {
            const char *remote = remote_count > 0 ? remotes[0].name : "origin";
            printf("Branch '" COLOR_GREEN("%s") "' set up to track remote branch 'main' from '%s' by %s.\n", 
                   branch, remote, track_mode);
        }
        
        return 0;
    }
    return -1;
}

static int handle_merge_conflicts(argus_t *argus)
{
    bool merge = argus_get(argus, "merge").as_bool;
    const char *conflict_style = argus_get(argus, "conflict").as_string;
    bool quiet = argus_get(argus, "quiet").as_bool;
    
    if (merge) {
        if (conflict_style && !quiet)
            printf("Using conflict style: %s\n", conflict_style);
        
        int file_count;
        const git_file_status_t *files = get_mock_file_status(&file_count);
        
        printf("Auto-merging conflicted files...\n");
        if (file_count > 0)
            printf("CONFLICT (content): Merge conflict in %s\n", files[0].filename);
        printf("Automatic merge failed; fix conflicts and then commit the result.\n");
        return 1;
    }
    return -1;
}

static bool check_branch_exists(const char *branch)
{
    int branch_count;
    const git_branch_t *branches = get_mock_branches(&branch_count);
    
    for (int i = 0; i < branch_count; i++) {
        if (strcmp(branches[i].name, branch) == 0)
            return true;
    }
    return false;
}

static int handle_branch_guessing(argus_t *argus, const char *branch)
{
    bool no_guess = argus_get(argus, "no-guess").as_bool;
    bool quiet = argus_get(argus, "quiet").as_bool;
    
    if (!check_branch_exists(branch) && !no_guess) {
        int remote_branch_count;
        const git_branch_t *remote_branches = get_mock_remote_branches(&remote_branch_count);
        
        for (int i = 0; i < remote_branch_count; i++) {
            const char *remote_name = strchr(remote_branches[i].name, '/');
            if (remote_name && strcmp(remote_name + 1, branch) == 0) {
                if (!quiet) {
                    printf("Branch '" COLOR_GREEN("%s") "' set up to track remote branch '" COLOR_CYAN("%s") "'.\n", 
                           branch, remote_branches[i].name);
                    printf("Switched to a new branch '" COLOR_GREEN("%s") "'\n", branch);
                }
                return 0;
            }
        }
    }
    return -1;
}

static int perform_branch_switch(argus_t *argus, const char *branch)
{
    bool quiet = argus_get(argus, "quiet").as_bool;
    bool force = argus_get(argus, "force").as_bool;
    
    if (!check_branch_exists(branch)) {
        printf(COLOR_RED("error: ") "pathspec '%s' did not match any file(s) known to git\n", branch);
        return 1;
    }
    
    int file_count;
    const git_file_status_t *files = get_mock_file_status(&file_count);
    bool has_modifications = false;
    
    for (int i = 0; i < file_count; i++) {
        if (files[i].modified) {
            has_modifications = true;
            break;
        }
    }
    
    if (has_modifications && force && !quiet)
        printf(COLOR_YELLOW("warning: ") "local modifications were discarded\n");
    
    if (!quiet) {
        printf("Switched to branch '" COLOR_GREEN("%s") "'\n", branch);
        printf("Your branch is up to date with 'origin/%s'.\n", branch);
    }
    
    return 0;
}

int switch_handler(argus_t *argus, void *data)
{
    (void)data;
    
    const char *branch = argus_get(argus, "branch").as_string;
    int result;
    
    if ((result = validate_branch_argument(argus, branch)) != -1)
        return result;
    
    if ((result = handle_orphan_branch(argus)) != -1)
        return result;
    
    if ((result = handle_branch_creation(argus, branch)) != -1)
        return result;
    
    if ((result = handle_merge_conflicts(argus)) != -1)
        return result;
    
    if ((result = handle_branch_guessing(argus, branch)) != -1)
        return result;
    
    return perform_branch_switch(argus, branch);
}