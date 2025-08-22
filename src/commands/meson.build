# Commands directory sources

# Main command sources
commands_sources = files(
    'config.c',
    'init.c',
    'add.c',
    'commit.c',
    'status.c',
    'log.c',
    'branch.c',
)

# Include subdirectories
subdir('remote')
subdir('stash')

# Combine all command sources
commands_sources += remote_sources
commands_sources += stash_sources