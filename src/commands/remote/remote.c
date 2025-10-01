#include <argus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands/git.h"
#include "commands/remote.h"
#include "colors.h"
#include "git_types.h"
#include "mock_data.h"

ARGUS_OPTIONS(
    remote_options,
    HELP_OPTION(),
    OPTION_FLAG('v', "verbose", HELP("Be a little more verbose and show remote url after name")),
    
    SUBCOMMAND("add", remote_add_options,
        HELP("Add a remote named <name> for the repository at <url>"),
        ACTION(remote_add_handler)),
    SUBCOMMAND("remove", remote_remove_options,
        HELP("Remove the remote named <name>"),
        ACTION(remote_remove_handler)),
    SUBCOMMAND("rm", remote_remove_options,
        HELP("Remove the remote named <name>"),
        ACTION(remote_remove_handler)),
    SUBCOMMAND("show", remote_show_options,
        HELP("Show information about the remote <name>"),
        ACTION(remote_show_handler)),
    SUBCOMMAND("rename", remote_rename_options,
        HELP("Rename the remote named <old> to <new>"),
        ACTION(remote_rename_handler)),
    SUBCOMMAND("set-url", remote_set_url_options,
        HELP("Change the URL for the remote named <name>"),
        ACTION(remote_set_url_handler)),
    SUBCOMMAND("get-url", remote_get_url_options,
        HELP("Retrieve the URLs for the remote named <name>"),
        ACTION(remote_get_url_handler)),
    SUBCOMMAND("prune", remote_prune_options,
        HELP("Delete stale references associated with <name>"),
        ACTION(remote_prune_handler)),
)

int remote_handler(argus_t *argus, void *data)
{
    (void)data;
    
    if (argus_has_command(argus))
        return argus_exec(argus, NULL);
    
    bool verbose = argus_get(argus, "verbose").as_bool;
    
    int remote_count;
    const git_remote_t *remotes = get_mock_remotes(&remote_count);
    
    for (int i = 0; i < remote_count; i++) {
        const git_remote_t *remote = &remotes[i];
        
        if (verbose) {
            printf(COLOR_CYAN("%s") "\t" COLOR_BLUE("%s") " (fetch)\n", remote->name, remote->url);
            printf(COLOR_CYAN("%s") "\t" COLOR_BLUE("%s") " (push)\n", remote->name, remote->push_url);
        } else {
            printf(COLOR_CYAN("%s") "\n", remote->name);
        }
    }
    
    return 0;
}