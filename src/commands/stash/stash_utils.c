#include <argus.h>
#include <stdio.h>
#include <string.h>

#include "stash_utils.h"
#include "colors.h"

const char* get_stash_param(argus_t *argus, const char *param_name)
{
    return argus_is_set(argus, param_name) 
           ? argus_get(argus, param_name).as_string 
           : "stash@{0}";
}

void print_stash_operation_result(const char *operation, const char *stash, bool quiet)
{
    if (!quiet)
    {
        if (strcmp(operation, "apply") == 0)
            printf("Applied stash entry: " COLOR_BLUE("%s") "\n", stash);
        else if (strcmp(operation, "pop") == 0)
            printf("Dropped and applied stash entry: " COLOR_BLUE("%s") "\n", stash);
        else if (strcmp(operation, "drop") == 0)
            printf("Dropped stash entry: " COLOR_BLUE("%s") "\n", stash);
    }
}

void print_stash_diff_files(void)
{
    printf("diff --git a/modified-file.txt b/modified-file.txt\n");
    printf("index abc1234..def5678 100644\n");
    printf("--- a/modified-file.txt\n");
    printf("+++ b/modified-file.txt\n");
    printf("@@ -1,3 +1,4 @@\n");
    printf(" Line 1\n");
    printf("+Added line\n");
    printf(" Line 2\n");
    printf(" Line 3\n");
}