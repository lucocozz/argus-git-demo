#include <argus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands/git.h"

// Main git options with all commands
ARGUS_OPTIONS(
    main_options,
    HELP_OPTION(),
    VERSION_OPTION(),
    
    // Global options
    OPTION_FLAG(
        'v', "verbose",
        HELP("Be more verbose")),
    OPTION_FLAG(
        'q', "quiet",
        HELP("Be more quiet"), 
        CONFLICT("verbose")),
    OPTION_STRING(
        'C', NULL,
        HELP("Run as if git was started in <path> instead of the current working directory"),
        HINT("path")),
    OPTION_MAP_STRING(
        'c', NULL,
        HELP("Pass a configuration parameter to the command"),
        HINT("name=value")),
    
    // Git commands
    SUBCOMMAND(
        "init", init_options,
        HELP("Create an empty Git repository or reinitialize an existing one"),
        ACTION(init_handler)),
    SUBCOMMAND(
        "add", add_options,
        HELP("Add file contents to the index"),
        ACTION(add_handler)),
    SUBCOMMAND(
        "commit", commit_options,
        HELP("Record changes to the repository"),
        ACTION(commit_handler)),
    SUBCOMMAND(
        "status", status_options,
        HELP("Show the working tree status"),
        ACTION(status_handler)),
    SUBCOMMAND(
        "log", log_options,
        HELP("Show commit logs"),
        ACTION(log_handler)),
    SUBCOMMAND(
        "remote", remote_options,
        HELP("Manage set of tracked repositories")),
    SUBCOMMAND(
        "config", config_options,
        HELP("Get and set repository or global options"),
        ACTION(config_handler)),
    SUBCOMMAND(
        "stash", stash_options,
        HELP("Stash the changes in a dirty working directory away")),
    SUBCOMMAND(
        "branch", branch_options,
        HELP("List, create, or delete branches"),
        ACTION(branch_handler)),
)


int main(int argc, char **argv)
{
    // Initialize Git CLI with Argus
    argus_t argus = argus_init(main_options, "git", "2.45.2");
    argus.description = "Git - the stupid content tracker";
    
    int status = argus_parse(&argus, argc, argv);
    if (status != ARGUS_SUCCESS)
        return status;

    if (!argus_has_command(&argus)) {
        argus_print_help(&argus);
        argus_free(&argus);
        return ARGUS_ERROR_NO_COMMAND;
    }

    // Execute Git command
    if (argus_get(&argus, "verbose").as_bool) {
        printf("Git configuration:\n");
        printf("  Verbose mode: enabled\n");
        printf("\n");
    }
    
    status = argus_exec(&argus, NULL);

    // Cleanup and exit
    argus_free(&argus);    
    return status;
}
