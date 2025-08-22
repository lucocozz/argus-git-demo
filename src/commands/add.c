#include <argus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "commands/git.h"
#include "colors.h"
#include "git_types.h"


ARGUS_OPTIONS(
    add_options,
    HELP_OPTION(),
    
    OPTION_FLAG('n', "dry-run", HELP("Dry run")),
    OPTION_FLAG('v', "verbose", HELP("Be verbose")),
    
    OPTION_FLAG('i', "interactive", HELP("Interactive picking")),
    OPTION_FLAG('p', "patch", HELP("Select hunks interactively")),
    OPTION_FLAG('e', "edit", HELP("Edit current diff and apply")),
    OPTION_FLAG('f', "force", HELP("Allow adding otherwise ignored files")),
    OPTION_FLAG('u', "update", HELP("Update tracked files")),
    OPTION_FLAG('\0', "renormalize", HELP("Renormalize EOL of tracked files (implies -u)")),
    OPTION_FLAG('N', "intent-to-add", HELP("Record only the fact that the path will be added later")),
    OPTION_FLAG(
        'A', "all", 
        HELP("Add changes from all tracked and untracked files")
    ),
    OPTION_FLAG(
        '\0', "ignore-removal", 
        HELP("Ignore paths removed in the working tree (same as --no-all)")
    ),
    OPTION_FLAG('\0', "refresh", HELP("Don't add, only refresh the index")),
    OPTION_FLAG(
        '\0', "ignore-errors", 
        HELP("Just skip files which cannot be added because of errors")
    ),
    OPTION_FLAG(
        '\0', "ignore-missing", 
        HELP("Check if - even missing - files are ignored in dry run")
    ),
    OPTION_FLAG(
        '\0', "sparse", 
        HELP("Allow updating entries outside of the sparse-checkout cone")
    ),
    OPTION_STRING(
        '\0', "chmod",
        HELP("Override the executable bit of the listed files"),
        HINT("(+|-)x"),
        VALIDATOR(V_CHOICE_STR("+x", "-x"))
    ),
    OPTION_STRING(
        '\0', "pathspec-from-file",
        HELP("Read pathspec from file"),
        HINT("file")
    ),
    OPTION_FLAG(
        '\0', "pathspec-file-nul", 
        HELP("With --pathspec-from-file, pathspec elements are "
             "separated with NUL character")
    ),
    
    POSITIONAL_MANY_STRING(
        "pathspec",
        HELP("Files to add content from"),
        FLAGS(FLAG_OPTIONAL)
    ),
)


int add_handler(argus_t *argus, void *data)
{
    (void)data;

    // ========================================================================
    bool        dry_run         = argus_get(argus, "dry-run").as_bool;
    bool        verbose         = argus_get(argus, "verbose").as_bool;
    bool        force           = argus_get(argus, "force").as_bool;
    bool        all             = argus_get(argus, "all").as_bool;
    bool        update          = argus_get(argus, "update").as_bool;
    bool        interactive     = argus_get(argus, "interactive").as_bool;
    bool        patch           = argus_get(argus, "patch").as_bool;
    bool        edit            = argus_get(argus, "edit").as_bool;
    bool        refresh         = argus_get(argus, "refresh").as_bool;
    bool        ignore_errors   = argus_get(argus, "ignore-errors").as_bool;
    bool        renormalize     = argus_get(argus, "renormalize").as_bool;
    const char *chmod_mode      = argus_is_set(argus, "chmod") 
                                  ? argus_get(argus, "chmod").as_string : NULL;
    
    // ========================================================================
    // Handle interactive modes
    if (interactive) {
        printf("           staged     unstaged path\n");
        printf("  1:    unchanged       +1/-1 modified-file.txt\n");
        printf("  2:    unchanged       +5/-0 new-file.txt\n");
        printf("\n*** Commands ***\n");
        printf("  1: status\t  2: update\t  3: revert\t  4: add untracked\n");
        printf("  5: patch\t  6: diff\t  7: quit\t  8: help\n");
        printf("What now> q\n");
        return 0;
    }
    
    if (patch) {
        printf("diff --git a/modified-file.txt b/modified-file.txt\n");
        printf("index abc1234..def5678 100644\n");
        printf(COLOR_RED("---") " a/modified-file.txt\n");
        printf(COLOR_GREEN("+++") " b/modified-file.txt\n");
        printf("@@ -1,3 +1,4 @@\n");
        printf(" Line 1\n");
        printf("+New line added\n");
        printf(" Line 2\n");
        printf(" Line 3\n");
        printf("(1/1) Stage this hunk [y,n,q,a,d,e,?]? y\n");
        return 0;
    }
    
    if (edit) {
        printf("Opening diff in editor...\n");
        return 0;
    }
    
    // Handle refresh mode
    if (refresh) {
        printf("Refreshing index...\n");
        return 0;
    }
    
    // Handle --all or --update options
    if (all) {
        if (dry_run)
            printf("add 'modified-file.txt'\n");
        else
            printf("Adding all tracked and untracked files...\n");
        return 0;
    }
    
    if (update) {
        if (dry_run)
            printf("add 'modified-file.txt'\n");
        else
            printf("Updating tracked files...\n");
        return 0;
    }
    
    // Check if pathspec is provided
    if (!argus_is_set(argus, "pathspec")) {
        printf(COLOR_YELLOW("Nothing specified, nothing added.") "\n");
        printf(COLOR_BLUE("hint: Maybe you wanted to say 'git add .'?") "\n");
        printf(COLOR_BLUE("hint: Turn this message off by running") "\n");
        printf(COLOR_BLUE("hint: \"git config advice.addEmptyPathspec false\"") "\n");
        return 1;
    }
    
    if (verbose || dry_run)
        printf("Adding files:\n");
    
    // Process all pathspec arguments
    // Process all pathspec arguments
    argus_array_it_t it = argus_array_it(argus, "pathspec");
    int file_count = 0;
    while (argus_array_next(&it)) {
        const char *pathspec = it.value.as_string;
        file_count++;
        
        if (dry_run || verbose) {
            printf("add '%s'\n", pathspec);
            if (force)
                printf("  (forced)\n");
            if (renormalize)
                printf("  (renormalized)\n");
            if (chmod_mode)
                printf("  (chmod %s)\n", chmod_mode);
        }
        
        if (ignore_errors && file_count == 2) {
            printf("error: pathspec '%s' did not match any files\n", pathspec);
            printf("(continuing due to --ignore-errors)\n");
        }
    }
    
    if (!dry_run && !verbose && file_count > 0)
        printf(COLOR_GREEN("Added %d file(s) to index") "\n", file_count);

    return 0;
}