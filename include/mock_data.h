#ifndef MOCK_DATA_H
#define MOCK_DATA_H

#include "git_types.h"

const git_remote_t* get_mock_remotes(int *count);
const git_stash_entry_t* get_mock_stashes(int *count);
const git_commit_t* get_mock_commits(int *count);
const git_branch_t* get_mock_branches(int *count);
const git_branch_t* get_mock_remote_branches(int *count);
const git_file_status_t* get_mock_file_status(int *count);
const git_config_entry_t* get_mock_config_entries(const char *scope, int *count);

#endif // MOCK_DATA_H