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
    fetch_options,
    HELP_OPTION(),
    
    GROUP_START("Basic options"),
        OPTION_FLAG('q', "quiet", 
            HELP("Suppress output")),
        OPTION_FLAG('v', "verbose", 
            HELP("Show detailed fetch information")),
        OPTION_FLAG('\0', "dry-run", 
            HELP("Show what would be fetched without fetching")),
        OPTION_FLAG('\0', "all", 
            HELP("Fetch all remotes")),
    GROUP_END(),
    
    GROUP_START("Update options"),
        OPTION_FLAG('f', "force", 
            HELP("Force update of local refs")),
        OPTION_FLAG('\0', "prune", 
            HELP("Remove deleted remote-tracking branches")),
        OPTION_FLAG('t', "tags", 
            HELP("Fetch all tags from remote")),
        OPTION_FLAG('\0', "no-tags", 
            HELP("Do not fetch tags")),
    GROUP_END(),
    
    POSITIONAL_STRING("repository", 
        HELP("Remote repository to fetch from"), 
        DEFAULT("origin"), 
        FLAGS(FLAG_OPTIONAL)),
    POSITIONAL_MANY_STRING("refspec", 
        HELP("References to fetch"), 
        FLAGS(FLAG_OPTIONAL),
        VALIDATOR(V_COUNT(0, 10))),
)

static int handle_dry_run(argus_t *argus, const char *repository)
{
    bool dry_run = argus_get(argus, "dry-run").as_bool;
    bool tags = argus_get(argus, "tags").as_bool;
    
    if (dry_run) {
        int remote_count;
        const git_remote_t *remotes = get_mock_remotes(&remote_count);
        const git_remote_t *target_remote = NULL;
        
        for (int i = 0; i < remote_count; i++) {
            if (strcmp(remotes[i].name, repository) == 0) {
                target_remote = &remotes[i];
                break;
            }
        }
        
        if (!target_remote && remote_count > 0)
            target_remote = &remotes[0];
        
        if (target_remote) {
            printf("From %s\n", target_remote->url);
            
            int branch_count;
            const git_branch_t *branches = get_mock_remote_branches(&branch_count);
            
            for (int i = 0; i < branch_count && i < 3; i++)
                printf(" * [would fetch] branch %s -> %s\n", 
                       branches[i].name + strlen("origin/"), branches[i].name);
            
            if (tags)
                printf(" * [would fetch] tag v1.0.0 -> v1.0.0\n");
        }
        return 0;
    }
    return -1;
}

static void execute_fetch_operation(argus_t *argus, const char *repository)
{
    bool all = argus_get(argus, "all").as_bool;
    bool quiet = argus_get(argus, "quiet").as_bool;
    bool verbose = argus_get(argus, "verbose").as_bool;
    
    int remote_count;
    const git_remote_t *remotes = get_mock_remotes(&remote_count);
    
    if (all) {
        for (int i = 0; i < remote_count; i++) {
            printf("Fetching %s\n", remotes[i].name);
            if (!quiet) {
                printf("From %s\n", remotes[i].url);
                
                int branch_count;
                const git_branch_t *branches = get_mock_remote_branches(&branch_count);
                
                for (int j = 0; j < branch_count; j++) {
                    if (strstr(branches[j].name, remotes[i].name))
                        printf(" * branch            %s -> %s\n", 
                               strchr(branches[j].name, '/') + 1, branches[j].name);
                }
            }
        }
        return;
    }
    
    const git_remote_t *target_remote = remote_count > 0 ? &remotes[0] : NULL;
    for (int i = 0; i < remote_count; i++) {
        if (strcmp(remotes[i].name, repository) == 0) {
            target_remote = &remotes[i];
            break;
        }
    }
    
    if (!quiet && target_remote) {
        printf("From %s\n", target_remote->url);
        
        if (verbose) {
            printf("remote: Enumerating objects: 42, done.\n");
            printf("remote: Counting objects: 100%% (42/42), done.\n");
            printf("remote: Compressing objects: 100%% (25/25), done.\n");
            printf("remote: Total 42 (delta 15), reused 35 (delta 8), pack-reused 0\n");
            printf("Unpacking objects: 100%% (42/42), 8.54 KiB | 2.85 MiB/s, done.\n");
        }
    }
}

static void display_fetch_results(argus_t *argus)
{
    bool quiet = argus_get(argus, "quiet").as_bool;
    bool verbose = argus_get(argus, "verbose").as_bool;
    bool tags = argus_get(argus, "tags").as_bool;
    bool force = argus_get(argus, "force").as_bool;
    
    if (quiet) 
        return;
    
    argus_array_it_t it = argus_array_it(argus, "refspec");
    bool has_refspecs = false;
    
    while (argus_array_next(&it)) {
        const char *refspec = it.value.as_string;
        const char *force_mark = force ? " + " : "   ";
        printf("%s%s -> origin/%s\n", force_mark, refspec, refspec);
        has_refspecs = true;
    }
    
    if (!has_refspecs) {
        int branch_count;
        const git_branch_t *branches = get_mock_branches(&branch_count);
        
        for (int i = 0; i < branch_count && i < 3; i++) {
            const char *status = (i == 0) ? " * branch" : 
                                (i == 1) ? " * [new branch]" : " = [up to date]";
            const char *force_mark = force ? " (forced update)" : "";
            
            printf("%s %s -> origin/%s%s\n", 
                   status, branches[i].name, branches[i].name, force_mark);
        }
    }
    
    if (tags) {
        printf(" * [new tag]         v1.0.0     -> v1.0.0\n");
        printf(" = [up to date]      v0.9.0     -> v0.9.0\n");
    }
    
    if (verbose) {
        int commit_count;
        const git_commit_t *commits = get_mock_commits(&commit_count);
        if (commit_count >= 2)
            printf("   %s..%s  main       -> origin/main\n", 
                   commits[1].hash, commits[0].hash);
    }
}

static void handle_prune_operations(argus_t *argus)
{
    bool prune = argus_get(argus, "prune").as_bool;
    bool quiet = argus_get(argus, "quiet").as_bool;
    
    if (prune && !quiet)
        printf(" x [deleted]         (none)     -> origin/old-feature\n");
}

int fetch_handler(argus_t *argus, void *data)
{
    (void)data;
    
    const char *repository = argus_get(argus, "repository").as_string;
    int result;
    
    if ((result = handle_dry_run(argus, repository)) != -1)
        return result;
    
    execute_fetch_operation(argus, repository);
    display_fetch_results(argus);
    handle_prune_operations(argus);
    
    return 0;
}