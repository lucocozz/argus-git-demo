#ifndef GIT_H
#define GIT_H

#include <argus/types.h>

// External option declarations for main commands
extern argus_option_t init_options[];
extern argus_option_t add_options[];
extern argus_option_t commit_options[];
extern argus_option_t status_options[];
extern argus_option_t log_options[];
extern argus_option_t branch_options[];
extern argus_option_t pull_options[];
extern argus_option_t fetch_options[];
extern argus_option_t push_options[];
extern argus_option_t checkout_options[];
extern argus_option_t switch_options[];

// External option declarations for nested commands
extern argus_option_t remote_options[];
extern argus_option_t config_options[];
extern argus_option_t stash_options[];

// Command handlers
int init_handler(argus_t *argus, void *data);
int add_handler(argus_t *argus, void *data);
int commit_handler(argus_t *argus, void *data);
int status_handler(argus_t *argus, void *data);
int log_handler(argus_t *argus, void *data);
int branch_handler(argus_t *argus, void *data);
int pull_handler(argus_t *argus, void *data);
int fetch_handler(argus_t *argus, void *data);
int push_handler(argus_t *argus, void *data);
int checkout_handler(argus_t *argus, void *data);
int switch_handler(argus_t *argus, void *data);

// Nested command handlers
int remote_handler(argus_t *argus, void *data);
int config_handler(argus_t *argus, void *data);
int stash_handler(argus_t *argus, void *data);

#endif // GIT_H