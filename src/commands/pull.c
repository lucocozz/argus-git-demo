#include <argus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "commands/git.h"
#include "colors.h"
#include "git_types.h"

ARGUS_OPTIONS(
    pull_options,
    HELP_OPTION(),
    
    GROUP_START("General options"),
        OPTION_FLAG('q', "quiet", HELP("Operate quietly")),
        OPTION_FLAG('v', "verbose", HELP("Be more verbose")),
        OPTION_FLAG('\0', "progress", HELP("Force progress reporting")),
        OPTION_FLAG('\0', "no-progress", HELP("Do not show progress")),
    GROUP_END(),
    
    GROUP_START("Fetch options"),
        OPTION_FLAG('\0', "all", HELP("Fetch all remotes")),
        OPTION_FLAG('a', "append", HELP("Append ref names and object names to .git/FETCH_HEAD")),
        OPTION_STRING('\0', "depth", HELP("Limit fetching to ancestor-chains not longer than n")),
        OPTION_STRING('\0', "deepen", HELP("Deepen history of shallow repository")),
        OPTION_STRING('\0', "shallow-since", HELP("Deepen history of shallow repository since date")),
        OPTION_STRING('\0', "shallow-exclude", HELP("Deepen history excluding ref")),
        OPTION_FLAG('\0', "unshallow", HELP("Convert shallow repository to complete one")),
        OPTION_FLAG('\0', "update-shallow", HELP("Accept refs that update .git/shallow")),
        OPTION_FLAG('\0', "force", HELP("Force update of local refs")),
        OPTION_FLAG('t', "tags", HELP("Fetch all tags from remote")),
        OPTION_FLAG('\0', "no-tags", HELP("Don't fetch tags from remote")),
    GROUP_END(),
    
    GROUP_START("Merge options"),  
        OPTION_FLAG('\0', "commit", HELP("Perform the merge and commit the result")),
        OPTION_FLAG('\0', "no-commit", HELP("Do not commit the merge")),
        OPTION_FLAG('\0', "edit", HELP("Edit the merge commit message")),
        OPTION_FLAG('\0', "no-edit", HELP("Accept auto-generated merge message")),
        OPTION_FLAG('\0', "ff", HELP("Fast-forward if possible")),
        OPTION_FLAG('\0', "no-ff", HELP("Create merge commit even if fast-forward is possible")),
        OPTION_FLAG('\0', "ff-only", HELP("Abort if fast-forward is not possible")),
        OPTION_FLAG('\0', "squash", HELP("Create single commit instead of merge")),
        OPTION_FLAG('\0', "no-squash", HELP("Do not squash commits")),
        OPTION_STRING('\0', "strategy", HELP("Use the given merge strategy")),
        OPTION_MAP_STRING('X', "strategy-option", HELP("Pass option to merge strategy"), HINT("option")),
        OPTION_FLAG('\0', "verify-signatures", HELP("Verify GPG signatures of commits")),
        OPTION_FLAG('\0', "no-verify-signatures", HELP("Do not verify GPG signatures")),
        OPTION_FLAG('S', "gpg-sign", HELP("GPG-sign the commit")),
        OPTION_FLAG('\0', "no-gpg-sign", HELP("Do not GPG-sign the commit")),
        OPTION_FLAG('\0', "autostash", HELP("Automatically stash/unstash local changes")),
        OPTION_FLAG('\0', "no-autostash", HELP("Do not automatically stash changes")),
    GROUP_END(),
    
    GROUP_START("Rebase options"),
        OPTION_FLAG('r', "rebase", HELP("Rebase current branch on top of upstream after fetching")),
        OPTION_FLAG('\0', "no-rebase", HELP("Do not rebase")),
    GROUP_END(),
    
    POSITIONAL_STRING("repository", HELP("Repository to pull from"), DEFAULT("origin"), FLAGS(FLAG_OPTIONAL)),
    POSITIONAL_STRING("refspec", HELP("Refspec to pull"), FLAGS(FLAG_OPTIONAL)),
)

static void handle_autostash(argus_t *argus, bool stash)
{
    bool autostash = argus_get(argus, "autostash").as_bool;
    bool quiet = argus_get(argus, "quiet").as_bool;
    
    if (!autostash) 
        return;
    
    if (stash) {
        if (!quiet)
            printf(COLOR_BLUE("Created autostash: abc1234") "\n");
    } else {
        if (!quiet)
            printf(COLOR_BLUE("Applied autostash.") "\n");
    }
}

static void execute_fetch(argus_t *argus, const char *repository, const char *refspec)
{
    bool quiet = argus_get(argus, "quiet").as_bool;
    bool verbose = argus_get(argus, "verbose").as_bool;
    bool progress = argus_get(argus, "progress").as_bool;
    bool tags = argus_get(argus, "tags").as_bool;
    
    if (!quiet) {
        printf("From https://github.com/user/repo\n");
        printf(" * branch            main       -> FETCH_HEAD\n");
        if (refspec)
            printf(" * branch            %s       -> FETCH_HEAD\n", refspec);
        else
            printf(" * [new branch]      feature    -> %s/feature\n", repository);
        
        if (tags) {
            printf(" * [new tag]         v1.0.0     -> v1.0.0\n");
            printf(" * [new tag]         v1.1.0     -> v1.1.0\n");
        }
        
        if (verbose || progress) {
            printf("remote: Enumerating objects: 15, done.\n");
            printf("remote: Counting objects: 100%% (15/15), done.\n");
            printf("remote: Compressing objects: 100%% (8/8), done.\n");
            printf("remote: Total 15 (delta 3), reused 10 (delta 1), pack-reused 0\n");
            printf("Unpacking objects: 100%% (15/15), 2.45 KiB | 1.23 MiB/s, done.\n");
        }
    }
}

static void execute_merge_or_rebase(argus_t *argus)
{
    bool rebase = argus_get(argus, "rebase").as_bool;
    bool ff_only = argus_get(argus, "ff-only").as_bool;
    bool no_ff = argus_get(argus, "no-ff").as_bool;
    bool quiet = argus_get(argus, "quiet").as_bool;
    bool verbose = argus_get(argus, "verbose").as_bool;
    const char *strategy = argus_get(argus, "strategy").as_string;
    
    if (rebase) {
        if (!quiet)
            printf(COLOR_BLUE("Rebasing...") "\n");
        printf("Successfully rebased and updated refs/heads/main.\n");
        
        if (verbose) {
            printf("First, rewinding head to replay your work on top of it...\n");
            printf("Applying: Add new feature implementation\n");
            printf("Applying: Fix validation bug in user input\n");
            printf("Applying: Update documentation for new API\n");
        }
    } else {
        if (ff_only) {
            printf(COLOR_GREEN("Updating abc1234..def5678") "\n");
            printf(COLOR_GREEN("Fast-forward") "\n");
        } else if (no_ff) {
            printf("Merge made by the '%s' strategy.\n", strategy ? strategy : "ort");
        } else {
            printf(COLOR_GREEN("Updating abc1234..def5678") "\n");
            printf(COLOR_GREEN("Fast-forward") "\n");
        }
        
        if (!quiet) {
            printf(" src/main.c           | 8 " COLOR_GREEN("+++++") COLOR_RED("---") "\n");
            printf(" tests/test_api.c     | 12 " COLOR_GREEN("+++++++++") COLOR_RED("---") "\n");
            printf(" docs/README.md       | 3 " COLOR_GREEN("+++") "\n");
            printf(" 3 files changed, 20 insertions(" COLOR_GREEN("+") "), 6 deletions(" COLOR_RED("-") ")\n");
        }
    }
}

int pull_handler(argus_t *argus, void *data)
{
    (void)data;
    
    const char *repository = argus_get(argus, "repository").as_string;
    const char *refspec = argus_get(argus, "refspec").as_string;
    
    handle_autostash(argus, true);
    execute_fetch(argus, repository, refspec);
    execute_merge_or_rebase(argus);
    handle_autostash(argus, false);
    
    return 0;
}