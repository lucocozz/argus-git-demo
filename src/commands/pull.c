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
    pull_options,
    HELP_OPTION(),
    
    GROUP_START("General options"),
        OPTION_FLAG('q', "quiet", HELP("Operate quietly")),
        OPTION_FLAG('v', "verbose", HELP("Be more verbose")),
    GROUP_END(),
    
    GROUP_START("Fetch options"),
        OPTION_FLAG('\0', "all", HELP("Fetch all remotes")),
        OPTION_FLAG('t', "tags", HELP("Fetch all tags")),
    GROUP_END(),
    
    GROUP_START("Merge options"),  
        OPTION_FLAG('\0', "no-commit", HELP("Do not commit the merge")),
        OPTION_FLAG('\0', "ff-only", HELP("Only allow fast-forward")),
        OPTION_FLAG('\0', "no-ff", HELP("Create merge commit")),
        OPTION_FLAG('\0', "squash", HELP("Squash commits")),
        OPTION_FLAG('\0', "autostash", HELP("Automatically stash/unstash")),
    GROUP_END(),
    
    GROUP_START("Rebase options"),
        OPTION_FLAG('r', "rebase", HELP("Rebase instead of merge")),
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
    bool tags = argus_get(argus, "tags").as_bool;
    
    int remote_count;
    const git_remote_t *remotes = get_mock_remotes(&remote_count);
    const char *url = remote_count > 0 ? remotes[0].url : "https://github.com/user/repo";
    
    if (verbose) {
        printf("Fetching from %s...\n", repository);
    }
    
    if (!quiet) {
        printf("From %s\n", url);
        printf(" * branch            main       -> FETCH_HEAD\n");
        if (refspec)
            printf(" * branch            %s       -> FETCH_HEAD\n", refspec);
        
        if (tags) {
            printf(" * [new tag]         v1.0.0     -> v1.0.0\n");
            printf(" * [new tag]         v1.1.0     -> v1.1.0\n");
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