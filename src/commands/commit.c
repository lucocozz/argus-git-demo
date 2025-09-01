#include <argus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "commands/git.h"
#include "colors.h"
#include "git_types.h"

ARGUS_OPTIONS(
    commit_options,
    HELP_OPTION(),
    
    GROUP_START("Commit message options"),
        OPTION_ARRAY_STRING('m', "message",
            HELP("Use the given <msg> as the commit message. "
                 "If multiple -m options are given, their values "
                 "are concatenated as separate paragraphs"),
            FLAGS(FLAG_UNIQUE)),
        OPTION_STRING('F', "file", HELP("Take the commit message from the given file")),
        OPTION_STRING('\0', "author",
            HELP("Override the commit author. Specify an explicit "
                 "author using the standard A U Thor <author@example.com> format")),
        OPTION_STRING('\0', "date", HELP("Override the author date used in the commit")),
        OPTION_STRING('c', "reedit-message",
            HELP("Take an existing commit object and reuse the log "
                 "message and the authorship information when creating the commit")),
        OPTION_STRING('C', "reuse-message",
            HELP("Take an existing commit object and reuse the log "
                 "message and the authorship information when creating the commit")),
        OPTION_STRING('\0', "fixup",
            HELP("Create a new commit which \"fixes up\" <commit> "
                 "when applied with git rebase --autosquash")),
        OPTION_STRING('\0', "squash",
            HELP("Construct a commit message for use with rebase --autosquash")),
        OPTION_FLAG('\0', "reset-author",
            HELP("When used with -C/-c/--amend options, or when "
                 "committing after a conflicting cherry-pick, declare "
                 "that the authorship of the resulting commit now belongs to the committer")),
        OPTION_FLAG('s', "signoff", HELP("Add a Signed-off-by trailer")),
        OPTION_STRING('t', "template",
            HELP("When editing the commit message, start the editor "
                 "with the contents in the given file")),
        OPTION_FLAG('e', "edit", HELP("Force edit of commit")),
        OPTION_STRING('\0', "cleanup",
            HELP("This option determines how the supplied commit "
                 "message should be cleaned up before committing"),
            VALIDATOR(V_CHOICE_STR("strip", "whitespace", "verbatim", "scissors", "default")),
            DEFAULT("default")),
    GROUP_END(),
    
    GROUP_START("Commit contents options"),
        OPTION_FLAG('a', "all",
            HELP("Tell the command to automatically stage files that "
                 "have been modified and deleted, but new files you have "
                 "not told Git about are not affected")),
        OPTION_FLAG('i', "include", HELP("Add specified files to index for commit")),
        OPTION_FLAG('\0', "interactive", HELP("Interactively add files")),
        OPTION_FLAG('p', "patch",
            HELP("Use the interactive patch selection interface "
                 "to choose which changes to commit")),
        OPTION_FLAG('o', "only", HELP("Commit only specified files")),
        OPTION_FLAG('n', "no-verify", HELP("Bypass pre-commit and commit-msg hooks")),
        OPTION_FLAG('\0', "verify", HELP("Opposite of --no-verify")),
        OPTION_FLAG('\0', "dry-run", HELP("Show what would be committed")),
        OPTION_FLAG('\0', "short", HELP("Show status concisely")),
        OPTION_FLAG('\0', "branch", HELP("Show branch information")),
        OPTION_FLAG('\0', "porcelain", HELP("Machine-readable output")),
        OPTION_FLAG('\0', "long", HELP("Show status in long format (default)")),
        OPTION_FLAG('z', "null", HELP("Terminate entries with NUL")),
        OPTION_FLAG('\0', "amend", HELP("Amend previous commit")),
        OPTION_FLAG('\0', "allow-empty",
            HELP("Usually recording a commit that has the exact same "
                 "tree as its sole parent commit is a mistake")),
        OPTION_FLAG('\0', "allow-empty-message",
            HELP("Like --allow-empty this command is primarily "
                 "for use by foreign SCM interface scripts")),
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
    const char *file = argus_is_set(argus, "file") ? argus_get(argus, "file").as_string : NULL;
    bool amend = argus_get(argus, "amend").as_bool;
    bool allow_empty = argus_get(argus, "allow-empty").as_bool;
    
    if (!has_messages && !file && !amend && !allow_empty) {
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
        printf("\t" COLOR_GREEN("new file:   new-file.txt") "\n");
        printf("\t" COLOR_GREEN("modified:   modified-file.txt") "\n");
        return 0;
    }
    return -1;
}

static void handle_patch_mode(argus_t *argus)
{
    bool patch = argus_get(argus, "patch").as_bool;
    
    if (patch) {
        printf("Entering interactive patch mode...\n");
        printf("diff --git a/modified-file.txt b/modified-file.txt\n");
        printf("Stage this hunk [y,n,q,a,d,e,?]? y\n");
    }
}

static void create_commit(argus_t *argus)
{
    bool verbose = argus_get(argus, "verbose").as_bool;
    bool quiet = argus_get(argus, "quiet").as_bool;
    bool amend = argus_get(argus, "amend").as_bool;
    bool signoff = argus_get(argus, "signoff").as_bool;
    const char *author = argus_is_set(argus, "author") ? argus_get(argus, "author").as_string : NULL;
    
    if (amend)
        printf("[" COLOR_GREEN("main") " " COLOR_YELLOW("abc1234") "]" " " COLOR_BLUE("Updated commit message") "\n");
    else
        printf("[" COLOR_GREEN("main") " " COLOR_YELLOW("def5678") "]" " " COLOR_BLUE("Add new feature") "\n");
    
    if (verbose) {
        printf(" 2 files changed, 15 insertions(+), 3 deletions(-)\n");
        printf(" create mode 100644 new-file.txt\n");
    } else if (!quiet) {
        printf(" 2 files changed, 15 insertions(+), 3 deletions(-)\n");
    }
    
    if (author)
        printf("Author: %s\n", author);
    
    if (signoff)
        printf("Signed-off-by: John Doe <john.doe@example.com>\n");
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