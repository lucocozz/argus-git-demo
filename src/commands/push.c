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
    push_options,
    HELP_OPTION(),
    
    GROUP_START("General options"),
        OPTION_FLAG('q', "quiet", HELP("Operate quietly")),
        OPTION_FLAG('v', "verbose", HELP("Be more verbose")),
        OPTION_FLAG('n', "dry-run", HELP("Show what would be pushed")),
    GROUP_END(),
    
    GROUP_START("Push options"),
        OPTION_FLAG('\0', "all", HELP("Push all branches")),
        OPTION_FLAG('d', "delete", HELP("Delete remote refs")),
        OPTION_FLAG('\0', "tags", HELP("Push all tags")),
        OPTION_FLAG('\0', "follow-tags", HELP("Push missing tags")),
    GROUP_END(),
    
    GROUP_START("Force options"),
        OPTION_FLAG('f', "force", HELP("Force updates")),
        OPTION_FLAG('\0', "force-with-lease", HELP("Force with safety checks")),
    GROUP_END(),
    
    GROUP_START("Upstream options"),
        OPTION_FLAG('u', "set-upstream", HELP("Set upstream tracking")),
    GROUP_END(),
    
    POSITIONAL_STRING("repository", HELP("Repository to push to"), DEFAULT("origin"), FLAGS(FLAG_OPTIONAL)),
    POSITIONAL_MANY_STRING("refspec", HELP("Refspecs to push"), FLAGS(FLAG_OPTIONAL)),
)

static int handle_dry_run(argus_t *argus, const char *repository)
{
    bool dry_run = argus_get(argus, "dry-run").as_bool;
    bool delete_refs = argus_get(argus, "delete").as_bool;
    
    if (dry_run) {
        int remote_count;
        const git_remote_t *remotes = get_mock_remotes(&remote_count);
        const char *url = "https://github.com/user/repo.git";
        
        for (int i = 0; i < remote_count; i++) {
            if (strcmp(remotes[i].name, repository) == 0) {
                url = remotes[i].url;
                break;
            }
        }
        
        printf("To %s\n", url);
        printf(" * [would push] main -> main\n");
        
        argus_array_it_t it = argus_array_it(argus, "refspec");
        while (argus_array_next(&it)) {
            const char *refspec = it.value.as_string;
            printf(" * [would push] %s -> %s\n", refspec, refspec);
        }
        
        if (delete_refs)
            printf(" - [would delete] old-feature\n");
        return 0;
    }
    return -1;
}

static void execute_push_operation(argus_t *argus, const char *repository)
{
    bool quiet = argus_get(argus, "quiet").as_bool;
    bool verbose = argus_get(argus, "verbose").as_bool;
    
    int remote_count;
    const git_remote_t *remotes = get_mock_remotes(&remote_count);
    const char *url = "https://github.com/user/repo.git";
    
    for (int i = 0; i < remote_count; i++) {
        if (strcmp(remotes[i].name, repository) == 0) {
            url = remotes[i].url;
            break;
        }
    }
    
    if (!quiet && verbose) {
        printf("Enumerating objects: 15, done.\n");
        printf("Counting objects: 100%% (15/15), done.\n");
        printf("Delta compression using up to 8 threads\n");
        printf("Compressing objects: 100%% (8/8), done.\n");
        printf("Writing objects: 100%% (15/15), 2.45 KiB | 2.45 MiB/s, done.\n");
        printf("Total 15 (delta 3), reused 10 (delta 1), pack-reused 0\n");
    }
    
    if (!quiet)
        printf("To %s\n", url);
    
    if (verbose)
        printf("remote: Resolving deltas: 100%% (3/3), done.\n");
}

static int validate_force_options(argus_t *argus)
{
    bool force = argus_get(argus, "force").as_bool;
    bool force_with_lease = argus_get(argus, "force-with-lease").as_bool;
    
    if (force && force_with_lease) {
        printf(COLOR_RED("error: ") "cannot use both --force and --force-with-lease\n");
        return 1;
    }
    return -1;
}

static void display_push_results(argus_t *argus)
{
    bool quiet = argus_get(argus, "quiet").as_bool;
    bool all = argus_get(argus, "all").as_bool;
    bool delete_refs = argus_get(argus, "delete").as_bool;
    bool force = argus_get(argus, "force").as_bool;
    bool force_with_lease = argus_get(argus, "force-with-lease").as_bool;
    bool tags = argus_get(argus, "tags").as_bool;
    bool follow_tags = argus_get(argus, "follow-tags").as_bool;
    
    if (quiet) 
        return;
    
    int commit_count, branch_count;
    const git_commit_t *commits = get_mock_commits(&commit_count);
    const git_branch_t *branches = get_mock_branches(&branch_count);
    
    const char *old_hash = commit_count > 1 ? commits[1].hash : "abc1234";
    const char *new_hash = commit_count > 0 ? commits[0].hash : "def5678";
    
    if (force || force_with_lease) {
        const char *safety = force_with_lease ? "with lease" : "forced";
        printf(" + %s...%s main -> main (" COLOR_YELLOW("%s update") ")\n", old_hash, new_hash, safety);
    } else if (all) {
        for (int i = 0; i < branch_count && i < 3; i++) {
            printf("   %s..%s  %s -> %s\n", old_hash, new_hash, branches[i].name, branches[i].name);
        }
    } else if (delete_refs) {
        argus_array_it_t it = argus_array_it(argus, "refspec");
        while (argus_array_next(&it)) {
            printf(" - [deleted]         %s\n", it.value.as_string);
        }
    } else {
        bool has_refspecs = argus_is_set(argus, "refspec");
        if (has_refspecs) {
            argus_array_it_t it = argus_array_it(argus, "refspec");
            while (argus_array_next(&it)) {
                printf("   %s..%s  %s -> %s\n", old_hash, new_hash, it.value.as_string, it.value.as_string);
            }
        } else {
            printf("   %s..%s  main -> main\n", old_hash, new_hash);
        }
    }
    
    if (tags || follow_tags) {
        printf(" * [new tag]         v1.0.0 -> v1.0.0\n");
        printf(" * [new tag]         v1.1.0 -> v1.1.0\n");
    }
}

static void handle_upstream_tracking(argus_t *argus)
{
    bool set_upstream = argus_get(argus, "set-upstream").as_bool;
    bool quiet = argus_get(argus, "quiet").as_bool;
    
    if (set_upstream && !quiet) {
        int branch_count;
        const git_branch_t *branches = get_mock_branches(&branch_count);
        const char *current = branch_count > 0 ? branches[0].name : "main";
        printf("Branch '%s' set up to track remote branch '%s' from 'origin'.\n", current, current);
    }
}

int push_handler(argus_t *argus, void *data)
{
    (void)data;
    
    const char *repository = argus_get(argus, "repository").as_string;
    int result;
    
    if ((result = validate_force_options(argus)) != -1)
        return result;
    
    if ((result = handle_dry_run(argus, repository)) != -1)
        return result;
    
    execute_push_operation(argus, repository);
    display_push_results(argus);
    handle_upstream_tracking(argus);
    
    return 0;
}