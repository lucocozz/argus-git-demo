#include <argus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "argus/regex.h"

#include "commands/git.h"
#include "colors.h"
#include "git_types.h"
#include "mock_data.h"

#define ARGUS_RE_AUTHOR_EMAIL                                                                  \
    MAKE_REGEX(                                                                                \
        "^(([A-Za-z\\s]+)\\s+<([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,})>|[a-zA-Z0-"    \
        "9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,})$",                                            \
        "author-email")

#define V_RE_AUTHOR_EMAIL() V_REGEX(ARGUS_RE_AUTHOR_EMAIL)

ARGUS_OPTIONS(
    commit_options,
    HELP_OPTION(),

    GROUP_START("General options"),
        OPTION_FLAG('v', "verbose",
            HELP("Show unified diff between the HEAD commit and what would be committed")),

        OPTION_FLAG('q', "quiet",
            HELP("Suppress commit summary message")),

        OPTION_FLAG('\0', "dry-run",
            HELP("Show what would be committed")),
    GROUP_END(),

    GROUP_START("Commit message options"),
        OPTION_ARRAY_STRING('m', "message",
            HELP("Use the given <msg> as the commit message. "
                 "If multiple -m options are given, their values "
                 "are concatenated as separate paragraphs"),
            VALIDATOR(V_COUNT(1, 3))),

        OPTION_STRING('F', "file",
            HELP("Take the commit message from the given file"),
            VALIDATOR(V_RE_FILENAME())),

        OPTION_STRING('\0', "author",
            HELP("Override the commit author. Specify an explicit "
                 "author using the standard \"Author <author@example.com>\" format"),
            VALIDATOR(V_RE_AUTHOR_EMAIL())),

        OPTION_FLAG('s', "signoff",
            HELP("Add a Signed-off-by trailer")),
    GROUP_END(),

    GROUP_START("Commit contents options"),
        OPTION_FLAG('a', "all",
            HELP("Tell the command to automatically stage files that "
                 "have been modified and deleted, but new files you have "
                 "not told Git about are not affected")),

        OPTION_FLAG('p', "patch",
            HELP("Use the interactive patch selection interface "
                 "to choose which changes to commit")),

        OPTION_FLAG('\0', "amend",
            HELP("Amend previous commit")),


        OPTION_FLAG('n', "no-verify",
            HELP("Bypass pre-commit and commit-msg hooks")),
    GROUP_END(),

    POSITIONAL_MANY_STRING("pathspec",
        HELP("When pathspec is given on the command line, commit the contents "
             "of the files that match the pathspec without recording the changes "
             "already added to the index"),
        FLAGS(FLAG_OPTIONAL)),
)

static bool validate_commit_message(argus_t *argus)
{
    bool has_messages = argus_is_set(argus, "message");
    const char *file = argus_get(argus, "file").as_string;
    bool amend = argus_get(argus, "amend").as_bool;
    
    if (!has_messages && (!file || strlen(file) == 0) && !amend) {
        printf(COLOR_RED("Aborting commit due to empty commit message.") "\n");
        return false;
    }
    return true;
}

static int handle_dry_run(argus_t *argus)
{
    bool dry_run = argus_get(argus, "dry-run").as_bool;
    
    if (dry_run) {
        printf("On branch " COLOR_GREEN("main") "\n");
        printf("Changes to be committed:\n");
        
        int file_count;
        const git_file_status_t *files = get_mock_file_status(&file_count);
        
        for (int i = 0; i < file_count; i++) {
            if (files[i].staged) {
                if (strcmp(files[i].status, "new") == 0)
                    printf("\t" COLOR_GREEN("new file:   %s") "\n", files[i].filename);
                else if (strcmp(files[i].status, "modified") == 0)
                    printf("\t" COLOR_GREEN("modified:   %s") "\n", files[i].filename);
            }
        }
        return 0;
    }
    return -1;
}

static void handle_patch_mode(argus_t *argus)
{
    bool patch = argus_get(argus, "patch").as_bool;
    
    if (patch) {
        int file_count;
        const git_file_status_t *files = get_mock_file_status(&file_count);
        
        printf("Entering interactive patch mode...\n");
        for (int i = 0; i < file_count; i++) {
            if (files[i].modified) {
                printf("diff --git a/%s b/%s\n", files[i].filename, files[i].filename);
                printf("Stage this hunk [y,n,q,a,d,e,?]? y\n");
                printf("Hunk staged.\n");
            }
        }
    }
}

static char* read_message_from_file(const char *filename)
{
    if (!filename || strlen(filename) == 0)
        return NULL;
        
    FILE *file = fopen(filename, "r");
    if (!file)
        return NULL;
        
    static char buffer[1024];
    if (fgets(buffer, sizeof(buffer), file)) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n')
            buffer[len-1] = '\0';
        fclose(file);
        return buffer;
    }
    
    fclose(file);
    return NULL;
}

static const char* get_commit_message(argus_t *argus)
{
    printf("message count: %ld\n", argus_count(argus, "message"));
    if (argus_is_set(argus, "message")) {
        static char concatenated_message[2048];
        concatenated_message[0] = '\0';
        
        argus_array_it_t it = argus_array_it(argus, "message");
        bool first = true;
        
        while (argus_array_next(&it)) {
            if (!first) {
                strcat(concatenated_message, "\n\n");
            }
            strcat(concatenated_message, it.value.as_string);
            first = false;
        }
        
        if (strlen(concatenated_message) > 0)
            return concatenated_message;
    }
    
    const char *file = argus_get(argus, "file").as_string;
    char *file_message = read_message_from_file(file);
    if (file_message)
        return file_message;
        
    return "Add new feature";
}

static void create_commit(argus_t *argus)
{
    bool verbose = argus_get(argus, "verbose").as_bool;
    bool quiet = argus_get(argus, "quiet").as_bool;
    bool amend = argus_get(argus, "amend").as_bool;
    bool signoff = argus_get(argus, "signoff").as_bool;
    bool all = argus_get(argus, "all").as_bool;
    const char *author = argus_get(argus, "author").as_string;
    
    const char *message = get_commit_message(argus);
    
    int commit_count;
    const git_commit_t *commits = get_mock_commits(&commit_count);
    
    if (amend)
        printf("[" COLOR_GREEN("main") " " COLOR_YELLOW("%s") "]" " " COLOR_BLUE("%s") "\n", 
               commits[0].hash, message);
    else
        printf("[" COLOR_GREEN("main") " " COLOR_YELLOW("def5678") "]" " " COLOR_BLUE("%s") "\n", 
               message);
    
    if (author && strlen(author) > 0)
        printf("Author: %s\n", author);
    
    int file_count;
    const git_file_status_t *files = get_mock_file_status(&file_count);
    int changed_files = 0;
    
    for (int i = 0; i < file_count; i++) {
        if (files[i].staged || (all && files[i].modified))
            changed_files++;
    }
    
    if (verbose) {
        printf(" %d files changed, 15 insertions(+), 3 deletions(-)\n", changed_files);
        for (int i = 0; i < file_count; i++) {
            if (files[i].staged || (all && files[i].modified)) {
                if (strcmp(files[i].status, "new") == 0)
                    printf(" create mode 100644 %s\n", files[i].filename);
                else
                    printf(" modify mode 100644 %s\n", files[i].filename);
            }
        }
    } else if (!quiet) {
        printf(" %d files changed, 15 insertions(+), 3 deletions(-)\n", changed_files);
    }
    
    if (signoff) {
        printf("\n%s\n\nSigned-off-by: John Doe <john.doe@example.com>\n", message);
    }
}

int commit_handler(argus_t *argus, void *data)
{
    (void)data;
    
    if (!validate_commit_message(argus))
        return 1;
    
    int dry_run_result = handle_dry_run(argus);
    if (dry_run_result != -1)
        return dry_run_result;
    
    handle_patch_mode(argus);
    create_commit(argus);
    
    return 0;
}