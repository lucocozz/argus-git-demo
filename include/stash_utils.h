#ifndef STASH_UTILS_H
#define STASH_UTILS_H

#include <argus.h>
#include <stdbool.h>

const char* get_stash_param(argus_t *argus, const char *param_name);
void print_stash_operation_result(const char *operation, const char *stash, bool quiet);
void print_stash_diff_files(void);

#endif // STASH_UTILS_H