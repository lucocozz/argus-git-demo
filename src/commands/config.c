#include <argus.h>
#include <argus/regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands/git.h"
#include "colors.h"
#include "git_types.h"
#include "mock_data.h"



ARGUS_OPTIONS(
    config_options,
    HELP_OPTION(),
    
    GROUP_START("Config scope"),
        OPTION_FLAG(0, "global", HELP("Use global config file")),
        OPTION_FLAG(0, "system", HELP("Use system config file")),
        OPTION_FLAG(0, "local", HELP("Use repository config file")),
    GROUP_END(),

    GROUP_START("Actions"),
        OPTION_FLAG(0, "get", HELP("Get config value")),
        OPTION_FLAG(0, "get-all", HELP("Get all matching values")),
        OPTION_FLAG(0, "add", HELP("Add new config entry")),
        OPTION_FLAG(0, "unset", HELP("Remove config entry")),
        OPTION_FLAG('l', "list", HELP("List all config")),
        OPTION_FLAG('e', "edit", HELP("Open config in editor")),
    GROUP_END(),

    GROUP_START("Display options"),
        OPTION_FLAG('z', "null", HELP("NUL-terminate entries")),
        OPTION_FLAG(0, "name-only", HELP("Show names only")),
        OPTION_FLAG(0, "show-origin", HELP("Show config file origin")),
        OPTION_FLAG(0, "show-scope", HELP("Show config scope")),
    GROUP_END(),

    POSITIONAL_STRING("name", HELP("Config key name"), FLAGS(FLAG_OPTIONAL)),
    POSITIONAL_STRING("value", HELP("Config value"), FLAGS(FLAG_OPTIONAL)),
)

static const char* determine_config_scope(argus_t *argus)
{
    bool global = argus_get(argus, "global").as_bool;
    bool system = argus_get(argus, "system").as_bool;
    
    if (global) return "global";
    if (system) return "system";
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
    bool get_all = argus_get(argus, "get-all").as_bool;
    
    if (!key) {
        printf(COLOR_RED("error: ") "key required for get operation\n");
        return 1;
    }
    
    const char *scope = determine_config_scope(argus);
    int config_count;
    const git_config_entry_t *configs = get_mock_config_entries(scope, &config_count);
    
    bool found = false;
    for (int i = 0; i < config_count; i++) {
        if (strcmp(configs[i].key, key) == 0) {
            printf("%s\n", configs[i].value);
            found = true;
            if (!get_all) break;
        }
    }
    
    if (!found) {
        printf(COLOR_RED("error: ") "key '%s' not found\n", key);
        return 1;
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



static int handle_list_operation(argus_t *argus)
{
    bool list = argus_get(argus, "list").as_bool;
    
    if (list) {
        const char *scope = determine_config_scope(argus);
        return handle_config_list(argus, scope);
    }
    return -1;
}

static int handle_edit_operation(argus_t *argus)
{
    bool edit = argus_get(argus, "edit").as_bool;
    
    if (edit) {
        const char *scope = determine_config_scope(argus);
        return handle_config_edit(argus, scope);
    }
    return -1;
}

static int handle_get_operation(argus_t *argus)
{
    bool get = argus_get(argus, "get").as_bool;
    bool get_all = argus_get(argus, "get-all").as_bool;
    
    if (get || get_all) {
        return handle_config_get_operations(argus);
    }
    return -1;
}

static int handle_set_operation(argus_t *argus)
{
    bool add = argus_get(argus, "add").as_bool;
    const char *name = argus_get(argus, "name").as_string;
    const char *value = argus_get(argus, "value").as_string;
    
    if (add || (name && value)) {
        const char *scope = determine_config_scope(argus);
        return handle_config_set_operations(argus, scope);
    }
    return -1;
}

static int handle_unset_operation(argus_t *argus)
{
    bool unset = argus_get(argus, "unset").as_bool;
    
    if (unset) {
        const char *scope = determine_config_scope(argus);
        return handle_config_unset_operations(argus, scope);
    }
    return -1;
}

int config_handler(argus_t *argus, void *data)
{
    (void)data;
    
    int result;
    
    if ((result = handle_list_operation(argus)) != -1)
        return result;
    
    if ((result = handle_edit_operation(argus)) != -1)
        return result;
    
    if ((result = handle_get_operation(argus)) != -1)
        return result;
    
    if ((result = handle_set_operation(argus)) != -1)
        return result;
    
    if ((result = handle_unset_operation(argus)) != -1)
        return result;
    
    printf(COLOR_YELLOW("hint: ") "use 'git config --list' to see all config\n");
    return 0;
}