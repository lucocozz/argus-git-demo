#ifndef STASH_H
#define STASH_H

#include <argus/types.h>

// External option declarations for stash subcommands
extern argus_option_t stash_push_options[];
extern argus_option_t stash_pop_options[];
extern argus_option_t stash_apply_options[];
extern argus_option_t stash_drop_options[];
extern argus_option_t stash_list_options[];
extern argus_option_t stash_show_options[];
extern argus_option_t stash_clear_options[];
extern argus_option_t stash_branch_options[];

// Subcommand handlers
int stash_push_handler(argus_t *argus, void *data);
int stash_pop_handler(argus_t *argus, void *data);
int stash_apply_handler(argus_t *argus, void *data);
int stash_drop_handler(argus_t *argus, void *data);
int stash_list_handler(argus_t *argus, void *data);
int stash_show_handler(argus_t *argus, void *data);
int stash_clear_handler(argus_t *argus, void *data);
int stash_branch_handler(argus_t *argus, void *data);

#endif // STASH_H