#include <argus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "commands/git.h"
#include "colors.h"
#include "git_types.h"

// ============================================================================
// GIT BRANCH COMMAND OPTIONS
// ============================================================================

ARGUS_OPTIONS(
    branch_options,
    HELP_OPTION(),
    
    GROUP_START("Generic options"),
        OPTION_FLAG(
            'v', "verbose", 
            HELP("Show hash and subject, give twice for upstream branch")
        ),
        OPTION_FLAG('q', "quiet", HELP("Suppress informational messages")),
        OPTION_STRING(
            't', "track",
            HELP("Set branch tracking configuration"),
            HINT("direct|inherit"),
            VALIDATOR(V_CHOICE_STR("direct", "inherit")),
            FLAGS(FLAG_OPTIONAL)
        ),
        OPTION_STRING(
            'u', "set-upstream-to",
            HELP("Change the upstream info"),
            HINT("upstream")
        ),
        OPTION_FLAG('\0', "unset-upstream", HELP("Unset the upstream info")),
        OPTION_FLAG('\0', "color", HELP("Use colored output")),
        OPTION_FLAG('r', "remotes", HELP("Act on remote-tracking branches")),
        OPTION_STRING(
            '\0', "contains", 
            HELP("Print only branches that contain the commit"),
            HINT("commit")
        ),
        OPTION_STRING(
            '\0', "no-contains",
            HELP("Print only branches that don't contain the commit"),
            HINT("commit")
        ),
        OPTION_FLAG('\0', "abbrev", HELP("Use <n> digits to display object names")),
    GROUP_END(),
    
    GROUP_START("Specific git-branch actions"),
        OPTION_FLAG('a', "all", HELP("List both remote-tracking and local branches")),
        OPTION_FLAG('d', "delete", HELP("Delete fully merged branch")),
        OPTION_FLAG('D', NULL, HELP("Delete branch (even if not merged)")),
        OPTION_FLAG('m', "move", HELP("Move/rename a branch and its reflog")),
        OPTION_FLAG('M', NULL, HELP("Move/rename a branch, even if target exists")),
        OPTION_FLAG('c', "copy", HELP("Copy a branch and its reflog")),
        OPTION_FLAG('C', NULL, HELP("Copy a branch, even if target exists")),
        OPTION_FLAG('l', "list", HELP("List branch names")),
        OPTION_FLAG('\0', "show-current", HELP("Show current branch name")),
        OPTION_FLAG('\0', "create-reflog", HELP("Create the branch's reflog")),
        OPTION_FLAG('\0', "edit-description", HELP("Edit the description for the branch")),
        OPTION_FLAG('f', "force", HELP("Force creation, move/rename, deletion")),
        OPTION_STRING(
            '\0', "merged", 
            HELP("Print only branches that are merged"),
            HINT("commit")
        ),
        OPTION_STRING(
            '\0', "no-merged", 
            HELP("Print only branches that are not merged"),
            HINT("commit")
        ),
        OPTION_FLAG('\0', "column", HELP("List branches in columns")),
        OPTION_FLAG('\0', "sort", HELP("Field name to sort on")),
        OPTION_STRING(
            '\0', "points-at",
            HELP("Print only branches of the object"),
            HINT("object")
        ),
        OPTION_FLAG('i', "ignore-case", HELP("Sorting and filtering are case insensitive")),
        OPTION_FLAG('\0', "recurse-submodules", HELP("Recurse through submodules")),
        OPTION_STRING(
            '\0', "format",
            HELP("Format to use for the output"),
            HINT("format")
        ),
    GROUP_END(),
    
    POSITIONAL_STRING("branchname", HELP("Branch name"), FLAGS(FLAG_OPTIONAL)),
    POSITIONAL_STRING("start-point", HELP("Start point for new branch"), FLAGS(FLAG_OPTIONAL)),
)

// ============================================================================
// GIT BRANCH HANDLER
// ============================================================================

int branch_handler(argus_t *argus, void *data)
{
    (void)data;
    
    // ========================================================================
    bool        all             = argus_get(argus, "all").as_bool;
    bool        remotes         = argus_get(argus, "remotes").as_bool;
    bool        verbose         = argus_get(argus, "verbose").as_bool;
    bool        delete_branch   = argus_get(argus, "delete").as_bool;
    bool        force_delete    = argus_get(argus, "D").as_bool;
    bool        move_branch     = argus_get(argus, "move").as_bool;
    bool        force_move      = argus_get(argus, "M").as_bool;
    bool        copy_branch     = argus_get(argus, "copy").as_bool;
    bool        force_copy      = argus_get(argus, "C").as_bool;
    bool        merged          = argus_get(argus, "merged").as_bool;
    bool        no_merged       = argus_get(argus, "no-merged").as_bool;
    bool        unset_upstream  = argus_get(argus, "unset-upstream").as_bool;
    const char *set_upstream    = argus_is_set(argus, "set-upstream-to") 
                                  ? argus_get(argus, "set-upstream-to").as_string : NULL;
    const char *track_mode      = argus_is_set(argus, "track") 
                                  ? argus_get(argus, "track").as_string : NULL;
    const char *contains        = argus_is_set(argus, "contains") 
                                  ? argus_get(argus, "contains").as_string : NULL;
    const char *format          = argus_is_set(argus, "format") 
                                  ? argus_get(argus, "format").as_string : NULL;
    const char *branchname      = argus_is_set(argus, "branchname") 
                                  ? argus_get(argus, "branchname").as_string : NULL;
    const char *start_point     = argus_is_set(argus, "start-point") 
                                  ? argus_get(argus, "start-point").as_string : NULL;
    
    // ========================================================================
    // Handle delete operations
    if ((delete_branch || force_delete) && branchname) {
        if (force_delete)
            printf("Deleted branch " COLOR_GREEN("%s") " (was " COLOR_YELLOW("abc1234") ").\n", branchname);
        else
            printf("Deleted branch " COLOR_GREEN("%s") " (was " COLOR_YELLOW("abc1234") ").\n", branchname);
        return 0;
    }
    
    // Handle move/rename operations
    if ((move_branch || force_move) && branchname) {
        const char *old_name = start_point ? start_point : "current-branch";
        printf("Branch " COLOR_GREEN("%s") " renamed to " COLOR_GREEN("%s") ".\n", old_name, branchname);
        return 0;
    }
    
    // Handle copy operations
    if ((copy_branch || force_copy) && branchname) {
        const char *source = start_point ? start_point : "current-branch";
        printf("Branch " COLOR_GREEN("%s") " copied to " COLOR_GREEN("%s") ".\n", source, branchname);
        return 0;
    }
    
    // Handle upstream configuration
    if (set_upstream && branchname) {
        printf("Branch '" COLOR_GREEN("%s") "' set up to track remote branch '" COLOR_CYAN("%s") "'.\n", 
               branchname, set_upstream);
        return 0;
    }
    
    if (unset_upstream && branchname) {
        printf("Branch '" COLOR_GREEN("%s") "' upstream tracking removed.\n", branchname);
        return 0;
    }
    
    // Handle branch creation
    if (branchname && !delete_branch && !move_branch && !copy_branch) {
        const char *start = start_point ? start_point : "HEAD";
        printf("Created branch '" COLOR_GREEN("%s") "' from '" COLOR_BLUE("%s") "'\n", 
               branchname, start);
        
        if (track_mode)
            printf("Branch '" COLOR_GREEN("%s") "' set up to track upstream (mode: %s).\n", 
                   branchname, track_mode);
        return 0;
    }
    
    // ========================================================================
    // Display simulated Git branch list
    const git_branch_t branches[] = {
        {"main", "abc1234", "Add new feature for user authentication", "current"},
        {"develop", "def5678", "Fix bug in payment processing", "local"},
        {"feature", "ghi9012", "Update documentation for API endpoints", "local"},
        {"hotfix", "jkl3456", "Emergency security patch", "local"}
    };
    
    const git_branch_t remote_branches[] = {
        {"origin/main", "abc1234", "Add new feature for user authentication", "remote"},
        {"origin/develop", "jkl3456", "Refactor database connection logic", "remote"},
        {"upstream/main", "abc1234", "Add new feature for user authentication", "remote"}
    };
    
    // Custom format output
    if (format) {
        printf("Using format: %s\n", format);
        printf("main\t\tabc1234\tAdd new feature\n");
        printf("develop\t\tdef5678\tFix bug\n");
        return 0;
    }
    
    // Filtering by contains
    if (contains) {
        printf("Branches containing commit '%s':\n", contains);
    }
    
    // Standard branch listing
    for (int i = 0; i < 4; i++) {
        const git_branch_t *branch = &branches[i];
        
        // Apply merge filtering
        if (merged && i > 1) {
            continue;
        }
        if (no_merged && i <= 1) {
            continue;
        }
        
        if (!remotes) {
            const char *prefix = strcmp(branch->type, "current") == 0 ? "* " : "  ";
            
            if (verbose)
                printf("%s" COLOR_GREEN("%-10s") " " COLOR_YELLOW("%s") " %s\n", 
                       prefix, branch->name, branch->hash, branch->message);
            else
                printf("%s" COLOR_GREEN("%s") "\n", prefix, branch->name);
        }
    }
    
    // Show remote branches if requested
    if (all || remotes) {
        for (int i = 0; i < 3; i++) {
            const git_branch_t *branch = &remote_branches[i];
            
            if (verbose)
                printf("  " COLOR_CYAN("%-20s") " " COLOR_YELLOW("%s") " %s\n", 
                       branch->name, branch->hash, branch->message);
            else
                printf("  " COLOR_CYAN("%s") "\n", branch->name);
        }
    }
    
    return 0;
}