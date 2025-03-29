#include "filesys.h"
#include "debug.h"
#include "utility.h"

#include <string.h>

#define DIRECTORY_ENTRY_SIZE (sizeof(inode_index_t) + MAX_FILE_NAME_LEN)
#define DIRECTORY_ENTRIES_PER_DATABLOCK (DATA_BLOCK_SIZE / DIRECTORY_ENTRY_SIZE)

// ----------------------- CORE FUNCTION ----------------------- //
int new_file(terminal_context_t *context, char *path, permission_t perms)
{
    (void) context;
    (void) path;
    (void) perms;
    return -2;
}

int new_directory(terminal_context_t *context, char *path)
{
    (void) context;
    (void) path;
    return -2;
}

int remove_file(terminal_context_t *context, char *path)
{
    (void) context;
    (void) path;
    return -2;
}

// we can only delete a directory if it is empty!!
int remove_directory(terminal_context_t *context, char *path)
{
    (void) context;
    (void) path;
    return -2;
}

int change_directory(terminal_context_t *context, char *path)
{
    (void) context;
    (void) path;
    return -2;
}

int list(terminal_context_t *context, char *path)
{
    (void) context;
    (void) path;
    return -2;
}

char *get_path_string(terminal_context_t *context)
{
    (void) context;

    return NULL;
}

int tree(terminal_context_t *context, char *path)
{
    (void) context;
    (void) path;
    return -2;
}

//Part 2
void new_terminal(filesystem_t *fs, terminal_context_t *term)
{
    (void) fs;
    (void) term;
    //check if inputs are valid

    //assign file system and root inode.
}

fs_file_t fs_open(terminal_context_t *context, char *path)
{
    (void) context;
    (void) path;

    //confirm path exists, leads to a file
    //allocate space for the file, assign its fs and inode. Set offset to 0.
    //return file

    return (fs_file_t)0;
}

void fs_close(fs_file_t file)
{
    (void)file;
}

size_t fs_read(fs_file_t file, void *buffer, size_t n)
{
    (void)file;
    (void)buffer;
    (void)n;

    return -2;
}

size_t fs_write(fs_file_t file, void *buffer, size_t n)
{
    (void)file;
    (void)buffer;
    (void)n;

    return -2;
}

int fs_seek(fs_file_t file, seek_mode_t seek_mode, int offset)
{
    (void)file;
    (void)seek_mode;
    (void)offset;

    return -2;
}

