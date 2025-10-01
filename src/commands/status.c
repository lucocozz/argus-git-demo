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
    
    GROUP_START("Output format"),
        OPTION_FLAG('s', "short", HELP("Show status concisely")),
        OPTION_STRING('\0', "porcelain",
            HELP("Machine-readable output format"),
            VALIDATOR(V_CHOICE_STR("v1", "v2")),
            FLAGS(FLAG_OPTIONAL)),
        OPTION_FLAG('v', "verbose", HELP("Show staged diff")),
        OPTION_FLAG('z', "null", HELP("Terminate entries with NUL")),
    GROUP_END(),
    
    GROUP_START("Display options"),
        OPTION_FLAG('b', "branch", HELP("Show branch information")),
        OPTION_FLAG('\0', "show-stash", HELP("Show stash information")),
        OPTION_FLAG('\0', "ahead-behind", HELP("Compute full ahead/behind values")),
    GROUP_END(),
    
    GROUP_START("File filtering"),
        OPTION_STRING('u', "untracked-files",
            HELP("Show untracked files"),
            VALIDATOR(V_CHOICE_STR("no", "normal", "all")),
            DEFAULT("all"),
            FLAGS(FLAG_OPTIONAL)),
        OPTION_STRING('\0', "ignored",
            HELP("Show ignored files"),
            VALIDATOR(V_CHOICE_STR("no", "traditional", "matching")),
            DEFAULT("no"),
            FLAGS(FLAG_OPTIONAL)),
    GROUP_END(),
    
    POSITIONAL_MANY_STRING("pathspec",
        HELP("Paths to limit the status output"),
        FLAGS(FLAG_OPTIONAL)),
)

static void print_porcelain_status(argus_t *argus, const git_file_status_t files[], int count)
{
    bool show_branch = argus_get(argus, "branch").as_bool;
    bool null_term = argus_get(argus, "null").as_bool;
    bool ahead_behind = argus_get(argus, "ahead-behind").as_bool;
    const char *untracked_mode = argus_get(argus, "untracked-files").as_string;
    const char *ignored_mode = argus_get(argus, "ignored").as_string;
    const char *term = null_term ? "\0" : "\n";
    
    int branch_count;
    const git_branch_t *branches = get_mock_branches(&branch_count);
    const char *current = branch_count > 0 ? branches[0].name : "main";
    
    if (show_branch) {
        if (ahead_behind)
            printf("## %s...origin/%s [ahead 2, behind 1]%s", current, current, term);
        else
            printf("## %s...origin/%s%s", current, current, term);
    }
    
    for (int i = 0; i < count; i++) {
        const git_file_status_t *file = &files[i];
        
        if (file->staged && file->modified)
            printf("MM %s%s", file->filename, term);
        else if (file->staged)
            printf("A  %s%s", file->filename, term);
        else if (file->modified)
            printf(" M %s%s", file->filename, term);
        else if (strcmp(file->status, "untracked") == 0 && strcmp(untracked_mode, "no") != 0)
            printf("?? %s%s", file->filename, term);
        else if (strcmp(file->status, "ignored") == 0 && strcmp(ignored_mode, "no") != 0)
            printf("!! %s%s", file->filename, term);
    }
}

static void print_standard_status(argus_t *argus, const git_file_status_t files[], int count)
{
    bool verbose = argus_get(argus, "verbose").as_bool;
    bool show_stash = argus_get(argus, "show-stash").as_bool;
    bool ahead_behind = argus_get(argus, "ahead-behind").as_bool;
    const char *untracked_mode = argus_get(argus, "untracked-files").as_string;
    const char *ignored_mode = argus_get(argus, "ignored").as_string;
    
    int branch_count, remote_count;
    const git_branch_t *branches = get_mock_branches(&branch_count);
    const git_remote_t *remotes = get_mock_remotes(&remote_count);
    
    const char *current = branch_count > 0 ? branches[0].name : "main";
    const char *remote = remote_count > 0 ? remotes[0].name : "origin";
    
    printf("On branch " COLOR_GREEN("%s") "\n", current);
    
    if (ahead_behind)
        printf("Your branch is ahead of '%s/%s' by 2 commits, behind by 1.\n\n", remote, current);
    else
        printf("Your branch is up to date with '" COLOR_CYAN("%s/%s") "'.\n\n", remote, current);
    
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

static int handle_porcelain_format(argus_t *argus)
{
    bool short_format = argus_get(argus, "short").as_bool;
    const char *porcelain = argus_get(argus, "porcelain").as_string;
    
    if (porcelain || short_format) {
        int file_count;
        const git_file_status_t *files = get_mock_file_status(&file_count);
        print_porcelain_status(argus, files, file_count);
        return 0;
    }
    return -1;
}

static void display_standard_status(argus_t *argus)
{
    int file_count;
    const git_file_status_t *files = get_mock_file_status(&file_count);
    
    bool has_pathspec = argus_is_set(argus, "pathspec");
    if (has_pathspec) {
        argus_array_it_t it = argus_array_it(argus, "pathspec");
        while (argus_array_next(&it)) {
            printf("Checking status for: %s\n", it.value.as_string);
        }
        printf("\n");
    }
    
    print_standard_status(argus, files, file_count);
}

int status_handler(argus_t *argus, void *data)
{
    (void)data;
    
    int result;
    
    if ((result = handle_porcelain_format(argus)) != -1)
        return result;
    
    display_standard_status(argus);
    return 0;
}