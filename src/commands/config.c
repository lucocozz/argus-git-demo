#include <argus.h>
#include <argus/regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands/git.h"
#include "colors.h"
#include "git_types.h"
#include "mock_data.h"

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
    
    GROUP_START("Config file location"),
        OPTION_FLAG(0, "global", HELP("use global config file")),
        OPTION_FLAG(0, "system", HELP("use system config file")),
        OPTION_FLAG(0, "local", HELP("use repository config file")),
        OPTION_FLAG(0, "worktree", HELP("use per-worktree config file")),
        OPTION_STRING('f', "file", HELP("use given config file")),
    GROUP_END(),

    GROUP_START("Action"),
        OPTION_FLAG(0, "get", HELP("get value: name [value-pattern]")),
        OPTION_FLAG(0, "get-all", HELP("get all values: key [value-pattern]")),
        OPTION_FLAG(0, "get-regexp", HELP("get values for regexp: name-regex [value-pattern]")),
        OPTION_FLAG(0, "get-urlmatch", HELP("get value specific for the URL: section[.var] URL")),
        
        OPTION_FLAG(0, "replace-all", HELP("replace all matching variables: name value [value-pattern]")),
        OPTION_FLAG(0, "add", HELP("add a new variable: name value")),
        
        OPTION_FLAG(0, "unset", HELP("remove a variable: name [value-pattern]")),
        OPTION_FLAG(0, "unset-all", HELP("remove all matches: name [value-pattern]")),
        
        OPTION_FLAG(0, "rename-section", HELP("rename section: old-name new-name")),
        OPTION_FLAG(0, "remove-section", HELP("remove a section: name")),
        
        OPTION_FLAG('l', "list", HELP("list all")),
        OPTION_FLAG('e', "edit", HELP("open an editor")),
        
        OPTION_FLAG(0, "get-color", HELP("find the color configured: slot [default]")),
        OPTION_FLAG(0, "get-colorbool", HELP("find the color setting: slot [stdout-is-tty]")),
    GROUP_END(),

    GROUP_START("Type"),
        OPTION_FLAG(0, "bool", HELP("value is \"true\" or \"false\"")),
    GROUP_END(),

    GROUP_START("Other"),
        OPTION_FLAG('z', "null", HELP("terminate values with NUL byte")),
        OPTION_FLAG(0, "name-only", HELP("show variable names only")),
        OPTION_FLAG(0, "show-origin", HELP("show origin of config (file, standard input, blob, command line)")),
        OPTION_FLAG(0, "show-scope", HELP("show scope of config (worktree, local, global, system, command)")),
        OPTION_STRING(0, "default", HELP("with --get, use default value when missing entry")),
    GROUP_END(),
)

static const char* determine_config_scope(argus_t *argus)
{
    bool global = argus_get(argus, "global").as_bool;
    bool system = argus_get(argus, "system").as_bool;
    bool worktree = argus_get(argus, "worktree").as_bool;
    const char *config_file = argus_get(argus, "file").as_string;
    
    if (config_file) return config_file;
    if (global) return "global";
    if (system) return "system";
    if (worktree) return "worktree";
    return "local";
}

static int handle_config_list(argus_t *argus, const char *scope)
{
    bool show_origin = argus_get(argus, "show-origin").as_bool;
    bool show_scope = argus_get(argus, "show-scope").as_bool;
    bool name_only = argus_get(argus, "name-only").as_bool;
    bool null_terminate = argus_get(argus, "null").as_bool;
    
    int config_count;
    const git_config_entry_t *configs = get_mock_config_entries(scope, &config_count);
    
    for (int i = 0; i < config_count; i++) {
        const git_config_entry_t *config = &configs[i];
        
        if (show_scope) printf("%s\t", config->scope);
        if (show_origin) printf("file:~/.gitconfig\t");
        if (name_only) printf("%s", config->key);
        else printf("%s=%s", config->key, config->value);
        
        printf(null_terminate ? "" : "\n");
    }
    return 0;
}

static int handle_config_edit(argus_t *argus, const char *scope)
{
    bool global = argus_get(argus, "global").as_bool;
    bool system = argus_get(argus, "system").as_bool;
    const char *config_file = argus_get(argus, "file").as_string;
    
    printf("Opening editor for %s config file...\n", scope);
    
    if (config_file) printf("Editing file: %s\n", config_file);
    else if (global) printf("Editing file: ~/.gitconfig\n");
    else if (system) printf("Editing file: /etc/gitconfig\n");
    else printf("Editing file: .git/config\n");
    return 0;
}

static int handle_config_get_operations(argus_t *argus)
{
    const char *key = argus_get(argus, "name").as_string;
    
    if (!key) {
        printf("error: key required for get operation\n");
        return 1;
    }
    
    bool get_all = argus_get(argus, "get-all").as_bool;
    bool get_regexp = argus_get(argus, "get-regexp").as_bool;
    bool get_urlmatch = argus_get(argus, "get-urlmatch").as_bool;
    bool as_bool = argus_get(argus, "bool").as_bool;
    
    if (strcmp(key, "user.name") == 0) printf(as_bool ? "true\n" : "John Doe\n");
    else if (strcmp(key, "user.email") == 0) printf("john.doe@example.com\n");
    else if (strcmp(key, "core.editor") == 0) printf("vim\n");
    else if (get_regexp || get_urlmatch || strstr(key, "*") || strstr(key, "remote")) {
        printf("remote.origin.url=https://github.com/user/repo.git\n");
        printf("remote.origin.fetch=+refs/heads/*:refs/remotes/origin/*\n");
        if (get_all) printf("remote.upstream.url=https://github.com/upstream/repo.git\n");
    } 
    else {
        const char *default_value = argus_get(argus, "default").as_string;
        if (default_value) printf("%s\n", default_value);
        else {
            printf("error: key '%s' not found\n", key);
            return 1;
        }
    }
    return 0;
}

static int handle_config_set_operations(argus_t *argus, const char *scope)
{
    const char *key_set = argus_get(argus, "name").as_string;
    const char *value_set = argus_get(argus, "value").as_string;
    
    if (!key_set || !value_set) return 1;
    
    bool add = argus_get(argus, "add").as_bool;
    bool replace_all = argus_get(argus, "replace-all").as_bool;
    
    if (add) printf("Adding %s config: %s = %s\n", scope, key_set, value_set);
    else if (replace_all) printf("Replacing all %s config: %s = %s\n", scope, key_set, value_set);
    else printf("Setting %s config: %s = %s\n", scope, key_set, value_set);
    return 0;
}

static int handle_config_unset_operations(argus_t *argus, const char *scope)
{
    const char *key = argus_get(argus, "name").as_string;
    
    if (!key) {
        printf("error: key required for unset operation\n");
        return 1;
    }
    
    bool unset_all = argus_get(argus, "unset-all").as_bool;
    
    if (unset_all) printf("Unsetting all %s config entries for: %s\n", scope, key);
    else printf("Unsetting %s config: %s\n", scope, key);
    return 0;
}

static int handle_config_section_operations(argus_t *argus, const char *scope)
{
    if (argus_get(argus, "rename-section").as_bool) {
        const char *old_section = argus_get(argus, "name").as_string;
        const char *new_section = argus_get(argus, "value").as_string;
        
        if (!old_section || !new_section) {
            printf("error: old and new section names required\n");
            return 1;
        }
        
        printf("Renaming section '%s' to '%s' in %s config\n", old_section, new_section, scope);
        return 0;
    }
    
    if (argus_get(argus, "remove-section").as_bool) {
        const char *section = argus_get(argus, "name").as_string;
        
        if (!section) {
            printf("error: section name required\n");
            return 1;
        }
        
        printf("Removing section '%s' from %s config\n", section, scope);
        return 0;
    }
    
    return 1;
}

static int handle_config_color_operations(argus_t *argus)
{
    if (argus_get(argus, "get-color").as_bool) {
        const char *color_name = argus_get(argus, "name").as_string;
        
        if (!color_name) {
            printf("error: color name required\n");
            return 1;
        }
        
        printf("Color for '%s': \\033[32m (green)\n", color_name);
        return 0;
    }
    
    if (argus_get(argus, "get-colorbool").as_bool) {
        const char *setting = argus_get(argus, "name").as_string;
        
        if (!setting) {
            printf("error: color setting name required\n");
            return 1;
        }
        
        printf("Color setting for '%s': true\n", setting);
        return 0;
    }
    
    return 1;
}

static bool has_get_operation(argus_t *argus)
{
    return argus_get(argus, "get").as_bool || 
           argus_get(argus, "get-all").as_bool ||
           argus_get(argus, "get-regexp").as_bool ||
           argus_get(argus, "get-urlmatch").as_bool;
}

static bool has_set_operation(argus_t *argus) {
    return argus_get(argus, "name").as_string && argus_get(argus, "value").as_string;
}

static bool has_unset_operation(argus_t *argus) {
    return argus_get(argus, "unset").as_bool || argus_get(argus, "unset-all").as_bool;
}

static bool has_section_operation(argus_t *argus) {
    return argus_get(argus, "rename-section").as_bool || argus_get(argus, "remove-section").as_bool;
}

static bool has_color_operation(argus_t *argus) {
    return argus_get(argus, "get-color").as_bool || argus_get(argus, "get-colorbool").as_bool;
}

int config_handler(argus_t *argus, void *data)
{
    (void)data;
    
    const char *scope = determine_config_scope(argus);
    
    if (argus_get(argus, "list").as_bool) return handle_config_list(argus, scope);
    if (argus_get(argus, "edit").as_bool) return handle_config_edit(argus, scope);
    if (has_get_operation(argus)) return handle_config_get_operations(argus);
    if (has_set_operation(argus)) return handle_config_set_operations(argus, scope);
    if (has_unset_operation(argus)) return handle_config_unset_operations(argus, scope);
    if (has_section_operation(argus)) return handle_config_section_operations(argus, scope);
    if (has_color_operation(argus)) return handle_config_color_operations(argus);

    return 0;
}