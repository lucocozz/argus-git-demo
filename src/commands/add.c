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
    add_options,
    HELP_OPTION(),
    
    GROUP_START("Basic options"),
        OPTION_FLAG('n', "dry-run", 
            HELP("Show what would be added without actually adding")),
        OPTION_FLAG('v', "verbose", 
            HELP("Show files being added")),
        OPTION_FLAG('q', "quiet", 
            HELP("Suppress output")),
        OPTION_FLAG('f', "force", 
            HELP("Allow adding ignored files")),
    GROUP_END(),
    
    GROUP_START("Mode options"),
        OPTION_FLAG('A', "all", 
            HELP("Add all tracked and untracked files")),
        OPTION_FLAG('u', "update", 
            HELP("Update only tracked files")),
        OPTION_FLAG('N', "intent-to-add", 
            HELP("Record intent to add files later")),
    GROUP_END(),

    GROUP_START("Interactive options"),
        OPTION_FLAG('i', "interactive", 
            HELP("Interactive file selection")),
        OPTION_FLAG('p', "patch", 
            HELP("Interactive hunk selection")),
    GROUP_END(),
    
    POSITIONAL_MANY_STRING("pathspec",
        HELP("Files to add to the index"),
        FLAGS(FLAG_OPTIONAL),
        VALIDATOR(V_COUNT(0, 100))),
)

static int handle_interactive_modes(argus_t *argus)
{
    bool interactive = argus_get(argus, "interactive").as_bool;
    bool patch = argus_get(argus, "patch").as_bool;
    
    if (interactive) {
        int file_count;
        const git_file_status_t *files = get_mock_file_status(&file_count);
        
        printf("           staged     unstaged path\n");
        for (int i = 0; i < file_count && i < 3; i++) {
            if (files[i].modified) {
                printf("  %d:    unchanged       +1/-1 %s\n", i + 1, files[i].filename);
            }
        }
        printf("\n*** Commands ***\n");
        printf("  1: status\t  2: update\t  3: revert\t  4: add untracked\n");
        printf("  5: patch\t  6: diff\t  7: quit\t  8: help\n");
        printf("What now> q\n");
        return 0;
    }
    
    if (patch) {
        int file_count;
        const git_file_status_t *files = get_mock_file_status(&file_count);
        const char *target_file = file_count > 0 ? files[0].filename : "file.txt";
        
        printf("diff --git a/%s b/%s\n", target_file, target_file);
        printf("index abc1234..def5678 100644\n");
        printf(COLOR_RED("---") " a/%s\n", target_file);
        printf(COLOR_GREEN("+++") " b/%s\n", target_file);
        printf("@@ -1,3 +1,4 @@\n");
        printf(" Line 1\n");
        printf(COLOR_GREEN("+New line added") "\n");
        printf(" Line 2\n");
        printf(" Line 3\n");
        printf("(1/1) Stage this hunk [y,n,q,a,d,e,?]? y\n");
        return 0;
    }
    
    return -1;
}

static int handle_special_modes(argus_t *argus)
{
    bool all = argus_get(argus, "all").as_bool;
    bool update = argus_get(argus, "update").as_bool;
    bool intent_to_add = argus_get(argus, "intent-to-add").as_bool;
    bool dry_run = argus_get(argus, "dry-run").as_bool;
    bool quiet = argus_get(argus, "quiet").as_bool;
    
    if (all || update || intent_to_add) {
        int file_count;
        const git_file_status_t *files = get_mock_file_status(&file_count);
        
        const char *mode_desc = all ? "all tracked and untracked" : 
                               update ? "tracked" : "intent-to-add";
        
        if (dry_run) {
            for (int i = 0; i < file_count; i++) {
                bool should_process = all || 
                                    (update && files[i].modified) ||
                                    (intent_to_add && strcmp(files[i].status, "untracked") == 0);
                if (should_process) {
                    const char *action = intent_to_add ? "intent-to-add" : "add";
                    printf("%s '%s'\n", action, files[i].filename);
                }
            }
        } else if (!quiet) {
            const char *action = intent_to_add ? "Recording intent for" : "Adding";
            printf("%s %s files...\n", action, mode_desc);
        }
        return 0;
    }
    
    return -1;
}

static void process_pathspecs(argus_t *argus)
{
    bool verbose = argus_get(argus, "verbose").as_bool;
    bool dry_run = argus_get(argus, "dry-run").as_bool;
    bool force = argus_get(argus, "force").as_bool;
    bool quiet = argus_get(argus, "quiet").as_bool;
    bool intent_to_add = argus_get(argus, "intent-to-add").as_bool;
    
    if ((verbose || dry_run) && !quiet) {
        const char *action = intent_to_add ? "Recording intent for files:" : "Adding files:";
        printf("%s\n", action);
    }
    
    argus_array_it_t it = argus_array_it(argus, "pathspec");
    int file_count = 0;
    
    while (argus_array_next(&it)) {
        const char *pathspec = it.value.as_string;
        file_count++;
        
        if ((dry_run || verbose) && !quiet) {
            const char *action = intent_to_add ? "intent-to-add" : "add";
            const char *force_note = force ? " (forced)" : "";
            printf("%s '%s'%s\n", action, pathspec, force_note);
        }
    }
    
    if (!dry_run && !verbose && !quiet && file_count > 0) {
        if (intent_to_add)
            printf(COLOR_GREEN("Recorded intent for %d file(s)") "\n", file_count);
        else
            printf(COLOR_GREEN("Added %d file(s) to index") "\n", file_count);
    }
}

int add_handler(argus_t *argus, void *data)
{
    (void)data;
    
    bool all = argus_get(argus, "all").as_bool;
    bool update = argus_get(argus, "update").as_bool;
    bool interactive = argus_get(argus, "interactive").as_bool;
    bool patch = argus_get(argus, "patch").as_bool;
    
    if ((all || update) && (interactive || patch)) {
        printf(COLOR_RED("error: ") "Cannot combine bulk operations with interactive modes\n");
        return 1;
    }
    
    int result;
    
    if ((result = handle_interactive_modes(argus)) != -1)
        return result;
    
    if ((result = handle_special_modes(argus)) != -1)
        return result;
    
    if (!argus_is_set(argus, "pathspec")) {
        printf(COLOR_YELLOW("Nothing specified, nothing added.") "\n");
        printf(COLOR_BLUE("hint: Maybe you wanted to say 'git add .'?") "\n");
        printf(COLOR_BLUE("hint: Turn this message off by running") "\n");
        printf(COLOR_BLUE("hint: \"git config advice.addEmptyPathspec false\"") "\n");
        return 1;
    }
    
    process_pathspecs(argus);
    return 0;
}