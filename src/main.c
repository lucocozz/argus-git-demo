#include <argus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands/git.h"

ARGUS_OPTIONS(
    main_options,
    HELP_OPTION(),
    VERSION_OPTION(),

    OPTION_FLAG(
        'v', "verbose",
        HELP("Be more verbose"),
        ENV_VAR("GIT_VERBOSE")),
    OPTION_FLAG(
        'q', "quiet",
        HELP("Be more quiet"),
        ENV_VAR("GIT_QUIET"),
        CONFLICT("verbose")),
    OPTION_STRING(
        'C', NULL,
        HELP("Run as if git was started in <path> instead of the current "
             "working directory"),
        ENV_VAR("GIT_DIR"),
        HINT("path")),
    OPTION_MAP_STRING(
        'c', NULL,
        HELP("Pass a configuration parameter to the command")),

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
    SUBCOMMAND(
        "pull", pull_options, 
        HELP("Fetch from and integrate with another repository or a local branch"), 
        ACTION(pull_handler)),
    SUBCOMMAND(
        "fetch", fetch_options, 
        HELP("Download objects and refs from another repository"), 
        ACTION(fetch_handler)),
    SUBCOMMAND(
        "push", push_options, 
        HELP("Update remote refs along with associated objects"), 
        ACTION(push_handler)),
    SUBCOMMAND(
        "checkout", checkout_options, 
        HELP("Switch branches or restore working tree files"), 
        ACTION(checkout_handler)),
    SUBCOMMAND(
        "switch", switch_options, 
        HELP("Switch branches"), 
        ACTION(switch_handler)),
)


int main(int argc, char **argv)
{
    argus_t argus = argus_init(main_options, "git", "2.45.2");
    argus.description = "Git - the stupid content tracker";
    argus.env_prefix = "GIT";
    
    int status = argus_parse(&argus, argc, argv);
    if (status != ARGUS_SUCCESS)
        return status;

    if (!argus_has_command(&argus)) {
        argus_print_help(&argus);
        argus_free(&argus);
        return ARGUS_ERROR_NO_COMMAND;
    }

    if (argus_get(&argus, "verbose").as_bool) {
        printf("Git configuration:\n");
        printf("  Verbose mode: enabled\n");
        printf("\n");
    }
    
    status = argus_exec(&argus, NULL);

    argus_free(&argus);    
    return status;
}
