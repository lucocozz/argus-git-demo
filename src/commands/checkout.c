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
    checkout_options,
    HELP_OPTION(),
    
    GROUP_START("Branch options"),
        OPTION_FLAG('b', NULL, 
            HELP("Create and checkout a new branch")),
        OPTION_FLAG('B', NULL, 
            HELP("Create/reset and checkout a branch")),
        OPTION_FLAG('\0', "detach", 
            HELP("Detach HEAD at named commit")),
        OPTION_STRING('t', "track",
            HELP("Set upstream tracking for new branch"),
            VALIDATOR(V_CHOICE_STR("direct", "inherit")),
            FLAGS(FLAG_OPTIONAL)),
        OPTION_FLAG('\0', "no-track", 
            HELP("Do not set upstream tracking")),
    GROUP_END(),
    
    GROUP_START("Action options"),
        OPTION_FLAG('f', "force", 
            HELP("Force checkout (discard local modifications)")),
        OPTION_FLAG('m', "merge", 
            HELP("Perform 3-way merge with new branch")),
        OPTION_STRING('\0', "conflict",
            HELP("Set conflict resolution style"),
            VALIDATOR(V_CHOICE_STR("merge", "diff3", "zdiff3")),
            FLAGS(FLAG_OPTIONAL)),
        OPTION_FLAG('p', "patch", 
            HELP("Interactively select hunks")),
    GROUP_END(),
    
    GROUP_START("Output options"),
        OPTION_FLAG('q', "quiet", 
            HELP("Suppress feedback messages")),
    GROUP_END(),

    POSITIONAL_STRING("tree-ish", 
        HELP("Branch, commit, or tree-ish to checkout"), 
        FLAGS(FLAG_OPTIONAL)),
    POSITIONAL_MANY_STRING("pathspec", 
        HELP("Limit checkout to specified paths"), 
        FLAGS(FLAG_OPTIONAL)),
)

static int handle_patch_mode(argus_t *argus)
{
    bool patch = argus_get(argus, "patch").as_bool;
    
    if (patch) {
        int file_count;
        const git_file_status_t *files = get_mock_file_status(&file_count);
        
        printf("Applying patch to working tree...\n");
        for (int i = 0; i < file_count && i < 2; i++) {
            if (files[i].modified) {
                printf("diff --git a/%s b/%s\n", files[i].filename, files[i].filename);
                printf("Apply this hunk to working tree [y,n,q,a,d,e,?]? y\n");
                printf("Hunk applied to '%s'\n", files[i].filename);
            }
        }
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
        
        int commit_count;
        const git_commit_t *commits = get_mock_commits(&commit_count);
        const git_commit_t *latest = &commits[0];
        
        const char *action = force_create ? "Reset" : "Switched to a new";
        
        if (!quiet) {
            printf("%s branch '" COLOR_GREEN("%s") "'\n", action, tree_ish);
            if (force_create)
                printf("Your branch is now at " COLOR_YELLOW("%s") " " COLOR_BLUE("%s") "\n", 
                       latest->hash, latest->message);
        }
        
        if (track_mode && !quiet) {
            int remote_count;
            const git_remote_t *remotes = get_mock_remotes(&remote_count);
            const char *remote_name = remote_count > 0 ? remotes[0].name : "origin";
            printf("Branch '" COLOR_GREEN("%s") "' set up to track remote branch 'main' from '%s' by %s.\n", 
                   tree_ish, remote_name, track_mode);
        }
        
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
            int commit_count;
            const git_commit_t *commits = get_mock_commits(&commit_count);
            const git_commit_t *target_commit = &commits[0];
            
            printf("Note: switching to '" COLOR_YELLOW("%s") "'.\n\n", tree_ish);
            printf("You are in 'detached HEAD' state. You can look around, make experimental\n");
            printf("changes and commit them, and you can discard any commits you make in this\n");
            printf("state without impacting any branches by switching back to a branch.\n\n");
            printf("HEAD is now at " COLOR_YELLOW("%s") " " COLOR_BLUE("%s") "\n", 
                   target_commit->hash, target_commit->message);
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
    const char *conflict_style = argus_get(argus, "conflict").as_string;
    
    if (!argus_is_set(argus, "pathspec"))
        return -1;

    if (merge && !quiet)
        printf("Merging changes to files...\n");

    if (conflict_style && !quiet)
        printf("Using conflict style: %s\n", conflict_style);
    
    int file_status_count;
    const git_file_status_t *file_statuses = get_mock_file_status(&file_status_count);
        
    argus_array_it_t it = argus_array_it(argus, "pathspec");
    int file_count = 0;

    while (argus_array_next(&it)) {
        file_count++;
        
        if (!quiet) {
            const git_file_status_t *file_status = file_count <= file_status_count ? 
                &file_statuses[file_count - 1] : &file_statuses[0];
            
            const char *status_msg = force ? 
                "Checked out '%s' (" COLOR_YELLOW("local modifications overwritten") ")\n" :
                "Updated '%s'\n";
            printf(status_msg, file_status->filename);
        }
    }

    if (!quiet && file_count > 0)
        printf("Updated %d file(s)\n", file_count);
    
    return 0;
}

static int switch_branch(argus_t *argus, const char *tree_ish)
{
    bool quiet = argus_get(argus, "quiet").as_bool;
    
    if (tree_ish) {
        if (!quiet) {
            int branch_count, remote_count;
            const git_branch_t *branches = get_mock_branches(&branch_count);
            const git_remote_t *remotes = get_mock_remotes(&remote_count);
            
            bool branch_exists = false;
            for (int i = 0; i < branch_count; i++) {
                if (strcmp(branches[i].name, tree_ish) == 0) {
                    branch_exists = true;
                    break;
                }
            }
            
            const char *remote_name = remote_count > 0 ? remotes[0].name : "origin";
            
            printf("Switched to branch '" COLOR_GREEN("%s") "'\n", tree_ish);
            if (branch_exists)
                printf("Your branch is up to date with '%s/%s'.\n", remote_name, tree_ish);
        }
        return 0;
    }
    return -1;
}

static void show_current_status(void)
{
    int branch_count, remote_count;
    const git_branch_t *branches = get_mock_branches(&branch_count);
    const git_remote_t *remotes = get_mock_remotes(&remote_count);
    
    const char *current_branch = "main";
    for (int i = 0; i < branch_count; i++) {
        if (strcmp(branches[i].type, "current") == 0) {
            current_branch = branches[i].name;
            break;
        }
    }
    
    const char *remote_name = remote_count > 0 ? remotes[0].name : "origin";
    
    printf("On branch " COLOR_GREEN("%s") "\n", current_branch);
    printf("Your branch is up to date with '" COLOR_CYAN("%s/%s") "'.\n\n", remote_name, current_branch);
    printf("nothing to commit, working tree clean\n");
}

int checkout_handler(argus_t *argus, void *data)
{
    (void)data;
    
    const char *tree_ish = argus_get(argus, "tree-ish").as_string;
    int result;
    
    if ((result = handle_patch_mode(argus)) != -1) 
        return result;
    
    if ((result = handle_branch_creation(argus, tree_ish)) != -1) 
        return result;
    
    if ((result = handle_detached_head(argus, tree_ish)) != -1) 
        return result;
    
    if ((result = handle_file_checkout(argus)) != -1) 
        return result;
    
    if ((result = switch_branch(argus, tree_ish)) != -1) 
        return result;
    
    show_current_status();
    return 0;
}
