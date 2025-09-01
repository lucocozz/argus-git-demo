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
    status_options,
    HELP_OPTION(),
    
    OPTION_FLAG('v', "verbose", HELP("Be verbose")),
    OPTION_FLAG('s', "short", HELP("Show status concisely")),
    OPTION_FLAG('b', "branch", HELP("Show branch information")),
    OPTION_FLAG('\0', "show-stash", HELP("Show stash information")),
    OPTION_FLAG('\0', "ahead-behind", HELP("Compute full ahead/behind values")),
    OPTION_STRING(
        '\0', "porcelain",
        HELP("Give the output in an easy-to-parse format for scripts. "
             "This is similar to the short output, but will remain stable "
             "across Git versions and regardless of user configuration"),
        VALIDATOR(V_CHOICE_STR("v1", "v2", "1", "2")),
        FLAGS(FLAG_OPTIONAL)
    ),
    OPTION_FLAG('\0', "long", HELP("Show status in long format (default)")),
    OPTION_FLAG('z', "null", HELP("Terminate entries with NUL")),
    OPTION_STRING(
        'u', "untracked-files",
        HELP("Show untracked files, optional modes: all, normal, no"),
        VALIDATOR(V_CHOICE_STR("no", "normal", "all")),
        DEFAULT("all"),
        FLAGS(FLAG_OPTIONAL)
    ),
    OPTION_STRING(
        '\0', "ignored",
        HELP("Show ignored files, optional modes: traditional, matching, no"),
        VALIDATOR(V_CHOICE_STR("traditional", "matching", "no")),
        DEFAULT("traditional"),
        FLAGS(FLAG_OPTIONAL)
    ),
    OPTION_STRING(
        '\0', "ignore-submodules",
        HELP("Ignore changes to submodules, optional when: all, dirty, untracked"),
        VALIDATOR(V_CHOICE_STR("all", "dirty", "untracked")),
        DEFAULT("all"),
        FLAGS(FLAG_OPTIONAL)
    ),
    OPTION_FLAG('\0', "column", HELP("List untracked files in columns")),
    OPTION_FLAG('\0', "no-renames", HELP("Do not detect renames")),
    OPTION_FLAG('\0', "renames", HELP("Opposite of --no-renames")),
    OPTION_STRING(
        'M', "find-renames",
        HELP("Detect renames, optionally set similarity index"),
        HINT("n"),
        FLAGS(FLAG_OPTIONAL)
    ),
    
    POSITIONAL_MANY_STRING(
        "pathspec",
        HELP("Paths to limit the status output"),
        FLAGS(FLAG_OPTIONAL)
    ),
)

static void print_porcelain_status(argus_t *argus, const git_file_status_t files[], int count)
{
    bool show_branch = argus_get(argus, "branch").as_bool;
    const char *untracked_mode = argus_get(argus, "untracked-files").as_string;
    const char *ignored_mode = argus_get(argus, "ignored").as_string;
    
    if (show_branch)
        printf("## main...origin/main\n");
    
    for (int i = 0; i < count; i++) {
        const git_file_status_t *file = &files[i];
        
        if (file->staged && file->modified)
            printf("MM %s\n", file->filename);
        else if (file->staged)
            printf("A  %s\n", file->filename);
        else if (file->modified)
            printf("M  %s\n", file->filename);
        else if (strcmp(file->status, "untracked") == 0 && strcmp(untracked_mode, "no") != 0)
            printf("?? %s\n", file->filename);
        else if (strcmp(file->status, "ignored") == 0 && ignored_mode && strcmp(ignored_mode, "no") != 0)
            printf("!! %s\n", file->filename);
    }
}

static void print_standard_status(argus_t *argus, const git_file_status_t files[], int count)
{
    bool verbose = argus_get(argus, "verbose").as_bool;
    bool show_stash = argus_get(argus, "show-stash").as_bool;
    const char *untracked_mode = argus_get(argus, "untracked-files").as_string;
    const char *ignored_mode = argus_get(argus, "ignored").as_string;
    
    printf("On branch " COLOR_GREEN("main") "\n");
    printf("Your branch is up to date with '" COLOR_CYAN("origin/main") "'.\n\n");
    
    if (show_stash)
        printf("Your stash currently has 2 entries\n\n");
    
    printf(COLOR_BLUE("Changes to be committed:") "\n");
    printf("  (use \"git restore --staged <file>...\" to unstage)\n");
    
    for (int i = 0; i < count; i++) {
        const git_file_status_t *file = &files[i];
        if (file->staged) {
            printf("\t" COLOR_GREEN("new file:   %s") "\n", file->filename);
            if (verbose) {
                printf("\n" COLOR_RED("---") " /dev/null\n");
                printf(COLOR_GREEN("+++") " b/%s\n", file->filename);
                printf("@@ -0,0 +1,3 @@\n");
                printf("+This is a new file\n");
                printf("+with some content\n");
                printf("+for demonstration\n");
            }
        }
    }
    printf("\n");
    
    printf(COLOR_YELLOW("Changes not staged for commit:") "\n");
    printf("  (use \"git add <file>...\" to update what will be committed)\n");
    printf("  (use \"git restore <file>...\" to discard changes in working directory)\n");
    
    for (int i = 0; i < count; i++) {
        const git_file_status_t *file = &files[i];
        if (file->modified && !file->staged)
            printf("\t" COLOR_YELLOW("modified:   %s") "\n", file->filename);
    }
    printf("\n");
    
    if (strcmp(untracked_mode, "no") != 0) {
        printf(COLOR_YELLOW("Untracked files:") "\n");
        printf("  (use \"git add <file>...\" to include in what will be committed)\n");
        
        for (int i = 0; i < count; i++) {
            const git_file_status_t *file = &files[i];
            if (strcmp(file->status, "untracked") == 0)
                printf("\t" COLOR_YELLOW("%s") "\n", file->filename);
        }
        printf("\n");
    }
    
    if (ignored_mode && strcmp(ignored_mode, "no") != 0) {
        printf("Ignored files:\n");
        printf("  (use \"git add -f <file>...\" if you really want to add them)\n");
        
        for (int i = 0; i < count; i++) {
            const git_file_status_t *file = &files[i];
            if (strcmp(file->status, "ignored") == 0)
                printf("\t%s\n", file->filename);
        }
        printf("\n");
    }
    
    printf("nothing added to commit but untracked files present (use \"git add\" to track)\n");
}

int status_handler(argus_t *argus, void *data)
{
    (void)data;
    
    int file_count;
    const git_file_status_t *files = get_mock_file_status(&file_count);
    
    bool short_format = argus_get(argus, "short").as_bool;
    const char *porcelain = argus_get(argus, "porcelain").as_string;
    
    if (porcelain || short_format)
        print_porcelain_status(argus, files, file_count);
    else
        print_standard_status(argus, files, file_count);
    
    return 0;
}