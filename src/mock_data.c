#include "mock_data.h"
#include <string.h>

const git_remote_t* get_mock_remotes(int *count)
{
    static const git_remote_t remotes[] = {
        {"origin", "https://github.com/user/repo.git", "https://github.com/user/repo.git", "both"},
        {"upstream", "https://github.com/upstream/repo.git", "https://github.com/upstream/repo.git", "both"}
    };
    *count = 2;
    return remotes;
}

const git_stash_entry_t* get_mock_stashes(int *count)
{
    static const git_stash_entry_t stashes[] = {
        {0, "WIP on main: abc1234 Add new feature for user authentication", "main", "2024-01-15 10:30:00"},
        {1, "On feature-branch: def5678 Fix bug in payment processing", "feature-branch", "2024-01-14 15:45:00"},
        {2, "WIP on main: ghi9012 Update documentation for API endpoints", "main", "2024-01-13 09:15:00"}
    };
    *count = 3;
    return stashes;
}

const git_commit_t* get_mock_commits(int *count)
{
    static const git_commit_t commits[] = {
        {"abc1234", "Add new feature for user authentication", "John Doe", "Mon Jan 15 10:30:45 2024", "john.doe@example.com"},
        {"def5678", "Fix bug in payment processing", "Jane Smith", "Sun Jan 14 15:45:20 2024", "jane.smith@example.com"},
        {"ghi9012", "Update documentation for API endpoints", "Bob Wilson", "Sat Jan 13 09:15:30 2024", "bob.wilson@example.com"},
        {"jkl3456", "Refactor database connection logic", "Alice Brown", "Fri Jan 12 14:22:10 2024", "alice.brown@example.com"},
        {"mno7890", "Initial commit", "John Doe", "Thu Jan 11 16:00:00 2024", "john.doe@example.com"}
    };
    *count = 5;
    return commits;
}

const git_branch_t* get_mock_branches(int *count)
{
    static const git_branch_t branches[] = {
        {"main", "abc1234", "Add new feature for user authentication", "current"},
        {"develop", "def5678", "Fix bug in payment processing", "local"},
        {"feature", "ghi9012", "Update documentation for API endpoints", "local"},
        {"hotfix", "jkl3456", "Emergency security patch", "local"}
    };
    *count = 4;
    return branches;
}

const git_branch_t* get_mock_remote_branches(int *count)
{
    static const git_branch_t remote_branches[] = {
        {"origin/main", "abc1234", "Add new feature for user authentication", "remote"},
        {"origin/develop", "jkl3456", "Refactor database connection logic", "remote"},
        {"upstream/main", "abc1234", "Add new feature for user authentication", "remote"}
    };
    *count = 3;
    return remote_branches;
}

const git_file_status_t* get_mock_file_status(int *count)
{
    static const git_file_status_t files[] = {
        {"new-file.txt", "new", true, false},
        {"modified-file.txt", "modified", false, true},
        {"untracked-file.txt", "untracked", false, false},
        {"ignored-file.log", "ignored", false, false}
    };
    *count = 4;
    return files;
}

const git_config_entry_t* get_mock_config_entries(const char *scope, int *count)
{
    static git_config_entry_t configs[8];
    
    const char *keys[] = {
        "user.name", "user.email", "core.editor", "core.autocrlf",
        "remote.origin.url", "remote.origin.fetch", "branch.main.remote", "branch.main.merge"
    };
    const char *values[] = {
        "John Doe", "john.doe@example.com", "vim", "input",
        "https://github.com/user/repo.git", "+refs/heads/*:refs/remotes/origin/*", 
        "origin", "refs/heads/main"
    };
    
    for (int i = 0; i < 8; i++) {
        configs[i].key = keys[i];
        configs[i].value = values[i];
        configs[i].scope = scope;
    }
    
    *count = 8;
    return configs;
}