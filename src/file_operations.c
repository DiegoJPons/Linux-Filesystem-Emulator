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
    
    info(1, "Checking dirr component: %s, Current File name: %s\n\n", dirname, (*opened_inode)->internal.file_name);

    for(size_t i = 0; i < entries ; i++) {
        byte *bytes = fs->dblocks + (index * 64) + (i * 16);

        dblock_index_t entry_index = (dblock_index_t)((bytes[1] << 8) | bytes[0]);
        inode_t *entry_inode = &fs->inodes[entry_index];
        char entry_name[MAX_FILE_NAME_LEN];
        strncpy(entry_name, (char*)(bytes + 2), MAX_FILE_NAME_LEN);


        info(1, "File name: '%s', Actual Name: '%s'\n", entry_name, fs->inodes[entry_index].internal.file_name);

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
            else if(strcmp(dirname, "..") == 0 && strcmp((*opened_inode)->internal.file_name, "root") == 0) {
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

    info(1, "File size: %zu, entries: %zu\n\n", (*opened_inode)->internal.file_size, entries);
    
    info(1, "Checking basename: %s\n\n", basename);
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
                info(1, "moving into basename: %s\n\n", basename)
                return 1;
            }
        }
    }
    return FILE_NOT_FOUND; 
}

fs_retcode_t get_inode(terminal_context_t *context, char *path, inode_t **opened_inode) {
    char *dirname = get_dirname(path);
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
    return SUCCESS;
}

int is_empty(terminal_context_t * context, inode_t *inode) {
    size_t entries = inode->internal.file_size / 16; 
    byte *dir_bytes = context->fs->dblocks + (inode->internal.direct_data[0] * 64);
   
    info(1, "INSIDE IS EMPTY\n");
    for (size_t i = 0; i < entries; i++) {
        byte *bytes = dir_bytes + (i * 16);
        char entry_name[MAX_FILE_NAME_LEN];

        info(1, "Entry name: %s\n", entry_name);
        strncpy(entry_name, (char*)(bytes + 2), MAX_FILE_NAME_LEN);
        if (strncmp(entry_name, ".", MAX_FILE_NAME_LEN) != 0 && strncmp(entry_name, "..", MAX_FILE_NAME_LEN) != 0) { 
            info(1, "NOT AN EMPTY INODE\n");
            return 0;
        }
    }

    return 1;
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

void update_directory(filesystem_t *fs, inode_t *parent_inode, inode_t *inode, char *file_name, inode_index_t index) {
    char current[16];
    current[0] = index & 0xff;
    current[1] = (index >> 8) & 0xff; 
    strncpy(current + 2, file_name, MAX_FILE_NAME_LEN);

    dblock_index_t block_index = parent_inode->internal.direct_data[0];
    byte* parent_bytes = fs->dblocks + (block_index * 64);

    char parent[16];
    parent[0] = parent_bytes[0] & 0xff;
    parent[1] = (parent_bytes[1] << 8) & 0xff; 
    strncpy(parent + 2, "..\n", MAX_FILE_NAME_LEN);

    dblock_index_t new_block;
    claim_available_dblock(fs, &new_block);
    byte* bytes = fs->dblocks + (new_block * 64);
    memcpy(bytes, current, 16);
    memcpy(bytes + 16, parent, 16);

    inode->internal.direct_data[0] = new_block;
    inode->internal.file_size = 32;
}

void update_parent_directory(filesystem_t *fs, inode_t **parent_inode, char *file_name, inode_index_t index) {

    char entry[16];
    entry[0] = index & 0xff;
    entry[1] = (index >> 8) & 0xff; 
    strncpy(entry + 2, file_name, MAX_FILE_NAME_LEN);
    
    info(1, "full entry: ");
    for(size_t j = 0; j < 16; j++) {
        info(1, "%c ", entry[j]);
    }
    info(1, "\n\n");
    size_t directory_entries = ((*parent_inode)->internal.file_size + 15) / 16;

    info(1, "Index: %u, num entries: %zu, file name: %s\n\n", index, directory_entries, (*parent_inode)->internal.file_name);
    byte* bytes;
    for(size_t i = 0; i < directory_entries; i++) {
        dblock_index_t dblock_index = (*parent_inode)->internal.direct_data[i / 4];
        bytes = fs->dblocks + (dblock_index * 64) + (i * 16);

        // info(1, "Directory entry (hex): ");
        // for (int j = 0; j < 16; j++) {
        //     info(1, "%02X ", bytes[j]);
        // }
        // info(1, "\n");

        if(memcmp(bytes, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 16) == 0) {
            info(1, "Tombstone was found at entry: %zu\n\n", i);
            memcpy(bytes, entry, 16);
            info(1, "entry stored: "); 
            for(size_t i = 0; i < 2; i++) {
                info(1, "%02x ", bytes[i]);
            }
            for(size_t j = 2; j < 16; j++) {
                info(1, "%c ", bytes[j]);
            }
             info(1, "\n");
            // (*parent_inode)->internal.file_size += 16;
            return;
        }
    }

    if(directory_entries % 4 == 0) {
        info(1, "Allocating new block for dirr entry!, entry num: %zu\n\n", directory_entries);

        dblock_index_t new_block;
        claim_available_dblock(fs, &new_block);
        (*parent_inode)->internal.direct_data[directory_entries / 4] = new_block;
        bytes = fs->dblocks + (new_block * 64) + (directory_entries % 4 * 16);
        memcpy(bytes, entry, 16);
    }

    else {
        info(1, "Adding dirr entry!, entry num: %zu\n\n", directory_entries);
        dblock_index_t dblock_index = (*parent_inode)->internal.direct_data[directory_entries / 4];
        bytes = fs->dblocks + (dblock_index * 64) + (directory_entries % 4 * 16);
        memcpy(bytes, entry, 16);
    }
}

int new_file(terminal_context_t *context, char *path, permission_t perms)
{
    if (context == NULL || path == NULL) {
        return 0;
    }

    inode_t *parent_inode = NULL;
    info(1, "Path: %s\n\n", path);
    fs_retcode_t code = get_inode(context, path, &parent_inode);

  
    if(code == DIR_NOT_FOUND) {
        REPORT_RETCODE(DIR_NOT_FOUND);
        info(1, "Debug DIR not FOund!\n");
        return -1;
    }

    char *basename = get_basename(path);
    code = check_basename(context->fs, basename, &parent_inode);
    if(code == 1) {
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
    
    char *file_name = malloc(MAX_FILE_NAME_LEN);
    strncpy(file_name, basename, MAX_FILE_NAME_LEN);

    inode_t *inode = &context->fs->inodes[inode_index];
    inode->internal.file_type = DATA_FILE;
    inode->internal.file_size = 0;
    inode->internal.file_perms = perms;
    strcpy(inode->internal.file_name, file_name);

    
    update_parent_directory(context->fs, &parent_inode, file_name, inode_index);

    info(1, "FINAL FILE NAME: %s\n\n", inode->internal.file_name);
    
    return 0;
}

int new_directory(terminal_context_t *context, char *path)
{
    if(context == NULL || path == NULL) {
        return 0;
    }

    inode_t *parent_inode = NULL;
    info(1, "Path: %s\n\n", path);
    fs_retcode_t code = get_inode(context, path, &parent_inode);

  
    if(code == DIR_NOT_FOUND) {
        REPORT_RETCODE(DIR_NOT_FOUND);
        info(1, "Debug DIR not FOund!\n");
        return -1;
    }

    char *basename = get_basename(path);
    code = check_basename(context->fs, basename, &parent_inode);
    if(code != FILE_NOT_FOUND) {
        REPORT_RETCODE(DIRECTORY_EXIST);
        info(1, "Debug directory exists!\n");
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
    
    char *file_name = malloc(MAX_FILE_NAME_LEN);
    strncpy(file_name, basename, MAX_FILE_NAME_LEN);

    inode_t *inode = &context->fs->inodes[inode_index];
    inode->internal.file_type = DIRECTORY;
    inode->internal.file_size = 0;
    inode->internal.file_perms = 0;
    strcpy(inode->internal.file_name, file_name);

    update_directory(context->fs, parent_inode, inode, file_name, inode_index);
    update_parent_directory(context->fs, &parent_inode, file_name, inode_index);

    return 0;
}

int remove_file(terminal_context_t *context, char *path)
{
    if (context == NULL || path == NULL) {
        return 0;
    }

    inode_t *parent_inode = NULL;
    info(1, "Path: %s\n\n", path);
    fs_retcode_t code = get_inode(context, path, &parent_inode);

  
    if(code == DIR_NOT_FOUND) {
        REPORT_RETCODE(DIR_NOT_FOUND);
        info(1, "Debug DIR not FOund!\n");
        return -1;
    }

    char *basename = get_basename(path);
    code = check_basename(context->fs, basename, &parent_inode);
    if(code != 1) {
        REPORT_RETCODE(FILE_NOT_FOUND);
        info(1, "Debug file exists!\n");
        return -1;
    }


    return 0;
}

// we can only delete a directory if it is empty!!
int remove_directory(terminal_context_t *context, char *path)
{
    if (context == NULL || path == NULL) {
        return 0;
    }

    info(1, "Path: %s\n", path);

    inode_t *inode = NULL;
    info(1, "Path: %s\n\n", path);
    fs_retcode_t code = get_inode(context, path, &inode);

  
    if(code == DIR_NOT_FOUND) {
        REPORT_RETCODE(DIR_NOT_FOUND);
        info(1, "Debug DIR not FOund!\n");
        return -1;
    }

    char *basename = get_basename(path);

    if(strncmp(basename, ".", MAX_FILE_NAME_LEN) == 0 || strncmp(basename, ".", MAX_FILE_NAME_LEN) == 0 ) {
        REPORT_RETCODE(INVALID_FILENAME);
        return -1;
    }
    code = check_basename(context->fs, basename, &inode);
    if(code != 1) {
        REPORT_RETCODE(DIR_NOT_FOUND);
        info(1, "Debug file exists!\n");
        return -1;
    }

    info(1, "Name of inode: %s\n", inode->internal.file_name);
    if(is_empty(context, inode) == 0) {
        REPORT_RETCODE(DIR_NOT_EMPTY);
        return -1;
    }

    if (context->working_directory == inode) {
        REPORT_RETCODE(ATTEMPT_DELETE_CWD);
        return -1;
    }

    return -1;
}

int change_directory(terminal_context_t *context, char *path)
{
    if (context == NULL || path == NULL) {
        return 0;
    }

    inode_t *inode = NULL;
    info(1, "Path: %s\n\n", path);
    fs_retcode_t code = get_inode(context, path, &inode);

    if(code == DIR_NOT_FOUND) {
        REPORT_RETCODE(DIR_NOT_FOUND);
        info(1, "Debug DIR not FOund!\n");
        return -1;
    }

    char *basename = get_basename(path);
    code = check_basename(context->fs, basename, &inode);

    if(code != 1) {
        REPORT_RETCODE(DIR_NOT_FOUND);
        info(1, "Debug file exists!\n");
        return -1;
    }

    context->working_directory = inode;
    return -2;
}

int list(terminal_context_t *context, char *path)
{
    if (context == NULL || path == NULL) {
        return 0;
    }

    inode_t *inode = NULL;
    info(1, "Path: %s\n\n", path);
    fs_retcode_t code = get_inode(context, path, &inode);

    if(code == DIR_NOT_FOUND) {
        REPORT_RETCODE(DIR_NOT_FOUND);
        return -1;
    }

    char *basename = get_basename(path);
    code = check_basename(context->fs, basename, &inode);

    if(code != 1) {
        REPORT_RETCODE(NOT_FOUND);
        return -1;
    }

    

    return -2;
}

char *get_path_string(terminal_context_t *context)
{
    char* string;

    if(context == NULL){
        string = malloc(1);
        string[0] = '\0';
        return string;
    }

    string = malloc(1);
    size_t current_string_size = 1;
    string[0] = '\0';
    inode_t *current_inode = context->working_directory;
    size_t done = 0;

    while(done != 1) {
        byte *bytes = context->fs->dblocks + (current_inode->internal.direct_data[0] * 64) + 16; 
        dblock_index_t parent_index = (dblock_index_t)((bytes[1] << 8) | bytes[0]);
        inode_t *parent_inode = &context->fs->inodes[parent_index];
        char* current_name = current_inode->internal.file_name;
        size_t current_name_len = strlen(current_name);

        if(strcmp(current_name, "root") == 0) {
            string = malloc(current_name_len + 1);
            strncpy(string, current_name, current_name_len);
            string[current_name_len] = '\0';
            return string;
        }

        string = realloc(string, current_string_size + current_name_len + 1);
        memmove(string + 1 + current_name_len, string, current_string_size);
        string[0] = '/';
        strncpy(string + 1, current_name, current_name_len);
        string[current_string_size + current_name_len + 1] = '\0';

        current_inode = parent_inode;
        current_string_size += 1 + current_name_len;

        if(strcmp(parent_inode->internal.file_name, "root") == 0) {
            done = 1;
        }

    }

    char* final_string = malloc(current_string_size + 4);
    strcpy(final_string, "root");
    strncpy(final_string + 4, string, current_string_size);
    free(string);

    return final_string;
}

int tree(terminal_context_t *context, char *path)
{
    if (context == NULL || path == NULL) {
        return 0;
    }

    inode_t *inode = NULL;
    fs_retcode_t code = get_inode(context, path, &inode);

    if(code == DIR_NOT_FOUND) {
        REPORT_RETCODE(DIR_NOT_FOUND);
        return -1;
    }

    char *basename = get_basename(path);
    code = check_basename(context->fs, basename, &inode);

    if(code != 1) {
        REPORT_RETCODE(NOT_FOUND);
        return -1;
    }

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
    char *basename = get_basename(path);
    code = check_basename(context->fs, basename, &opened_inode);
    if(code != 1){
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

