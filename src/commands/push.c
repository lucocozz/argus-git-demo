#include <argus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "commands/git.h"
#include "colors.h"
#include "git_types.h"

ARGUS_OPTIONS(
    push_options,
    HELP_OPTION(),
    
    GROUP_START("General options"),
        OPTION_FLAG('q', "quiet", HELP("Operate quietly")),
        OPTION_FLAG('v', "verbose", HELP("Be more verbose")),
        OPTION_FLAG('\0', "progress", HELP("Force progress reporting")),
        OPTION_FLAG('\0', "no-progress", HELP("Do not show progress")),
        OPTION_FLAG('n', "dry-run", HELP("Do everything except actually send the updates")),
        OPTION_FLAG('\0', "porcelain", HELP("Produce machine-readable output")),
        OPTION_FLAG('\0', "all", HELP("Push all refs under refs/heads/")),
        OPTION_FLAG('\0', "mirror", HELP("Mirror all refs")),
        OPTION_FLAG('d', "delete", HELP("Delete refs")),
        OPTION_FLAG('\0', "atomic", HELP("Request atomic transaction on remote side")),
        OPTION_FLAG('\0', "no-atomic", HELP("Do not request atomic transaction")),
    GROUP_END(),
    
    GROUP_START("Force options"),
        OPTION_FLAG('f', "force", HELP("Force updates")),
        OPTION_FLAG('\0', "force-with-lease", HELP("Force updates with lease")),
        OPTION_FLAG('\0', "force-if-includes", HELP("Force updates if the tip of the remote-tracking ref has been integrated locally")),
        OPTION_FLAG('\0', "no-force-with-lease", HELP("Cancel a previous --force-with-lease")),
    GROUP_END(),
    
    GROUP_START("Upstream options"),
        OPTION_FLAG('u', "set-upstream", HELP("Set upstream for git pull/status")),
        OPTION_STRING('\0', "upstream", HELP("Equivalent to --set-upstream-to")),
        OPTION_FLAG('\0', "no-verify", HELP("Bypass pre-push hook")),
        OPTION_FLAG('\0', "verify", HELP("Run pre-push hook")),
        OPTION_FLAG('\0', "follow-tags", HELP("Push missing annotated tags")),
        OPTION_FLAG('\0', "signed", HELP("GPG sign the push")),
        OPTION_FLAG('\0', "no-signed", HELP("Do not GPG sign the push")),
    GROUP_END(),
    
    GROUP_START("Tag and submodule options"),
        OPTION_FLAG('\0', "tags", HELP("Push all refs under refs/tags")),
        OPTION_FLAG('\0', "no-tags", HELP("Don't push tags")),
        OPTION_STRING('\0', "recurse-submodules",
            HELP("Control recursive pushing of submodules"),
            VALIDATOR(V_CHOICE_STR("check", "on-demand", "only", "no")),
            FLAGS(FLAG_OPTIONAL)),
        OPTION_FLAG('\0', "no-recurse-submodules", HELP("Disable recursive pushing of submodules")),
        OPTION_FLAG('\0', "thin", HELP("Use thin pack")),
        OPTION_FLAG('\0', "no-thin", HELP("Do not use thin pack")),
    GROUP_END(),
    
    GROUP_START("Network options"),
        OPTION_STRING('\0', "receive-pack", HELP("Path to git-receive-pack on the remote")),
        OPTION_STRING('\0', "exec", HELP("Path to git-receive-pack on the remote")),
        OPTION_FLAG('4', "ipv4", HELP("Use IPv4 addresses only")),
        OPTION_FLAG('6', "ipv6", HELP("Use IPv6 addresses only")),
    GROUP_END(),
    
    POSITIONAL_STRING("repository", HELP("Repository to push to"), DEFAULT("origin"), FLAGS(FLAG_OPTIONAL)),
    POSITIONAL_MANY_STRING("refspec", HELP("Refspecs to push"), FLAGS(FLAG_OPTIONAL)),
)

static int handle_dry_run(argus_t *argus, const char *repository)
{
    (void)repository;
    bool dry_run = argus_get(argus, "dry-run").as_bool;
    bool delete_refs = argus_get(argus, "delete").as_bool;
    
    if (dry_run) {
        printf("To https://github.com/user/repo.git\n");
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

static void validate_push_refs(argus_t *argus)
{
    bool delete_refs = argus_get(argus, "delete").as_bool;
    bool quiet = argus_get(argus, "quiet").as_bool;
    
    if (delete_refs && !quiet)
        printf("Validating refs for deletion...\n");
}

static void execute_push_operation(argus_t *argus, const char *repository)
{
    bool quiet = argus_get(argus, "quiet").as_bool;
    bool verbose = argus_get(argus, "verbose").as_bool;
    bool progress = argus_get(argus, "progress").as_bool;
    
    if (!quiet) {
        if (verbose || progress) {
            printf("Enumerating objects: 15, done.\n");
            printf("Counting objects: 100%% (15/15), done.\n");
            printf("Delta compression using up to 8 threads\n");
            printf("Compressing objects: 100%% (8/8), done.\n");
            printf("Writing objects: 100%% (15/15), 2.45 KiB | 2.45 MiB/s, done.\n");
            printf("Total 15 (delta 3), reused 10 (delta 1), pack-reused 0\n");
        }
        
        printf("To https://github.com/user/repo.git\n");
    }
    
    if (verbose) {
        printf("POST git-receive-pack (1234 bytes)\n");
        printf("remote: Resolving deltas: 100%% (3/3), done.\n");
        printf("To %s\n", repository);
    }
}

static void handle_upstream_tracking(argus_t *argus)
{
    bool set_upstream = argus_get(argus, "set-upstream").as_bool;
    bool quiet = argus_get(argus, "quiet").as_bool;
    
    if (set_upstream && !quiet)
        printf("Branch 'main' set up to track remote branch 'main' from 'origin'.\n");
}

static void display_push_results(argus_t *argus)
{
    bool quiet = argus_get(argus, "quiet").as_bool;
    bool all = argus_get(argus, "all").as_bool;
    bool mirror = argus_get(argus, "mirror").as_bool;
    bool delete_refs = argus_get(argus, "delete").as_bool;
    bool force = argus_get(argus, "force").as_bool;
    bool force_with_lease = argus_get(argus, "force-with-lease").as_bool;
    bool tags = argus_get(argus, "tags").as_bool;
    bool follow_tags = argus_get(argus, "follow-tags").as_bool;
    bool atomic = argus_get(argus, "atomic").as_bool;
    
    if (quiet) 
        return;
    
    if (force || force_with_lease) {
        printf(" + abc1234...def5678 main -> main (" COLOR_YELLOW("forced update") ")\n");
    } else if (all) {
        printf("   abc1234..def5678  main -> main\n");
        printf("   ghi9012..jkl3456  develop -> develop\n");
        printf(" * [new branch]      feature -> feature\n");
    } else if (mirror) {
        printf("   abc1234..def5678  main -> main\n");
        printf(" * [new branch]      feature -> feature\n");
        printf(" - [deleted]         old-feature\n");
    } else if (delete_refs) {
        argus_array_it_t it = argus_array_it(argus, "refspec");
        while (argus_array_next(&it)) {
            const char *refspec = it.value.as_string;
            printf(" - [deleted]         %s\n", refspec);
        }
    } else {
        argus_array_it_t it = argus_array_it(argus, "refspec");
        bool has_refspecs = false;
        while (argus_array_next(&it)) {
            const char *refspec = it.value.as_string;
            printf("   abc1234..def5678  %s -> %s\n", refspec, refspec);
            has_refspecs = true;
        }
        if (!has_refspecs)
            printf("   abc1234..def5678  main -> main\n");
    }
    
    if (tags || follow_tags) {
        printf(" * [new tag]         v1.0.0 -> v1.0.0\n");
        printf(" * [new tag]         v1.1.0 -> v1.1.0\n");
    }
    
    if (atomic)
        printf("remote: Atomic push completed successfully\n");
}

int push_handler(argus_t *argus, void *data)
{
    (void)data;
    
    const char *repository = argus_get(argus, "repository").as_string;
    
    int dry_run_result = handle_dry_run(argus, repository);
    if (dry_run_result != -1)
        return dry_run_result;
    
    validate_push_refs(argus);
    execute_push_operation(argus, repository);
    display_push_results(argus);
    handle_upstream_tracking(argus);
    
    return 0;
}