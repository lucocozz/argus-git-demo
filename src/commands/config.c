#include <argus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands/git.h"
#include "colors.h"
#include "git_types.h"


extern argus_option_t config_get_options[];
extern argus_option_t config_set_options[];
extern argus_option_t config_unset_options[];
extern argus_option_t config_list_options[];
extern argus_option_t config_edit_options[];

int config_get_handler(argus_t *argus, void *data);
int config_set_handler(argus_t *argus, void *data);
int config_unset_handler(argus_t *argus, void *data);
int config_list_handler(argus_t *argus, void *data);
int config_edit_handler(argus_t *argus, void *data);



ARGUS_OPTIONS(
    config_options,
    HELP_OPTION(),
    
    // Config file location options
    GROUP_START("Config file location"),
        OPTION_FLAG(0, "global",   HELP("use global config file")),
        OPTION_FLAG(0, "system",   HELP("use system config file")),
        OPTION_FLAG(0, "local",    HELP("use repository config file")),
        OPTION_FLAG(0, "worktree", HELP("use per-worktree config file")),
        OPTION_STRING('f', "file", HELP("use given config file"), HINT("file")),
    GROUP_END(),

    // Action options  
    GROUP_START("Action"),
        // Get operations
        OPTION_FLAG(0, "get",           HELP("get value: name [value-pattern]")),
        OPTION_FLAG(0, "get-all",       HELP("get all values: key [value-pattern]")),
        OPTION_FLAG(0, "get-regexp",    HELP("get values for regexp: name-regex [value-pattern]")),
        OPTION_FLAG(0, "get-urlmatch",  HELP("get value specific for the URL: section[.var] URL")),
        
        // Set operations
        OPTION_FLAG(0, "replace-all",   HELP("replace all matching variables: name value [value-pattern]")),
        OPTION_FLAG(0, "add",           HELP("add a new variable: name value")),
        
        // Unset operations
        OPTION_FLAG(0, "unset",         HELP("remove a variable: name [value-pattern]")),
        OPTION_FLAG(0, "unset-all",     HELP("remove all matches: name [value-pattern]")),
        
        // Section operations
        OPTION_FLAG(0, "rename-section", HELP("rename section: old-name new-name")),
        OPTION_FLAG(0, "remove-section", HELP("remove a section: name")),
        
        // Display operations
        OPTION_FLAG('l', "list",        HELP("list all")),
        OPTION_FLAG('e', "edit",        HELP("open an editor")),
        
        // Color operations
        OPTION_FLAG(0, "get-color",     HELP("find the color configured: slot [default]")),
        OPTION_FLAG(0, "get-colorbool", HELP("find the color setting: slot [stdout-is-tty]")),
    GROUP_END(),

    // Type options
    GROUP_START("Type"),
        OPTION_FLAG(0, "bool", HELP("value is \"true\" or \"false\"")),
    GROUP_END(),

    // Other options
    GROUP_START("Other"),
        OPTION_FLAG('z', "null",        HELP("terminate values with NUL byte")),
        OPTION_FLAG(0, "name-only",     HELP("show variable names only")),
        OPTION_FLAG(0, "show-origin",   HELP("show origin of config (file, standard input, blob, command line)")),
        OPTION_FLAG(0, "show-scope",    HELP("show scope of config (worktree, local, global, system, command)")),
        OPTION_STRING(0, "default",     HELP("with --get, use default value when missing entry"), HINT("value")),
    GROUP_END(),
)


int config_handler(argus_t *argus, void *data)
{
    (void)data;
    
    // ========================================================================
    // Parse scope options
    // ========================================================================
    
    bool global   = argus_get(argus, "global").as_bool;
    bool system   = argus_get(argus, "system").as_bool;
    bool local    = argus_get(argus, "local").as_bool;
    bool worktree = argus_get(argus, "worktree").as_bool;
    
    // Determine config scope
    const char *scope = "local";
    if (global)        scope = "global";
    else if (system)   scope = "system";
    else if (local)    scope = "local";
    else if (worktree) scope = "worktree";
    
    // Check for custom config file
    const char *config_file = NULL;
    if (argus_is_set(argus, "file")) {
        config_file = argus_get(argus, "file").as_string;
        scope = config_file;
    }
    
    // ========================================================================
    // Handle list action
    // ========================================================================
    
    if (argus_get(argus, "list").as_bool) {
        bool show_origin     = argus_get(argus, "show-origin").as_bool;
        bool show_scope      = argus_get(argus, "show-scope").as_bool;
        bool name_only       = argus_get(argus, "name-only").as_bool;
        bool null_terminate  = argus_get(argus, "null").as_bool;
        
        const git_config_entry_t configs[] = {
            {"user.name", "John Doe", scope},
            {"user.email", "john.doe@example.com", scope}, 
            {"core.editor", "vim", scope},
            {"core.autocrlf", "input", scope},
            {"remote.origin.url", "https://github.com/user/repo.git", scope},
            {"remote.origin.fetch", "+refs/heads/*:refs/remotes/origin/*", scope},
            {"branch.main.remote", "origin", scope},
            {"branch.main.merge", "refs/heads/main", scope}
        };
        int config_count = 8;
        
        for (int i = 0; i < config_count; i++) {
            const git_config_entry_t *config = &configs[i];
            
            if (show_scope) printf("%s\t", config->scope);
            if (show_origin) printf("file:~/.gitconfig\t");
            
            if (name_only) {
                printf("%s", config->key);
            } else {
                printf("%s=%s", config->key, config->value);
            }
            
            printf(null_terminate ? "" : "\n");
        }
        return 0;
    }
    
    // ========================================================================
    // Handle edit action
    // ========================================================================
    
    if (argus_get(argus, "edit").as_bool) {
        printf("Opening editor for %s config file...\n", scope);
        
        // Determine which config file to edit
        if (config_file)
            printf("Editing file: %s\n", config_file);
        else if (global)
            printf("Editing file: ~/.gitconfig\n");
        else if (system)
            printf("Editing file: /etc/gitconfig\n");
        else
            printf("Editing file: .git/config\n");
        return 0;
    }
    
    // ========================================================================
    // Handle get actions
    // ========================================================================
    
    if (argus_get(argus, "get").as_bool || 
        argus_get(argus, "get-all").as_bool ||
        argus_get(argus, "get-regexp").as_bool ||
        argus_get(argus, "get-urlmatch").as_bool) {
        
        // Validate required key parameter
        if (!argus_is_set(argus, "name")) {
            printf("error: key required for get operation\n");
            return 1;
        }
        
        // Parse get operation parameters
        const char *key        = argus_get(argus, "name").as_string;
        bool        get_all    = argus_get(argus, "get-all").as_bool;
        bool        get_regexp = argus_get(argus, "get-regexp").as_bool;
        bool        get_urlmatch = argus_get(argus, "get-urlmatch").as_bool;
        bool        as_bool    = argus_get(argus, "bool").as_bool;
        
        // Simulate getting config values based on key
        if (strcmp(key, "user.name") == 0)
            printf(as_bool ? "true\n" : "John Doe\n");
        else if (strcmp(key, "user.email") == 0)
            printf("john.doe@example.com\n");
        else if (strcmp(key, "core.editor") == 0)
            printf("vim\n");
        else if (get_regexp || get_urlmatch || strstr(key, "*") || strstr(key, "remote")) {
            printf("remote.origin.url=https://github.com/user/repo.git\n");
            printf("remote.origin.fetch=+refs/heads/*:refs/remotes/origin/*\n");
            if (get_all)
                printf("remote.upstream.url=https://github.com/upstream/repo.git\n");
        } 
        else {
            // Use default value if available, otherwise error
            if (argus_is_set(argus, "default"))
                printf("%s\n", argus_get(argus, "default").as_string);
            else {
                printf("error: key '%s' not found\n", key);
                return 1;
            }
        }
        return 0;
    }
    
    // ========================================================================
    // Handle set operation (when both name and value are provided)
    // ========================================================================
    
    if (argus_is_set(argus, "name") && argus_is_set(argus, "value")) {
        const char *key        = argus_get(argus, "name").as_string;
        const char *value      = argus_get(argus, "value").as_string;
        bool        add        = argus_get(argus, "add").as_bool;
        bool        replace_all = argus_get(argus, "replace-all").as_bool;
        
        // Display appropriate action message
        if (add)
            printf("Adding %s config: %s = %s\n", scope, key, value);
        else if (replace_all)
            printf("Replacing all %s config: %s = %s\n", scope, key, value);
        else
            printf("Setting %s config: %s = %s\n", scope, key, value);
        return 0;
    }
    
    // ========================================================================
    // Handle unset operations
    // ========================================================================
    
    if (argus_get(argus, "unset").as_bool || argus_get(argus, "unset-all").as_bool) {
        // Validate required key parameter
        if (!argus_is_set(argus, "name")) {
            printf("error: key required for unset operation\n");
            return 1;
        }
        
        const char *key       = argus_get(argus, "name").as_string;
        bool        unset_all = argus_get(argus, "unset-all").as_bool;
        
        // Display appropriate unset message
        if (unset_all)
            printf("Unsetting all %s config entries for: %s\n", scope, key);
        else
            printf("Unsetting %s config: %s\n", scope, key);
        return 0;
    }
    
    // ========================================================================
    // Handle section operations
    // ========================================================================
    
    if (argus_get(argus, "rename-section").as_bool) {
        // Validate required parameters
        if (!argus_is_set(argus, "name") || !argus_is_set(argus, "value")) {
            printf("error: old and new section names required\n");
            return 1;
        }
        
        const char *old_section = argus_get(argus, "name").as_string;
        const char *new_section = argus_get(argus, "value").as_string;
        
        printf("Renaming section '%s' to '%s' in %s config\n", old_section, new_section, scope);
        return 0;
    }
    
    if (argus_get(argus, "remove-section").as_bool) {
        // Validate required parameter
        if (!argus_is_set(argus, "name")) {
            printf("error: section name required\n");
            return 1;
        }
        
        const char *section = argus_get(argus, "name").as_string;
        printf("Removing section '%s' from %s config\n", section, scope);
        return 0;
    }
    
    // ========================================================================
    // Handle color operations
    // ========================================================================
    
    if (argus_get(argus, "get-color").as_bool) {
        // Validate required parameter
        if (!argus_is_set(argus, "name")) {
            printf("error: color name required\n");
            return 1;
        }
        
        const char *color_name = argus_get(argus, "name").as_string;
        printf("Color for '%s': \\033[32m (green)\n", color_name);
        return 0;
    }
    
    if (argus_get(argus, "get-colorbool").as_bool) {
        // Validate required parameter
        if (!argus_is_set(argus, "name")) {
            printf("error: color setting name required\n");
            return 1;
        }
        
        const char *setting = argus_get(argus, "name").as_string;
        printf("Color setting for '%s': true\n", setting);
        return 0;
    }

    return 0;
}