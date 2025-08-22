#include "validation_utils.h"
#include <string.h>
#include <stdio.h>

bool is_valid_remote_name(const char *name)
{
    if (!name || strlen(name) == 0) {
        return false;
    }
    
    for (const char *p = name; *p; p++) {
        if (!isalnum(*p) && *p != '-' && *p != '_' && *p != '.') {
            return false;
        }
    }
    
    return true;
}

bool is_valid_stash_ref(const char *ref)
{
    if (!ref || strlen(ref) == 0) {
        return false;
    }
    
    if (strncmp(ref, "stash@{", 7) == 0 && ref[strlen(ref)-1] == '}') {
        return true;
    }
    
    return false;
}

bool is_valid_url(const char *url)
{
    if (!url || strlen(url) == 0) {
        return false;
    }
    
    return (strncmp(url, "http://", 7) == 0 || 
            strncmp(url, "https://", 8) == 0 ||
            strncmp(url, "git@", 4) == 0 ||
            strncmp(url, "ssh://", 6) == 0);
}

bool is_valid_branch_name(const char *name)
{
    if (!name || strlen(name) == 0) {
        return false;
    }
    
    if (name[0] == '-' || name[0] == '.' || 
        strstr(name, "..") || strstr(name, "//") ||
        name[strlen(name)-1] == '.' || name[strlen(name)-1] == '/') {
        return false;
    }
    
    return true;
}