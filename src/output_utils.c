#include "output_utils.h"
#include "colors.h"
#include <stdio.h>
#include <string.h>

void print_git_status_header(const char *branch)
{
    printf("On branch " COLOR_GREEN("%s") "\n", branch);
    printf("Your branch is up to date with '" COLOR_CYAN("origin/%s") "'.\n\n", branch);
}

void print_file_status_line(const char *status, const char *filename)
{
    if (strcmp(status, "new") == 0) {
        printf("\t" COLOR_GREEN("new file:   %s") "\n", filename);
    } else if (strcmp(status, "modified") == 0) {
        printf("\t" COLOR_YELLOW("modified:   %s") "\n", filename);
    } else if (strcmp(status, "deleted") == 0) {
        printf("\t" COLOR_RED("deleted:    %s") "\n", filename);
    } else if (strcmp(status, "renamed") == 0) {
        printf("\t" COLOR_GREEN("renamed:    %s") "\n", filename);
    }
}

void print_operation_result(const char *operation, const char *target, bool success)
{
    if (success) {
        printf(COLOR_GREEN("%s successful") " for " COLOR_BLUE("%s") "\n", operation, target);
    } else {
        printf(COLOR_RED("%s failed") " for " COLOR_BLUE("%s") "\n", operation, target);
    }
}

void print_branch_line(const char *name, const char *hash, const char *message, bool is_current, bool verbose)
{
    const char *prefix = is_current ? "* " : "  ";
    
    if (verbose) {
        printf("%s" COLOR_GREEN("%-10s") " " COLOR_YELLOW("%s") " %s\n", 
               prefix, name, hash, message);
    } else {
        printf("%s" COLOR_GREEN("%s") "\n", prefix, name);
    }
}

void print_remote_branch_line(const char *name, const char *hash, const char *message, bool verbose)
{
    if (verbose) {
        printf("  " COLOR_CYAN("%-20s") " " COLOR_YELLOW("%s") " %s\n", 
               name, hash, message);
    } else {
        printf("  " COLOR_CYAN("%s") "\n", name);
    }
}