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
    log_options,
    HELP_OPTION(),
    
    GROUP_START("Output format"),
        OPTION_FLAG('\0', "oneline", HELP("Show each commit on a single line")),
        OPTION_STRING('\0', "pretty",
            HELP("Pretty-print format"),
            VALIDATOR(V_CHOICE_STR("oneline", "short", "medium", "full", "fuller")),
            DEFAULT("medium"),
            FLAGS(FLAG_OPTIONAL)),
        OPTION_STRING('\0', "format", HELP("Custom format string")),
        OPTION_FLAG('\0', "abbrev-commit", HELP("Show abbreviated commit hash")),
    GROUP_END(),
    
    GROUP_START("Display options"),
        OPTION_FLAG('\0', "graph", HELP("Draw graphical commit history")),
        OPTION_STRING('\0', "decorate",
            HELP("Show ref names"),
            VALIDATOR(V_CHOICE_STR("short", "full", "no")),
            DEFAULT("short"),
            FLAGS(FLAG_OPTIONAL)),
    GROUP_END(),
    
    GROUP_START("Diff options"),
        OPTION_FLAG('p', "patch", HELP("Generate patch")),
        OPTION_FLAG('\0', "stat", HELP("Generate diffstat")),
        OPTION_FLAG('\0', "shortstat", HELP("Show summary line only")),
        OPTION_FLAG('\0', "name-only", HELP("Show only filenames")),
        OPTION_FLAG('\0', "name-status", HELP("Show filenames with status")),
    GROUP_END(),
    
    GROUP_START("Limit options"),
        OPTION_INT('n', "max-count", HELP("Limit number of commits"), HINT("number")),
        OPTION_INT('\0', "skip", HELP("Skip commits"), HINT("number")),
    GROUP_END(),
    
    POSITIONAL_MANY_STRING("revision", HELP("Show commits from revisions"), FLAGS(FLAG_OPTIONAL)),
)

static void format_commit_hash(const git_commit_t *commit, argus_t *argus, char *display_hash)
{
    bool abbrev_commit = argus_get(argus, "abbrev-commit").as_bool;
    bool oneline = argus_get(argus, "oneline").as_bool;
    const char *pretty = argus_get(argus, "pretty").as_string;
    
    if (abbrev_commit || oneline || (pretty && strcmp(pretty, "oneline") == 0))
        snprintf(display_hash, 8, "%s", commit->hash);
    else
        strcpy(display_hash, commit->hash);
}

static void print_commit_decorations(int commit_index, argus_t *argus)
{
    const char *decorate = argus_get(argus, "decorate").as_string;
    
    if (decorate && strcmp(decorate, "no") != 0) {
        if (commit_index == 0)
            printf(" (HEAD -> " COLOR_GREEN("main") ", " COLOR_CYAN("origin/main") ")");
        else if (commit_index == 2)
            printf(COLOR_BLUE(" (tag: v1.0.0)"));
    }
}

static void print_commit_oneline(const git_commit_t *commit, argus_t *argus, const char *display_hash)
{
    bool graph = argus_get(argus, "graph").as_bool;
    
    if (graph)
        printf("* ");
    printf(COLOR_YELLOW("%s") " %s\n", display_hash, commit->message);
}

static void print_commit_standard(const git_commit_t *commit, argus_t *argus, const char *display_hash, int commit_index)
{
    bool graph = argus_get(argus, "graph").as_bool;
    const char *pretty = argus_get(argus, "pretty").as_string;
    
    if (graph)
        printf("* ");
    printf("commit " COLOR_YELLOW("%s"), display_hash);
    
    print_commit_decorations(commit_index, argus);
    printf("\n");
    
    if (pretty && (strcmp(pretty, "full") == 0 || strcmp(pretty, "fuller") == 0)) {
        printf("Author: " COLOR_BLUE("%s <%s>") "\n", commit->author, commit->email);
        printf("Commit: " COLOR_BLUE("%s <%s>") "\n", commit->author, commit->email);
    } else {
        printf("Author: " COLOR_BLUE("%s <%s>") "\n", commit->author, commit->email);
    }
    
    printf("Date:   %s\n\n", commit->date);
    printf("    %s\n", commit->message);
}

static void print_commit_stats(argus_t *argus)
{
    bool stat = argus_get(argus, "stat").as_bool;
    bool numstat = argus_get(argus, "numstat").as_bool;
    bool shortstat = argus_get(argus, "shortstat").as_bool;
    bool name_only = argus_get(argus, "name-only").as_bool;
    bool name_status = argus_get(argus, "name-status").as_bool;
    
    if (stat) {
        printf("\n src/main.c     | 15 " COLOR_GREEN("+++++++") COLOR_RED("------") "\n");
        printf(" src/utils.c    |  8 " COLOR_GREEN("+++++++") "\n");
        printf(" 2 files changed, 17 insertions(+), 6 deletions(-)\n");
    } else if (numstat) {
        printf("\n17\t6\tsrc/main.c\n");
        printf("8\t0\tsrc/utils.c\n");
    } else if (shortstat) {
        printf("\n 2 files changed, 17 insertions(+), 6 deletions(-)\n");
    } else if (name_only) {
        printf("\nsrc/main.c\n");
        printf("src/utils.c\n");
    } else if (name_status) {
        printf("\nM\tsrc/main.c\n");
        printf("A\tsrc/utils.c\n");
    }
}

static void print_commit_patch(argus_t *argus)
{
    bool patch = argus_get(argus, "patch").as_bool;
    
    if (patch) {
        printf("\ndiff --git a/src/main.c b/src/main.c\n");
        printf("index abc1234..def5678 100644\n");
        printf(COLOR_RED("---") " a/src/main.c\n");
        printf(COLOR_GREEN("+++") " b/src/main.c\n");
        printf("@@ -10,6 +10,9 @@ int main() {\n");
        printf("+    // New functionality added\n");
        printf("     return 0;\n");
        printf(" }\n");
    }
}

static void display_commits(argus_t *argus, const git_commit_t *commits, int start, int end)
{
    bool oneline = argus_get(argus, "oneline").as_bool;
    const char *pretty = argus_get(argus, "pretty").as_string;
    const char *custom_format = argus_get(argus, "format").as_string;
    
    for (int i = start; i < end; i++) {
        const git_commit_t *commit = &commits[i];
        char display_hash[41];
        
        format_commit_hash(commit, argus, display_hash);
        
        if (oneline || (pretty && strcmp(pretty, "oneline") == 0)) {
            print_commit_oneline(commit, argus, display_hash);
        } else if (custom_format) {
            printf("%s %s by %s\n", display_hash, commit->message, commit->author);
        } else {
            print_commit_standard(commit, argus, display_hash, i);
            print_commit_stats(argus);
            print_commit_patch(argus);
            printf("\n");
        }
    }
}

int log_handler(argus_t *argus, void *data)
{
    (void)data;
    
    int total_count;
    const git_commit_t *commits = get_mock_commits(&total_count);
    int max_count = argus_get(argus, "max-count").as_int;
    int skip = argus_get(argus, "skip").as_int;
    
    int start_index = skip;
    int end_count = total_count;
    
    if (max_count > 0)
        end_count = start_index + max_count;
    if (end_count > total_count)
        end_count = total_count;
    
    if (argus_is_set(argus, "revision")) {
        argus_array_it_t it = argus_array_it(argus, "revision");
        while (argus_array_next(&it)) {
            printf("Showing commits for revision: %s\n", it.value.as_string);
        }
        printf("\n");
    }
    
    display_commits(argus, commits, start_index, end_count);
    return 0;
}