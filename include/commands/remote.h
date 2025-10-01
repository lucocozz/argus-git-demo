#ifndef REMOTE_H
#define REMOTE_H

#include <argus/types.h>

// External option declarations for remote subcommands
extern argus_option_t remote_add_options[];
extern argus_option_t remote_remove_options[];
extern argus_option_t remote_show_options[];
extern argus_option_t remote_rename_options[];
extern argus_option_t remote_set_url_options[];
extern argus_option_t remote_get_url_options[];
extern argus_option_t remote_prune_options[];

// Subcommand handlers
int remote_add_handler(argus_t *argus, void *data);
int remote_remove_handler(argus_t *argus, void *data);
int remote_show_handler(argus_t *argus, void *data);
int remote_rename_handler(argus_t *argus, void *data);
int remote_set_url_handler(argus_t *argus, void *data);
int remote_get_url_handler(argus_t *argus, void *data);
int remote_prune_handler(argus_t *argus, void *data);

#endif // REMOTE_H