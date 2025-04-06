#include "filesys.h"
#include "debug.h"
#include "utility.h"

#include <string.h>
#include <stdbool.h>

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
    if(fs == NULL || term == NULL) {
        return;
    }

    term->fs = fs;
    term->working_directory = &fs->inodes[0];
}

int check_dirr_component(filesystem_t *fs, const char *dirname, inode_t **opened_inode) {

    dblock_index_t index = (*opened_inode)->internal.direct_data[0];
    size_t entries = (*opened_inode)->internal.file_size / 16;
    
    for(size_t i = 0; i <= entries; i++) {
        byte *bytes;
        
        if(i > 16) {
            size_t used_dblocks = ((*opened_inode)->internal.file_size / 64) - 4;
            size_t iblock_num = used_dblocks / 15;
            size_t blocks_in_current_iblock = used_dblocks % 15;
   
            dblock_index_t current = (*opened_inode)->internal.indirect_dblock;
            for (size_t i = 0; i < iblock_num; i++) {
                dblock_index_t *dpointers = cast_dblock_ptr(fs->dblocks + current * 64);
                current = dpointers[15];  
            }

            dblock_index_t *index = cast_dblock_ptr(fs->dblocks + current * 64);
            dblock_index_t data_block = index[blocks_in_current_iblock];
            bytes = fs->dblocks + (data_block * 64);
        }
        else {
            bytes = fs->dblocks + (index * 64) + (i * 16);
        }

        dblock_index_t entry_index = (dblock_index_t)((bytes[1] << 8) | bytes[0]);
        inode_t *entry_inode = &fs->inodes[entry_index];
        char entry_name[MAX_FILE_NAME_LEN];
        strncpy(entry_name, (char*)(bytes + 2), MAX_FILE_NAME_LEN);

        if(strcmp(entry_name, dirname) == 0) {
            if(strcmp(entry_name, ".") == 0 || ((strcmp(entry_name, "..") == 0) && (strcmp((*opened_inode)->internal.file_name, "root") == 0))) {
                return 1;
            }
            else {
                if(entry_inode->internal.file_type != DIRECTORY) {
                    return DIR_NOT_FOUND;
                }
                *opened_inode = entry_inode;
                return 1;
            }
        }
    }
    return DIR_NOT_FOUND; 
}

int check_basename(filesystem_t *fs, const char *basename, inode_t **opened_inode) {
    
    dblock_index_t index = (*opened_inode)->internal.direct_data[0];
    size_t entries = (*opened_inode)->internal.file_size / 16;

    for(size_t i = 0; i <= entries; i++) {
        byte *bytes;
        
        if(i > 16) {
            size_t used_dblocks = ((*opened_inode)->internal.file_size / 64) - 4;
            size_t iblock_num = used_dblocks / 15;
            size_t blocks_in_current_iblock = used_dblocks % 15;
   
            dblock_index_t current = (*opened_inode)->internal.indirect_dblock;
            for (size_t i = 0; i < iblock_num; i++) {
                dblock_index_t *dpointers = cast_dblock_ptr(fs->dblocks + current * 64);
                current = dpointers[15];  
            }

            dblock_index_t *index = cast_dblock_ptr(fs->dblocks + current * 64);
            dblock_index_t data_block = index[blocks_in_current_iblock];
            bytes = fs->dblocks + (data_block * 64);
        }
        else {
            bytes = fs->dblocks + (index * 64) + (i * 16);
        }

        dblock_index_t entry_index = (dblock_index_t)((bytes[1] << 8) | bytes[0]);
        inode_t *entry_inode = &fs->inodes[entry_index];
        char entry_name[MAX_FILE_NAME_LEN];
        strncpy(entry_name, (char*)(bytes + 2), MAX_FILE_NAME_LEN);

        if(strcmp(entry_name, basename) == 0) {
            if(strcmp(entry_name, ".") == 0 || ((strcmp(entry_name, "..") == 0) && (strcmp((*opened_inode)->internal.file_name, "root") == 0))) {
                return 1;
            }
            else {
                if(entry_inode->internal.file_type != DATA_FILE) {
                    return INVALID_FILE_TYPE;
                }
                *opened_inode = entry_inode;
                return 1;
            }
        }
    }
    return FILE_NOT_FOUND; 
}

fs_file_t fs_open(terminal_context_t *context, char *path)
{
    if (path == NULL || context == NULL) {
        return NULL;
    }

    char dirname[MAX_FILE_NAME_LEN];
    char basename[MAX_FILE_NAME_LEN];
    char *last = strrchr(path, '/');
    
    if (last == NULL) {
        dirname[0] = '\0';
        strcpy(basename, path);
    } else {
        size_t length = strrchr(path, '/') - path;
        size_t amount = strlen(path) - length;
        strncpy(basename, last + 1, amount);
        strncpy(dirname, path, strrchr(path, '/') - path);
        dirname[length] = '\0';
    }

    char *ptr = NULL;
    char *token = strtok_r(dirname, "/", &ptr);

    inode_t *opened_inode = &context->fs->inodes[0];

    while (token != NULL) {
        if(check_dirr_component(context->fs, token, &opened_inode) == DIR_NOT_FOUND) {
            REPORT_RETCODE(DIR_NOT_FOUND);
            return NULL;
        }
        token = strtok_r(NULL, "/", &ptr);
    }

    int check = check_basename(context->fs, basename, &opened_inode);

    if(check != 1) {
        REPORT_RETCODE(check);
        return NULL;
    }
   
    fs_file_t file = (fs_file_t)malloc(sizeof(struct fs_file));
    file->offset = 0;
    file->fs = context->fs;
    file->inode = opened_inode;
    
    return file;
}

void fs_close(fs_file_t file)
{
   if(file != NULL) {
    free(file);
   }
}

size_t fs_read(fs_file_t file, void *buffer, size_t n)
{
    if(file == NULL) {
        return 0;
    }

    filesystem_t *fs = file->fs;
    inode_t *inode = file->inode;
    size_t bytes_to_read = (file->offset + n < inode->internal.file_size) ? n : inode->internal.file_size - file->offset;
    size_t bytes_read = 0;
    fs_retcode_t code = inode_read_data(fs, inode, file->offset, buffer, bytes_to_read, &bytes_read);

    if (code == SUCCESS) {
        file->offset += bytes_to_read;
    }
    return bytes_read;
}

size_t fs_write(fs_file_t file, void *buffer, size_t n)
{
    if(file == NULL) {
        return 0;
    }

    filesystem_t *fs = file->fs;
    inode_t *inode = file->inode;

    fs_retcode_t code = inode_modify_data(fs, inode, file->offset, buffer, n);

    if(code == INSUFFICIENT_DBLOCKS) {
        return 0;
    }

    if (code == SUCCESS) {
        file->offset += n;
    }
    return n;
}

int fs_seek(fs_file_t file, seek_mode_t seek_mode, int offset)
{
    if(file == NULL) {
        return -1;
    }

    int final_offset;

    if(seek_mode == FS_SEEK_START) {
        final_offset = offset;
    }
    else if(seek_mode == FS_SEEK_CURRENT) {
        final_offset = file->offset + offset;
    }
    else if(seek_mode == FS_SEEK_END) {
        final_offset = file->inode->internal.file_size + offset;
    }
    else {
        return -1;
    }

    if(final_offset < 0) {
        return -1;
    }
   
    final_offset = (final_offset > (int) file->inode->internal.file_size) ? (int) file->inode->internal.file_size : final_offset;
    
    file->offset = final_offset;

    return 0;
}

