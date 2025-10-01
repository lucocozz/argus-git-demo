#ifndef GIT_TYPES_H
#define GIT_TYPES_H

#include <stdbool.h>

typedef struct {
    const char *hash;
    const char *message;
    const char *author;
    const char *date;
    const char *email;
} git_commit_t;

typedef struct {
    const char *name;
    const char *hash;
    const char *message;
    const char *type;
} git_branch_t;

typedef struct {
    const char *filename;
    const char *status;
    bool staged;
    bool modified;
} git_file_status_t;

typedef struct {
    const char *name;
    const char *url;
    const char *push_url;
    const char *type;
} git_remote_t;

typedef struct {
    int index;
    const char *description;
    const char *branch;
    const char *timestamp;
} git_stash_entry_t;

typedef struct {
    const char *key;
    const char *value;
    const char *scope;
} git_config_entry_t;

#endif // GIT_TYPES_H