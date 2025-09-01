#include <argus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands/stash.h"
#include "colors.h"

ARGUS_OPTIONS(
    stash_show_options,
    HELP_OPTION(),
    OPTION_FLAG(
        'p', "patch",
        HELP("Show the patch")
    ),
    OPTION_FLAG(
        0, "stat",
        HELP("Show diffstat")
    ),
    OPTION_FLAG(
        0, "name-only",
        HELP("Show only names of changed files")
    ),
    OPTION_FLAG(
        0, "name-status",
        HELP("Show names and status of changed files")
    ),
    POSITIONAL_STRING("stash", HELP("The stash entry to show"), DEFAULT("stash@{0}"), FLAGS(FLAG_OPTIONAL)),
)

int stash_show_handler(argus_t *argus, void *data) {
    (void)data;
    
    const char *stash = argus_get(argus, "stash").as_string;
    bool patch = argus_get(argus, "patch").as_bool;
    bool stat = argus_get(argus, "stat").as_bool;
    bool name_only = argus_get(argus, "name-only").as_bool;
    bool name_status = argus_get(argus, "name-status").as_bool;
    
    printf("Showing %s\n", stash);
    
    if (name_only) {
        printf("src/main.c\n");
        printf("src/utils.c\n");
        printf("README.md\n");
    } else if (name_status) {
        printf("M\tsrc/main.c\n");
        printf("M\tsrc/utils.c\n");
        printf("A\tREADME.md\n");
    } else if (patch) {
        printf("diff --git a/src/main.c b/src/main.c\n");
        printf("index abc1234..def5678 100644\n");
        printf(COLOR_RED("---") " a/src/main.c\n");
        printf(COLOR_GREEN("+++") " b/src/main.c\n");
        printf("@@ -10,6 +10,9 @@ int main() {\n");
        printf("     printf(\"Hello World\\n\");\n");
        printf("+    // TODO: Add more functionality\n");
        printf("+    printf(\"Debug: Starting application\\n\");\n");
        printf("+\n");
        printf("     return 0;\n");
        printf(" }\n");
    } else if (stat) {
        printf(" README.md   |  5 " COLOR_GREEN("+++++") "\n");
        printf(" src/main.c |  3 " COLOR_GREEN("+++") "\n");
        printf(" src/utils.c|  8 " COLOR_GREEN("++++++++") "\n");
        printf(" 3 files changed, 16 insertions(+)\n");
        printf("\nDetailed statistics:\n");
        printf("  Total lines added: 16\n");
        printf("  Total lines removed: 0\n");
        printf("  Files modified: 3\n");
    } else {
        printf(" README.md   |  5 " COLOR_GREEN("+++++") "\n");
        printf(" src/main.c |  3 " COLOR_GREEN("+++") "\n");
        printf(" src/utils.c|  8 " COLOR_GREEN("++++++++") "\n");
        printf(" 3 files changed, 16 insertions(+)\n");
    }
    
    return 0;
}