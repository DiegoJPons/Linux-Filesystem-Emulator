#include "filesys.h"
#include "debug.h"
#include "utility.h"

#include <string.h>
#include <stdbool.h>

#define DIRECTORY_ENTRY_SIZE (sizeof(inode_index_t) + MAX_FILE_NAME_LEN)
#define DIRECTORY_ENTRIES_PER_DATABLOCK (DATA_BLOCK_SIZE / DIRECTORY_ENTRY_SIZE)

// ----------------------- CORE FUNCTION ----------------------- //
char* get_basename(char *path) {
    
    static char basename[MAX_FILE_NAME_LEN];
    char *last = strrchr(path, '/');
    
    if (last == NULL) {
        strcpy(basename, path);
    } else {
        size_t length = strrchr(path, '/') - path;
        size_t amount = strlen(path) - length;
        strncpy(basename, last + 1, amount);
    }

    return basename;
}

char* get_dirname(char *path) {

    static char dirname[MAX_FILE_NAME_LEN];
    char *last = strrchr(path, '/');
    
    if (last == NULL) {
        dirname[0] = '\0';
    } else {
        size_t length = strrchr(path, '/') - path;
        strncpy(dirname, path, length);
        dirname[length] = '\0';
    }
    return dirname;
}

int check_dirr_component(filesystem_t *fs, const char *dirname, inode_t **opened_inode) {

    dblock_index_t index = (*opened_inode)->internal.direct_data[0];
    size_t entries = ((*opened_inode)->internal.file_size + 15)  / 16;

    info(1, "File size: %zu, entries: %zu\n\n", (*opened_inode)->internal.file_size, entries);
    
    info(1, "Checking dirr component: %s\n\n", dirname);

    for(size_t i = 0; i < entries; i++) {
        byte *bytes = fs->dblocks + (index * 64) + (i * 16);

        dblock_index_t entry_index = (dblock_index_t)((bytes[1] << 8) | bytes[0]);
        inode_t *entry_inode = &fs->inodes[entry_index];
        char entry_name[MAX_FILE_NAME_LEN];
        strncpy(entry_name, (char*)(bytes + 2), MAX_FILE_NAME_LEN);

        info(1, "Directory entry (hex): ");
        for (int j = 0; j < 16; j++) {
            info(1, "%02X ", bytes[j]);
        }
        info(1, "\n");

        info(1, "File name: '%s'\n", entry_name);

        if(memcmp(bytes, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 16) == 0) {
            entries += 1;
            info(1, "Tombstone found!, entries: %zu\n\n", entries);
            continue;
        } 
        if(strcmp(entry_name, dirname) == 0) {
            info(1, "Found component: %s\n\n", dirname);
            if(strcmp(entry_name, ".") == 0) {
                return 1;
            }
            else {
                if(entry_inode->internal.file_type != DIRECTORY) {
                    info(1, "Invalid file type: %s\n\n", dirname);
                    return DIR_NOT_FOUND;
                }
                *opened_inode = entry_inode;
                return 1;
            }
        }
    }
    info(1, "component wasnt found: %s\n\n", dirname);
    return DIR_NOT_FOUND; 
}

int check_basename(filesystem_t *fs, const char *basename, inode_t **opened_inode) {
    
    dblock_index_t index = (*opened_inode)->internal.direct_data[0];
    size_t entries = (*opened_inode)->internal.file_size / 16;

    for(size_t i = 0; i <= entries; i++) {
        byte *bytes = fs->dblocks + (index * 64) + (i * 16);
        
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

fs_retcode_t get_inode(terminal_context_t *context, char *path, inode_t **opened_inode) {
    char *dirname = get_dirname(path);
    char *basename = get_basename(path);
    char *ptr = NULL;
    char *token = strtok_r(dirname, "/", &ptr);

   *opened_inode = &context->fs->inodes[0];

    while (token != NULL) {
        if(check_dirr_component(context->fs, token, opened_inode) == DIR_NOT_FOUND) {
            info(1, "Failed dirr component: %s\n\n", token);
            return DIR_NOT_FOUND;
        }
        token = strtok_r(NULL, "/", &ptr);
    }

    int check = check_basename(context->fs, basename, opened_inode);

    if(check != 1) {
        return check;
    }

    return SUCCESS;
}

fs_retcode_t enough_dblocks(terminal_context_t *context, inode_t *opened_inode) {
    size_t dblocks_available = available_dblocks(context->fs);

    size_t dirr_entries_in_last = opened_inode->internal.file_size / 16;

    if(dirr_entries_in_last % 4 == 0) {
        if(calculate_necessary_dblock_amount(opened_inode->internal.file_size) + 1 > dblocks_available) {
            return INSUFFICIENT_DBLOCKS;
        }
    }

    return SUCCESS;
}

void update_parent_directory(filesystem_t *fs, inode_t **parent_inode, char *file_name, inode_index_t index) {

    char entry[16];
    entry[0] = index & 0xff;
    entry[1] = (index >> 8) & 0xff;
    strcpy(entry + 2, file_name);
    
    size_t directory_entries = (*parent_inode)->internal.file_size / 16;
    byte* bytes;
    for(size_t i = 0; i < directory_entries; i++) {
        dblock_index_t dblock_index = (*parent_inode)->internal.direct_data[i / 4];
        bytes = fs->dblocks + (dblock_index * 64) + (i * 16);
        if(memcmp(bytes, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 16) == 0) {
            memcpy(bytes, entry, 16);
            return;
        }
    }

    if(directory_entries % 4 == 0) {
        dblock_index_t new_block;
        claim_available_dblock(fs, &new_block);
        (*parent_inode)->internal.direct_data[directory_entries / 4] = new_block;
        bytes = fs->dblocks + (new_block * 64) + (directory_entries % 4 * 16);
        memcpy(bytes, entry, 16);
    }

    else {
        dblock_index_t dblock_index = (*parent_inode)->internal.direct_data[directory_entries / 4];
        bytes = fs->dblocks + (dblock_index * 64) + (directory_entries % 4 * 16);
        memcpy(bytes, entry, 16);
    }

    // (*parent_inode)->internal.file_size += 16;
}
int new_file(terminal_context_t *context, char *path, permission_t perms)
{
    if (context == NULL || path == NULL) {
        return 0;
    }

    inode_t *parent_inode = NULL;
    fs_retcode_t code = get_inode(context, path, &parent_inode);

    info(1, "Path: %s\n\n", path);
    if(code == DIR_NOT_FOUND) {
        REPORT_RETCODE(DIR_NOT_FOUND);
        info(1, "Debug DIR not FOund!\n");
        return -1;
    }

    if(code == SUCCESS) {
        REPORT_RETCODE(FILE_EXIST);
        info(1, "Debug file exists!\n");
        return -1;
    }

    if(enough_dblocks(context, parent_inode) == INSUFFICIENT_DBLOCKS) {
        REPORT_RETCODE(INSUFFICIENT_DBLOCKS);
        info(1, "insufficient dblocks!\n");
        return -1;
    }
   
    inode_index_t inode_index; 
    code = claim_available_inode(context->fs, &inode_index);
    if (code == INODE_UNAVAILABLE) {
        REPORT_RETCODE(INODE_UNAVAILABLE);
        info(1, "inode unavailable!\n");
        return -1; 
    }
    char *basename = get_basename(path);
    char *file_name = malloc(MAX_FILE_NAME_LEN);
    strncpy(file_name, basename, MAX_FILE_NAME_LEN);

    inode_t *inode = &context->fs->inodes[inode_index];
    inode->internal.file_type = DATA_FILE;
    inode->internal.file_size = 0;
    inode->internal.file_perms = perms;
    strcpy(inode->internal.file_name, file_name);
    
    update_parent_directory(context->fs, &parent_inode, file_name, inode_index);
    
    return 0;
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

fs_file_t fs_open(terminal_context_t *context, char *path)
{
    if (path == NULL || context == NULL) {
        return NULL;
    }

    inode_t *opened_inode = NULL;
    fs_retcode_t code = get_inode(context, path, &opened_inode);

    if(code != SUCCESS) {
        REPORT_RETCODE(code);
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

