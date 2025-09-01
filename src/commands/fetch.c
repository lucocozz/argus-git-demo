#include <argus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "commands/git.h"
#include "colors.h"
#include "git_types.h"

ARGUS_OPTIONS(
    fetch_options,
    HELP_OPTION(),
    
    GROUP_START("General options"),
        OPTION_FLAG('q', "quiet", HELP("Operate quietly")),
        OPTION_FLAG('v', "verbose", HELP("Be more verbose")),
        OPTION_FLAG('\0', "progress", HELP("Force progress reporting")),
        OPTION_FLAG('\0', "no-progress", HELP("Do not show progress")),
        OPTION_FLAG('\0', "all", HELP("Fetch all remotes")),
        OPTION_FLAG('a', "append", HELP("Append ref names and object names to .git/FETCH_HEAD")),
        OPTION_FLAG('\0', "atomic", HELP("Use atomic transaction when fetching refs")),
        OPTION_FLAG('\0', "force", HELP("Force update of local refs")),
        OPTION_FLAG('\0', "keep", HELP("Keep downloaded pack")),
        OPTION_FLAG('m', "multiple", HELP("Allow several repositories to be fetched")),
        OPTION_FLAG('\0', "dry-run", HELP("Show what would be done")),
        OPTION_FLAG('\0', "porcelain", HELP("Produce machine-readable output")),
    GROUP_END(),
    
    GROUP_START("Shallow clone options"),
        OPTION_STRING('\0', "depth", HELP("Limit fetching to ancestor-chains not longer than n")),
        OPTION_STRING('\0', "deepen", HELP("Deepen history of shallow repository")),
        OPTION_STRING('\0', "shallow-since", HELP("Deepen history of shallow repository since date")),
        OPTION_STRING('\0', "shallow-exclude", HELP("Deepen history excluding ref")),
        OPTION_FLAG('\0', "unshallow", HELP("Convert shallow repository to complete one")),
        OPTION_FLAG('\0', "update-shallow", HELP("Accept refs that update .git/shallow")),
        OPTION_STRING('\0', "negotiation-tip", HELP("Report that we have only objects reachable from this commit")),
    GROUP_END(),
    
    GROUP_START("Tag and reference options"),
        OPTION_FLAG('t', "tags", HELP("Fetch all tags from remote")),
        OPTION_FLAG('\0', "no-tags", HELP("Don't fetch tags from remote")),
        OPTION_FLAG('\0', "prune", HELP("Remove any remote-tracking references that no longer exist on the remote")),
        OPTION_FLAG('P', "prune-tags", HELP("Remove any local tags that no longer exist on the remote")),
        OPTION_STRING('\0', "refmap", HELP("Use this refspec to map the refs to remote-tracking branches")),
        OPTION_STRING('\0', "recurse-submodules",
            HELP("Control recursive fetching of submodules"),
            VALIDATOR(V_CHOICE_STR("yes", "on-demand", "no")),
            FLAGS(FLAG_OPTIONAL)),
        OPTION_FLAG('\0', "no-recurse-submodules", HELP("Disable recursive fetching of submodules")),
    GROUP_END(),
    
    GROUP_START("Network options"),
        OPTION_FLAG('4', "ipv4", HELP("Use IPv4 addresses only")),
        OPTION_FLAG('6', "ipv6", HELP("Use IPv6 addresses only")),
        OPTION_STRING('\0', "upload-pack", HELP("Path to upload pack on remote end")),
        OPTION_FLAG('\0', "set-upstream", HELP("Set upstream for git pull/status")),
        OPTION_FLAG('\0', "write-fetch-head", HELP("Write the list of remote refs to .git/FETCH_HEAD")),
        OPTION_FLAG('\0', "no-write-fetch-head", HELP("Don't write .git/FETCH_HEAD")),
        OPTION_FLAG('\0', "auto-maintenance", HELP("Run 'git maintenance run --auto' at the end")),
        OPTION_FLAG('\0', "auto-gc", HELP("Run 'git gc --auto' at the end")),
    GROUP_END(),
    
    POSITIONAL_STRING("repository", HELP("Repository to fetch from"), DEFAULT("origin"), FLAGS(FLAG_OPTIONAL)),
    POSITIONAL_MANY_STRING("refspec", HELP("Refspecs to fetch"), FLAGS(FLAG_OPTIONAL)),
)

static int handle_dry_run(argus_t *argus, const char *repository)
{
    (void)repository;
    bool dry_run = argus_get(argus, "dry-run").as_bool;
    bool tags = argus_get(argus, "tags").as_bool;
    
    if (dry_run) {
        printf("From https://github.com/user/repo\n");
        printf(" * [would fetch] branch main -> origin/main\n");
        printf(" * [would fetch] branch feature -> origin/feature\n");
        if (tags)
            printf(" * [would fetch] tag v1.0.0 -> v1.0.0\n");
        return 0;
    }
    return -1;
}

static void handle_shallow_options(argus_t *argus)
{
    bool unshallow = argus_get(argus, "unshallow").as_bool;
    bool quiet = argus_get(argus, "quiet").as_bool;
    const char *depth = argus_get(argus, "depth").as_string;
    
    if (unshallow && !quiet) {
        printf("remote: Enumerating objects: 1234, done.\n");
        printf("remote: Total 1234 (delta 0), reused 0 (delta 0), pack-reused 1234\n");
        printf("Receiving objects: 100%% (1234/1234), 456.78 KiB | 1.23 MiB/s, done.\n");
        printf("Resolving deltas: 100%% (567/567), done.\n");
    }
    
    if (depth && !quiet)
        printf("Fetching to depth %s\n", depth);
}

static void execute_fetch_operation(argus_t *argus, const char *repository)
{
    (void)repository;
    bool all = argus_get(argus, "all").as_bool;
    bool quiet = argus_get(argus, "quiet").as_bool;
    bool verbose = argus_get(argus, "verbose").as_bool;
    bool progress = argus_get(argus, "progress").as_bool;
    
    if (all) {
        printf("Fetching origin\n");
        printf("Fetching upstream\n");
        if (!quiet) {
            printf("From https://github.com/user/repo\n");
            printf(" * branch            main       -> origin/main\n");
            printf(" * branch            develop    -> origin/develop\n");
            printf("From https://github.com/upstream/repo\n");
            printf(" * branch            main       -> upstream/main\n");
        }
        return;
    }
    
    if (!quiet) {
        printf("From https://github.com/user/repo\n");
        
        if (verbose || progress) {
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
        printf(" * branch            %s     -> origin/%s\n", refspec, refspec);
        has_refspecs = true;
    }
    
    if (!has_refspecs) {
        printf(" * branch            main       -> origin/main\n");
        printf(" * [new branch]      feature    -> origin/feature\n");
        printf(" = [up to date]      develop    -> origin/develop\n");
    }
    
    if (tags) {
        printf(" * [new tag]         v1.0.0     -> v1.0.0\n");
        printf(" * [new tag]         v1.1.0     -> v1.1.0\n");
        printf(" = [up to date]      v0.9.0     -> v0.9.0\n");
    }
    
    if (force)
        printf(" + abc1234...def5678 main       -> origin/main  (forced update)\n");
    
    if (verbose) {
        printf("POST git-upload-pack (1234 bytes)\n");
        printf("   abc1234..def5678  main       -> origin/main\n");
        printf(" * [new branch]      feature    -> origin/feature\n");
    }
}

static void handle_prune_operations(argus_t *argus)
{
    bool prune = argus_get(argus, "prune").as_bool;
    bool prune_tags = argus_get(argus, "prune-tags").as_bool;
    bool quiet = argus_get(argus, "quiet").as_bool;
    
    if (!quiet) {
        if (prune)
            printf(" x [deleted]         (none)     -> origin/old-feature\n");
        
        if (prune_tags)
            printf(" x [deleted]         (none)     -> v0.8.0\n");
    }
}

int fetch_handler(argus_t *argus, void *data)
{
    (void)data;
    
    const char *repository = argus_get(argus, "repository").as_string;
    
    int dry_run_result = handle_dry_run(argus, repository);
    if (dry_run_result != -1)
        return dry_run_result;
    
    handle_shallow_options(argus);
    execute_fetch_operation(argus, repository);
    display_fetch_results(argus);
    handle_prune_operations(argus);
    
    return 0;
}