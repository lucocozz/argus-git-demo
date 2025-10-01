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
    branch_options,
    HELP_OPTION(),
    
    GROUP_START("List options"),
        OPTION_FLAG('v', "verbose", 
            HELP("Show hash and commit subject line for each head")),
        OPTION_FLAG('a', "all", 
            HELP("List both remote-tracking and local branches")),
        OPTION_FLAG('r', "remotes", 
            HELP("List or delete remote-tracking branches")),
        OPTION_STRING('\0', "contains", 
            HELP("Print only branches that contain the commit"), 
            HINT("commit")),
        OPTION_STRING('\0', "merged", 
            HELP("Print only branches that are merged"), 
            HINT("commit"), 
            FLAGS(FLAG_OPTIONAL)),
        OPTION_STRING('\0', "no-merged", 
            HELP("Print only branches that are not merged"), 
            HINT("commit"), 
            FLAGS(FLAG_OPTIONAL)),
        OPTION_FLAG('\0', "show-current", 
            HELP("Print the name of the current branch")),
    GROUP_END(),
    
    GROUP_START("Action options"),
        OPTION_FLAG('d', "delete", 
            HELP("Delete fully merged branch")),
        OPTION_FLAG('D', NULL, 
            HELP("Delete branch (even if not merged)")),
        OPTION_FLAG('m', "move", 
            HELP("Move/rename a branch and its reflog")),
        OPTION_FLAG('M', NULL, 
            HELP("Move/rename a branch, even if target exists")),
        OPTION_FLAG('c', "copy", 
            HELP("Copy a branch and its reflog")),
        OPTION_FLAG('C', NULL, 
            HELP("Copy a branch, even if target exists")),
        OPTION_FLAG('f', "force", 
            HELP("Reset branchname to startpoint, even if branchname exists already")),
    GROUP_END(),

    GROUP_START("Tracking options"),
        OPTION_STRING('t', "track", 
            HELP("Set up tracking configuration"),
            VALIDATOR(V_CHOICE_STR("direct", "inherit")),
            FLAGS(FLAG_OPTIONAL)),
        OPTION_STRING('u', "set-upstream-to", 
            HELP("Set up upstream configuration"), 
            HINT("upstream")),
        OPTION_FLAG('\0', "unset-upstream", 
            HELP("Remove upstream configuration")),
    GROUP_END(),

    GROUP_START("Output options"),
        OPTION_FLAG('q', "quiet", 
            HELP("Suppress informational messages")),
        OPTION_STRING('\0', "format", 
            HELP("Pretty-print branches using custom format"),
            HINT("format")),
    GROUP_END(),

    POSITIONAL_STRING("branchname", 
        HELP("Name of the branch to operate on"), 
        FLAGS(FLAG_OPTIONAL),
        VALIDATOR(V_LENGTH(1, 100))),
    POSITIONAL_STRING("start-point", 
        HELP("Starting point for the new branch"), 
        FLAGS(FLAG_OPTIONAL)),
)

static int handle_branch_deletion(argus_t *argus, const char *branchname)
{
    bool force_delete = argus_get(argus, "D").as_bool;
    bool quiet = argus_get(argus, "quiet").as_bool;
    
    if (!quiet) {
        const char *forced_str = force_delete ? " [forced]" : "";
        printf("Deleted branch " COLOR_GREEN("%s") " (was " COLOR_YELLOW("abc1234") ")%s.\n", branchname, forced_str);
    }
    
    return 0;
}

static int handle_branch_move_copy(argus_t *argus, const char *branchname, const char *start_point)
{
    bool move_branch = argus_get(argus, "move").as_bool;
    bool force_move = argus_get(argus, "M").as_bool;
    bool copy_branch = argus_get(argus, "copy").as_bool;
    bool force_copy = argus_get(argus, "C").as_bool;
    bool force = argus_get(argus, "force").as_bool;
    bool quiet = argus_get(argus, "quiet").as_bool;
    
    if (move_branch || force_move) {
        const char *old_name = start_point ? start_point : "current-branch";
        const char *forced_str = (force_move || force) ? "forcibly " : "";
        if (!quiet)
            printf("Branch " COLOR_GREEN("%s") " %srenamed to " COLOR_GREEN("%s") ".\n", old_name, forced_str, branchname);
        return 0;
    }
    
    if (copy_branch || force_copy) {
        const char *source = start_point ? start_point : "current-branch";
        const char *forced_str = (force_copy || force) ? "forcibly " : "";
        if (!quiet)
            printf("Branch " COLOR_GREEN("%s") " %scopied to " COLOR_GREEN("%s") ".\n", source, forced_str, branchname);
        return 0;
    }
    
    return 1;
}

static int handle_branch_upstream(argus_t *argus, const char *branchname)
{
    const char *set_upstream = argus_get(argus, "set-upstream-to").as_string;
    bool unset_upstream = argus_get(argus, "unset-upstream").as_bool;
    bool quiet = argus_get(argus, "quiet").as_bool;
    
    if (set_upstream && !quiet) {
        printf("Branch '" COLOR_GREEN("%s") "' set up to track remote branch '" COLOR_CYAN("%s") "'.\n", 
               branchname, set_upstream);
        return 0;
    }
    
    if (unset_upstream && !quiet) {
        printf("Branch '" COLOR_GREEN("%s") "' upstream tracking removed.\n", branchname);
        return 0;
    }
    
    return set_upstream || unset_upstream ? 0 : 1;
}

static int handle_branch_creation(argus_t *argus, const char *branchname, const char *start_point)
{
    const char *track_mode = argus_get(argus, "track").as_string;
    bool force = argus_get(argus, "force").as_bool;
    bool quiet = argus_get(argus, "quiet").as_bool;
    const char *start = start_point ? start_point : "HEAD";
    
    if (!quiet) {
        const char *action = force ? "Reset" : "Created";
        printf("%s branch '" COLOR_GREEN("%s") "' %s '" COLOR_BLUE("%s") "'\n", 
               action, branchname, force ? "to" : "from", start);
    }
    
    if (track_mode && !quiet)
        printf("Branch '" COLOR_GREEN("%s") "' set up to track upstream (mode: %s).\n", branchname, track_mode);
    
    return 0;
}

static void display_local_branches(const git_branch_t *branches, int count, bool verbose, bool merged, bool no_merged)
{
    for (int i = 0; i < count; i++) {
        const git_branch_t *branch = &branches[i];
        
        if (merged && i > 1) continue;
        if (no_merged && i <= 1) continue;
        
        const char *prefix = strcmp(branch->type, "current") == 0 ? "* " : "  ";
        
        if (verbose) 
            printf("%s" COLOR_GREEN("%-10s") " " COLOR_YELLOW("%s") " %s\n", prefix, branch->name, branch->hash, branch->message);
        else 
            printf("%s" COLOR_GREEN("%s") "\n", prefix, branch->name);
    }
}

static void display_remote_branches(const git_branch_t *branches, int count, bool verbose)
{
    for (int i = 0; i < count; i++) {
        const git_branch_t *branch = &branches[i];
        
        if (verbose) 
            printf("  " COLOR_CYAN("%-20s") " " COLOR_YELLOW("%s") " %s\n", branch->name, branch->hash, branch->message);
        else 
            printf("  " COLOR_CYAN("%s") "\n", branch->name);
    }
}

static void display_branch_list(argus_t *argus, bool verbose, bool remotes, bool all)
{
    const char *contains = argus_get(argus, "contains").as_string;
    const char *format = argus_get(argus, "format").as_string;
    const char *merged = argus_get(argus, "merged").as_string;
    const char *no_merged = argus_get(argus, "no-merged").as_string;
    bool quiet = argus_get(argus, "quiet").as_bool;
    
    if (format) {
        if (!quiet) printf("Using custom format: %s\n", format);
        printf("main\t\tabc1234\tAdd new feature\n");
        printf("develop\t\tdef5678\tFix bug\n");
        return;
    }
    
    if (contains && !quiet)
        printf("Branches containing commit '%s':\n", contains);
    
    int branch_count, remote_branch_count;
    const git_branch_t *branches = get_mock_branches(&branch_count);
    const git_branch_t *remote_branches = get_mock_remote_branches(&remote_branch_count);
    
    bool filter_merged = (merged != NULL);
    bool filter_no_merged = (no_merged != NULL);
    
    if (!remotes)
        display_local_branches(branches, branch_count, verbose, filter_merged, filter_no_merged);
    if (all || remotes)
        display_remote_branches(remote_branches, remote_branch_count, verbose);
}

static bool has_deletion_flags(argus_t *argus)
{
    return argus_get(argus, "delete").as_bool || argus_get(argus, "D").as_bool;
}

static bool has_move_copy_flags(argus_t *argus)
{
    return argus_get(argus, "move").as_bool || argus_get(argus, "M").as_bool ||
           argus_get(argus, "copy").as_bool || argus_get(argus, "C").as_bool;
}

static bool has_upstream_flags(argus_t *argus)
{
    return argus_get(argus, "set-upstream-to").as_string || argus_get(argus, "unset-upstream").as_bool;
}

static bool has_action_flags(argus_t *argus)
{
    return has_deletion_flags(argus) || has_move_copy_flags(argus) || has_upstream_flags(argus);
}

int branch_handler(argus_t *argus, void *data)
{
    (void)data;
    
    const char *branchname = argus_get(argus, "branchname").as_string;
    const char *start_point = argus_get(argus, "start-point").as_string;
    bool verbose = argus_get(argus, "verbose").as_bool;
    bool remotes = argus_get(argus, "remotes").as_bool;
    bool all = argus_get(argus, "all").as_bool;
    bool show_current = argus_get(argus, "show-current").as_bool;
    bool quiet = argus_get(argus, "quiet").as_bool;
    
    if (show_current) {
        printf("main\n");
        return 0;
    }
    
    if (has_deletion_flags(argus) && branchname) {
        if (!quiet) printf("Deleting branch '%s'...\n", branchname);
        return handle_branch_deletion(argus, branchname);
    }
    
    if (has_move_copy_flags(argus) && branchname)
        return handle_branch_move_copy(argus, branchname, start_point);
    
    if (has_upstream_flags(argus) && branchname)
        return handle_branch_upstream(argus, branchname);
    
    if (branchname && !has_action_flags(argus))
        return handle_branch_creation(argus, branchname, start_point);
    
    display_branch_list(argus, verbose, remotes, all);
    return 0;
}