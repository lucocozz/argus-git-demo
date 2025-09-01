#ifndef VALIDATION_UTILS_H
#define VALIDATION_UTILS_H

#include <stdbool.h>
#include <ctype.h>

bool is_valid_remote_name(const char *name);
bool is_valid_stash_ref(const char *ref);
bool is_valid_url(const char *url);
bool is_valid_branch_name(const char *name);

#endif // VALIDATION_UTILS_H