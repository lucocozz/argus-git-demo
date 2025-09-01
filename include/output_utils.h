#ifndef OUTPUT_UTILS_H
#define OUTPUT_UTILS_H

#include <stdbool.h>

void print_git_status_header(const char *branch);
void print_file_status_line(const char *status, const char *filename);
void print_operation_result(const char *operation, const char *target, bool success);
void print_branch_line(const char *name, const char *hash, const char *message, bool is_current, bool verbose);
void print_remote_branch_line(const char *name, const char *hash, const char *message, bool verbose);

#endif // OUTPUT_UTILS_H