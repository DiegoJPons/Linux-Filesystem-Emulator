#ifndef FILESYS_H
#define FILESYS_H

/**
 * !! DO NOT MODIFY THIS FILE !!
 */

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#define STR(x) #x

#define DATA_BLOCK_SIZE 64
#define MAX_FILE_NAME_LEN 14
#define INODE_DIRECT_BLOCK_COUNT 4

#define REPORT_RETCODE(retcode) \
do { \
    fprintf(stdout, "Error: %s\n", fs_retcode_string_table[retcode]); \
} while (0)

typedef uint8_t byte;
typedef uint32_t dblock_index_t;
typedef uint16_t inode_index_t;

typedef enum fs_retcode
{
    SUCCESS,
    INVALID_INPUT,
    SYSTEM_ERROR,
    INODE_UNAVAILABLE,
    DBLOCK_UNAVAILABLE,
    INSUFFICIENT_DBLOCKS,
    INVALID_FILE_TYPE,
    INVALID_BINARY_FORMAT,
    FILE_NOT_FOUND,
    DIR_NOT_FOUND,
    NOT_FOUND,
    EMPTY_FILENAME,
    INVALID_FILENAME,
    DIR_NOT_EMPTY,
    FILE_EXIST,
    DIRECTORY_EXIST,
    ATTEMPT_DELETE_CWD,
    NOT_IMPLEMENTED,
    FS_RETCODE_TOTAL
} fs_retcode_t;

extern const char *fs_retcode_string_table[FS_RETCODE_TOTAL];

typedef enum file_type
{
    DATA_FILE,
    DIRECTORY
} file_type_t;

typedef enum permission
{
    FS_READ = 0x1,
    FS_WRITE = 0x2,
    FS_EXECUTE = 0x4
} permission_t;

struct inode_internal
{
    file_type_t file_type;
    permission_t file_perms;
    char file_name[MAX_FILE_NAME_LEN];
    size_t file_size;
    dblock_index_t direct_data[INODE_DIRECT_BLOCK_COUNT];
    dblock_index_t indirect_dblock;
};

typedef union inode
{
    inode_index_t next_free_inode;
    struct inode_internal internal;
} inode_t;

typedef struct filesystem
{   
    inode_index_t available_inode; 
    inode_t *inodes;
    size_t inode_count;
    byte *dblock_bitmask;
    byte *dblocks;
    size_t dblock_count;
} filesystem_t;

/*----------------------------------------------------*
 |  PART 0: INITIALIZATION & INODE/DBLOCK ALLOCATION  |
 |  THIS PART IS OPTIONAL. THE CODE IS PROVIDED.      |
 |  functions you need to implement:                  |
 |      1. `new_filesystem`                           |
 |      2. `free_filesystem`                          |
 |      3. `available_inodes`                         |
 |      4. `available_dblocks`                        |
 |      5. `claim_available_inode`                    |
 |      6. `claim_available_dblock`                   |
 |      7. `release_inode`                            |
 |      8. `release_dblock`                           |
 |  Relevant Sections in filesystem.md:               |
 |      - Background and Design Details               |
 |      - Part 0 and Part 1 Structs                   |
 |      - Part 0 functions                            |
 *----------------------------------------------------*/

/**
 * creates a new filesystem along with the root directory.
 * should dynamically allocate the list of inodes, list of data blocks, 
 * and list of bytes for the data block bitmask. These lists should be initialized to
 * 0 except the bitmask whose bits are all set to 1.
 * 
 * the first inode is the root directory named "root" of the file system. 
 * the root directory should have size 0 and not be linked to any data blocks 
 * (all indices set to 0).
 * 
 * the remaining inodes are inactive and each should have the `next_available_inode`
 * be set to the immediate index to the right. the exception is the last inode in the
 * list whose `next_available_inode` is set to 0. the `available_inode` is set to 1.
 * 
 * all the bits of the data block bitmasks should be set to 1 to mark all data blocks as
 * available (even if the number of bits exceed the number of data blocks). the exception
 * is the first bit which should be set to 0 to mark the first dblock as allocated.
 * 
 * the first dblock should contain one directory entry. the first directory entry
 * should have an inode index of 0 and have the entry name be '.'
 * 
 * @param fs the file system to initialize
 * @param inode_total the total number of inodes in the file system
 * @param dblock_total the total number of data blocks in the file system
 * @return SUCCESS if file system is correctly initilaized.
 *         INVALID_INPUT if `inode_total` or `dblock_total` is equal to 0.
 *         INVALID_INPUT if fs is null 
 */
fs_retcode_t new_filesystem(filesystem_t *fs, size_t inode_total, size_t dblock_total);

/**
 * free any buffer allocated for `fs`, but does not attempt to free `fs` itself.abs
 * if fs is null, then do not free anything.
 * 
 * @param fs the file system to free
 */
void free_filesystem(filesystem_t *fs);


// EXTRACT DATA ABOUT FILE SYSTEM

/**
 * calculates the available number of inodes in a file system
 * 
 * iterates through inactive inodes via their `next_free_inode` field,
 * starting from the `available_inode` field of the file system argument
 * 
 * @param fs the file system to calculate the available inodes in
 * @return the number of available inodes in the `fs`. if `fs` is null, 0.
 */
size_t available_inodes(filesystem_t *fs);

/**
 * calculates the available number of data blocks in a file system
 * 
 * uses the bitmask in `fs` to calculate the available data blocks.
 * if the bit is set, then it is available. if not, then it is unavailable.
 * 
 * @param fs the file system to calculate the available data blocks in
 * @return the number of available data blocks in the `fs`. if `fs` is null, 0.
 */
size_t available_dblocks(filesystem_t *fs);

/**
 * claims the available inode for the caller and mark it as now unavailable until
 * it is released.
 * 
 * uses the `available_inode` field of the `fs` to claim that inode. 
 * `available_inode` needs to be correctly updated to be the index of the subsequent
 * free inode to the one being claimed. the index of the claimed inode is stored
 * in the `index` pointer.
 * 
 * @param fs the file system to claim the inode from
 * @param index the address to store the index of the claimed inode in
 * @return SUCCESS if the inode is successfully claimed.
 *         INVALID_INPUT if `fs` is null.
 *         INODE_UNAVAILABLE if there are no available inodes.
 */
fs_retcode_t claim_available_inode(filesystem_t *fs, inode_index_t *index);

/**
 * claim the available data block for the caller and marks it as unavailable until it is
 * released.
 * 
 * uses the `dblock_bitmask` of `fs` to determine the index of the first available data block.
 * the bitmask is updated to mark the data block as unavailable. 
 * 
 * @param fs the file system to claim the data block from
 * @param index the address to store the index of the claimed data block in
 * @return SUCCESS if the data block is successfully claimed.
 *         INVALID_INPUT if `fs` is null.
 *         DBLOCK_UNAVAILABLE if there are no available data blocks.
 */
fs_retcode_t claim_available_dblock(filesystem_t *fs, dblock_index_t *index);

/**
 * releases a claimed inode and marks it as available now
 * 
 * `inode` is marked unavailable by setting its `next_free_inode` field as the as the
 * file system's `available_inode` and then updating `available_inode` to the index of
 * `inode` in the inode list.
 * 
 * does not modify the contents of `inode_internal`, only the `next_free_inode` field. 
 * there is not point in zeroing out that data since it will be assumedly overwritten
 * by a caller to `claim_available_inode`
 * 
 * @param fs the file system to release the inode
 * @param inode the inode to release
 * @return SUCCESS if the inode is successfully released.
 *         INVALID INPUT if either `fs` and `inode` is null.
 *         INVALID_INPUT if `inode` is the root directory.
 */
fs_retcode_t release_inode(filesystem_t *fs, inode_t *inode);

/**
 * releases a claimed data block and marks it as unavailable now
 * 
 * the index of the dblock is set to 0 in the `dblock_bitmask` field of `fs`. 
 * the data within dblock should not be modified.
 * 
 * @param fs the file system to release the data block in
 * @param dblock the data block to release
 * @return SUCCESS if the data block is successfully released.
 *         INVALID_INPUT if either `fs` or `dblock` is null.
 *         INVALID_INPUT if `data_block` does not point to the beginning of a data block.
 */
fs_retcode_t release_dblock(filesystem_t *fs, byte *dblock);

/*---------------------------------------------*
 |  PART 1: LOW LEVEL INODE-DATA MANIPULATION  |
 |  functions you need to implement:           |
 |      1. `inode_write_data`                  |
 |      2. `inode_read_data`                   |
 |      3. `inode_modify_data`                 |
 |      4. `inode_shrink_data`                 |
 |      5. `inode_release_data`                |
 |  Relevant Sections in filesys.md:           |
 |      - I-Nodes                              |
 |      - D-Blocks                             |
 |      - I-Node and D-block: The Big Picture  |
 |      - Part 0 and Part 1 Structs            |
 |      - Part 1 Functions                     |
 *---------------------------------------------*/
 
/**
 * writes data to the data blocks associated with an inode
 * 
 * the data is written to the end of the data contained by the inode. 
 * data blocks and index data blocks should be allocated as necessary.
 * the direct data blocks, if not filled, should be written to first.
 * subsequently, write the remaining data to the indirect data blocks. 
 * 
 * if there is not enough data blocks to satisfy the write, then the file
 * system should NOT be modified. 
 * 
 * @param fs the file system the inode is in
 * @param inode the inode to write data in
 * @param data the data to write to the inode
 * @param n the number of bytes in data to write to the inode
 * @return SUCCESS if the data is successfully written
 *         INVALID_INPUT if fs or inode is null
 *         INSUFFICIENT_DBLOCKS if there is not enough available data blocks
 */
fs_retcode_t inode_write_data(filesystem_t *fs, inode_t *inode, void *data, size_t n);

/**
 * reads data from the data blocks from an inode and stores it in a buffer
 * 
 * reads the `n` bytes data stored starting at `offset` in an inode and writes that 
 * data into a buffer. if there is not `n` bytes from the `offset` in the inode, then
 * read the bytes until the end. the `bytes_read` should be updated with the actual
 * number of bytes read from the inode and written to the buffer.
 * 
 * @param fs the file system the inode is in
 * @param inode the inode to read data from
 * @param offset the offset into the data to read from
 * @param buffer the buffer to store the data into
 * @param n the number of bytes to read from the inode starting from the offset
 * @param bytes_read the address to store the number of bytes actually read
 * @return SUCCESS if the data is successfully read 
 *         INVALID_INPUT if fs or inode or bytes_read is null
 */
fs_retcode_t inode_read_data(filesystem_t *fs, inode_t *inode, size_t offset, void *buffer, size_t n, size_t *bytes_read);

/**
 * modifies data in the data block associated with the inode starting from an offset. 
 * 
 * starting at the `offset` (as long as offset is <= file size), modify any data 
 * stored in the inode with the bytes in the buffer. if there is no more data to modify, 
 * then then write the remaining bytes to the end of the file.asm
 * 
 * if there is not enough data blocks to satisfy the modify, then the file
 * system should NOT be modified. 
 *
 * @param fs the file system the inode is in
 * @param inode the inode to modify the data
 * @param offset the offset into the data to modify the data
 * @param buffer the new data to be stored in the inode
 * @param n the number of bytes in the buffer to write
 * @return SUCCESS if the data is successfully modified
 *         INVALID_INPUT if the fs or inode is null
 *         INVALID_INPUT if the offset exceeds the size of the file
 *         INSUFFICIENT_DBLOCKS if there is not enough available data blocks
 */
fs_retcode_t inode_modify_data(filesystem_t *fs, inode_t *inode, size_t offset, void *buffer, size_t n);

/**
 * shrinks the inode file size and frees any D-block as necessary
 * 
 * if all the dblocks are freed that are referenced in an index dblock, the index dblock should then be freed.
 * the file size of the inode should also be updated to the new_size 
 * 
 * @param fs the file system the inode is in
 * @param inode the inode to shrink
 * @param new_size the smaller inode size
 * @return SUCCESS if the inode is successfully shrunk
 *         INVALID_INPUT if fs or inode is null
 */
fs_retcode_t inode_shrink_data(filesystem_t *fs, inode_t *inode, size_t new_size);

/**
 * releases all the data associated with a data block and updates the file size to
 * 0. all data blocks including index data blocks (if present) should be released.
 * 
 * @param fs the file system the inode is in
 * @param inode the inode to release the data from
 * @return SUCCESS if the data is successfully released
 *         INVALID_INPUT if fs or inode is null
 */
fs_retcode_t inode_release_data(filesystem_t *fs, inode_t *inode);

typedef struct terminal_context
{
    filesystem_t *fs;
    inode_t *working_directory;
} terminal_context_t;

struct fs_file
{
    filesystem_t *fs;
    inode_t *inode;
    size_t offset;
};

typedef struct fs_file *fs_file_t;

/*----------------------------------------------*
 |  PART 2: HIGH LEVEL FILE IO                  |
 |  functions you need to implement:            |
 |      1. `new_terminal`                       |
 |      2. `fs_open`                            |
 |      3. `fs_close`                           |
 |      4. `fs_read`                            |
 |      5. `fs_write`                           |
 |      6. `fs_seek`                            |
 |  Relevant Sections in filesys.md:            |
 |      - Part 1 Functions                      |
 |      - Part 2 and Part 3 Structs and Macros  |
 |      - Part 2 and Part 3 Core Concepts       |
 |      - Part 2 Functions                      |
 *----------------------------------------------*/

/**
 * creates a new terminal starting at the root directory
 * 
 * @param fs the file system to create the terminal for
 * @param context the address of terminal to initialize
 */
void new_terminal(filesystem_t *fs, terminal_context_t *term);

/**
 * opens a file and returns a file object for modification
 * 
 * if the file type is not DATA_FILE, then this is an error
 * 
 * @param context the context containing information about the file system
 * and the current working directory
 * @param path the path of the file relative to the terminal context
 * @return an address to the dynamically allocated file handler, null on any error
 */
fs_file_t fs_open(terminal_context_t *context, char *path);

/**
 * closes a file by deallocating the file object.
 * if file is NULL, do nothing.
 * 
 * @param file the file to be closed
 */
void fs_close(fs_file_t file);

/**
 * reads the content of a file and stores it in a buffer
 * 
 * @param file the file handler returned by `fs_open`
 * @param buffer the buffer to store the data in
 * @param n the number of bytes to read from the file
 * @return the number of bytes read. if `file` is null, return 0.
 */
size_t fs_read(fs_file_t file, void *buffer, size_t n);

/**
 * write the content of a file from a buffer
 * 
 * @param file the file handler returned by `fs_open`
 * @param buffer the buffer to write the data from
 * @param n the number of bytes to write to the file
 * @return the number of bytes written. if `file` is null or any error, return 0.
 */
size_t fs_write(fs_file_t file, void *buffer, size_t n);

typedef enum seek_mode
{
    FS_SEEK_CURRENT,
    FS_SEEK_START,
    FS_SEEK_END
} seek_mode_t;

/**
 * moves the currnet position in the file
 * 
 * @param file the file handler returned by `fs_open`
 * @param seek_mode the mode for seek
 * @param offset the offset relative to the seek_mode 
 * @return 0 if successful, -1 if any error occurs
 */
int fs_seek(fs_file_t file, seek_mode_t seek_mode, int offset);

/*----------------------------------------------*
 |  PART 3: HIGH LEVEL FILE SYSTEM OPERATIONS   |
 |  functions you need to implement:            |
 |      1. `new_terminal`                       |
 |      2. `new_file`                           |
 |      3. `new_directory`                      |
 |      4. `remove_file`                        |
 |      5. `remove_directory`                   |
 |      6. `change_directory`                   |
 |      7. `list`                               |
 |      8. `get_path_string`                    |
 |      9. `tree`                               |
 |  Relevant Sections in filesys.md:            |
 |      - File and Directory Representation     |
 |      - File System Overview                  |
 |      - Part 0 Functions                      |
 |      - Part 1 Functions                      |
 |      - Part 2 and Part 3 Structs and Macros  |
 |      - Part 2 and Part 3 Core Concepts       |
 |      - Part 3 Functions                      |
 *----------------------------------------------*/

// HIGH LEVEL FILE SYSTEM OPERATIONS

/**
 * creates a new file in a directory
 * 
 * @param context the context containing information about the file system 
 * and the parent directory
 * @param path the path to the new file
 * @param perms the permission of the file
 * @return 0 if successful. -1 on any failure.
 */
int new_file(terminal_context_t *context, char *path, permission_t perms);

/**
 * creates a new directory in a parent directory
 * 
 * @param context the context containing information about the file system
 * and the parent directory
 * @param name the path to the file
 * @return 0 if successful, -1 on any failure.
 */
int new_directory(terminal_context_t *context, char *path);

/**
 * deletes a file in a directory 
 * 
 * @param context the context containing information about the file system
 * and the current working directory
 * @param path the path to to be deleted file relative to the current working directory
 * @return 0 if successful, -1 on any failure.
 */
int remove_file(terminal_context_t *context, char *path);

/**
 * deletes a directory, if empty. 
 * 
 * @param context the context containing information about the file system
 * and the current working directory
 * @param path the path to the to be deleted directory relative to the current working
 * directory
 * @return 0 if successful, -1 on any failure.
 */
int remove_directory(terminal_context_t *context, char *path);

/**
 * change the current working directory
 * 
 * @param context the context containing information about the file system
 * and the current working directory
 * @param path the new working directory relative to the current working directory
 * @return 0 if successful, -1 if an error occurs
 */
int change_directory(terminal_context_t *context, char *path);

/**
 * list the contents of a directory
 * 
 * @param context the context the context containing information about the file system
 * and the current working directory
 * @param path the directory relative to current working directory to list the files in
 * @return 0 if successful, -1 if an error occurs
 */
int list(terminal_context_t *context, char *path);

/**
 * returns a string for the path of the working directory
 * 
 * @param context the context contianing information about the file system and the current
 * working directory
 * @return a dynamically allocated string for the path
 */
char *get_path_string(terminal_context_t *context);

/**
 * displays the content of a directory as a tree
 * 
 * @param context the context contianing information about the file system and the current
 * working directory
 * @param path the file or directory relative to the current working directory to show
 * the contents of as a tree
 * @return 0 if successful, -1 if an error occurs
 */
int tree(terminal_context_t *context, char *path);


// ---------------------------------------------------------------------------------------------------- //
/**
 * !! THE FOLLOWING FUNCTIONS ARE ALREADY IMPLEMENTED. 
 * !! Do not implement these functions yourself.
 * !! THESE FUNCTIONS ARE IMPLEMENTED ALREADY IN src/utility.c
 * 
 * !! SEE "Utility Functions" section in the filesystem.md file to read more about using these functions
 */

 /**
 * loads a file system from a input file
 * 
 * @param file the input file to load the file system from
 * @param fs the filesystem to write the content of the input file to
 * @return SUCCESS if the file system is correctly loaded
 */
fs_retcode_t load_filesystem(FILE* file, filesystem_t *fs);

/**
 * stores a file system to an output file
 * 
 * @param file the output file to write the file system to
 * @param fs the file system to store in the output file
 * @return SUCCESS if the file system is correctly saved
 */
fs_retcode_t save_filesystem(FILE* file, filesystem_t *fs);

// DEBUGGING FUNCTION

typedef enum fs_display_flag
{
    DISPLAY_FS_FORMAT = 0x1,
    DISPLAY_INODES = 0x2,
    DISPLAY_DBLOCKS = 0x4,
    DISPLAY_ALL = 0x7
} fs_display_flag_t;

/**
 * displays a filesystem to standard output
 * 
 * @param fs the file system to display in stdout
 * @param flag the flag that describes what information about the filesystem
 * should be displayed.
 */
void display_filesystem(filesystem_t *fs, fs_display_flag_t flag);

#endif