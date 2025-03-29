#include "filesys.h"
#include "utility.h"

#include <string.h>
#include <stdlib.h>

/**
 * !! DO NOT MODIFY THIS FILE !!
 */

#define DBLOCK_MASK_SIZE(blk_count) (((blk_count) + 7) / (sizeof(byte) * 8))
#define INDIRECT_DBLOCK_INDEX_COUNT (DATA_BLOCK_SIZE / sizeof(dblock_index_t) - 1)
#define INDIRECT_DBLOCK_MAX_DATA_SIZE ( DATA_BLOCK_SIZE * INDIRECT_DBLOCK_INDEX_COUNT )
#define NEXT_INDIRECT_INDEX_OFFSET (DATA_BLOCK_SIZE - sizeof(dblock_index_t))
#define DBLOCK_DISPLAY_LEN 16

const char *fs_retcode_string_table[FS_RETCODE_TOTAL] = {
    "Success",
    "Invalid input",
    "System Error",
    "No inodes available",
    "No dblocks available",
    "Not enough dblocks for operation",
    "Invalid file type",
    "Invalid binary format for filesystem",
    "File not found",
    "Directory not found",
    "Object not found",
    "Empty file name",
    "Invalid file name",
    "Directory is not empty",
    "File already exists",
    "Directory already exists",
    "Cannot delete current working directory",
    "Function not implemented"
};

// -------------------------------- HELPER FUNCTIONS -------------------------------- //

static void extract_filename(inode_t *inode, char *buffer)
{
    size_t i = 0;
    for (; i <= MAX_FILE_NAME_LEN; ++i)
    {
        if (inode->internal.file_name[i])
        {
            buffer[i] = inode->internal.file_name[i];
        }
        else break;
    }
    buffer[i] = '\0';
}

static void display_direct_dblock_indices(filesystem_t *fs, inode_t *node)
{
    size_t file_size = node->internal.file_size;
    size_t dblocks_needed = (file_size + DATA_BLOCK_SIZE - 1) / DATA_BLOCK_SIZE;
    
    size_t direct_dblocks_used = dblocks_needed < INODE_DIRECT_BLOCK_COUNT ? dblocks_needed : INODE_DIRECT_BLOCK_COUNT;

    for (size_t i = 0; i < direct_dblocks_used; ++i)
    {
        printf("%u ", node->internal.direct_data[i]);
    }
}

static void set_inode_mask(filesystem_t *fs, byte *mask)
{
    inode_index_t iter = fs->available_inode;
    while (iter != 0)
    {
        mask[iter / 8] |= 1 << (iter % 8);
        iter = fs->inodes[iter].next_free_inode;
    }
}

static void display_indirect_dblock_indices(filesystem_t *fs, inode_t *node)
{
    size_t file_size = node->internal.file_size;
    size_t dblocks_needed = (file_size + DATA_BLOCK_SIZE - 1) / DATA_BLOCK_SIZE;

    // since this func is only called if we know there must be indirect data block indices
    size_t indirect_dblocks_needed = dblocks_needed - INODE_DIRECT_BLOCK_COUNT;

    // take the index of the first index block
    dblock_index_t index_blk_idx = node->internal.indirect_dblock;
    size_t i = 0;
    while (i < indirect_dblocks_needed)
    {
        size_t indirect_idx_offset = i % INDIRECT_DBLOCK_INDEX_COUNT;
        // if we have looked through all the indices stored inside of an index block, we update to look at the next index block
        if (i != 0 && indirect_idx_offset == 0)
        {
            index_blk_idx = *cast_dblock_ptr(&fs->dblocks[ index_blk_idx * DATA_BLOCK_SIZE + NEXT_INDIRECT_INDEX_OFFSET ]);
        }
        // index_blk_idx * DATA_BLOCK_SIZE is the number of bytes into the byte array that data block number index_blk_idx begins
        // indirect_idx_offset * sizeof(dblock_index_t) is the number of bytes into the data block that the indirect_dblock_index index begins.
        // so, the line below returns the dblock index at index indirect_idx_offset in the index_blk_idx index block.
        dblock_index_t indirect_dblock_index = *cast_dblock_ptr(&fs->dblocks[ index_blk_idx * DATA_BLOCK_SIZE + indirect_idx_offset * sizeof(dblock_index_t) ]);
        printf("%u ", indirect_dblock_index);
        ++i;
    };  
}

static void display_indirect_index_indices(filesystem_t *fs, inode_t *node)
{
    size_t file_size = node->internal.file_size;
    size_t dblocks_needed = (file_size + DATA_BLOCK_SIZE - 1) / DATA_BLOCK_SIZE;

    // since this func is only called if we know there must be indirect data block indices
    size_t indirect_dblocks_needed = dblocks_needed - INODE_DIRECT_BLOCK_COUNT;

    // take the index of the first index block
    dblock_index_t index_blk_idx = node->internal.indirect_dblock;
    size_t i = 0;
    while (i < indirect_dblocks_needed)
    {
        size_t indirect_idx_offset = i % INDIRECT_DBLOCK_INDEX_COUNT;
        // if we have looked through all the indices stored inside of an index block, we update to look at the next index block
        if (i != 0 && indirect_idx_offset == 0)
        {
            index_blk_idx = *cast_dblock_ptr(&fs->dblocks[ index_blk_idx * DATA_BLOCK_SIZE + NEXT_INDIRECT_INDEX_OFFSET ]);
        }
        printf("%u ", index_blk_idx);
        i += INDIRECT_DBLOCK_INDEX_COUNT;
    };  
}

// -------------------------------- CORE FUNCTIONS -------------------------------- //

// calculates the number of index dblocks used for a file size
size_t calculate_index_dblock_amount(size_t file_size)
{
    if (file_size < DATA_BLOCK_SIZE * INODE_DIRECT_BLOCK_COUNT) return 0;
    return (file_size - DATA_BLOCK_SIZE * INODE_DIRECT_BLOCK_COUNT + INDIRECT_DBLOCK_MAX_DATA_SIZE - 1) / INDIRECT_DBLOCK_MAX_DATA_SIZE; 
}

// calculates the number of dblocks necessary for a file_size
// includes all data dblocks and index_dblocks
size_t calculate_necessary_dblock_amount(size_t file_size)
{
    return (file_size + DATA_BLOCK_SIZE - 1) / DATA_BLOCK_SIZE + calculate_index_dblock_amount(file_size);
}   

// non UB way to convert byte pointer to dblock_index_t pointer
dblock_index_t *cast_dblock_ptr(void *addr)
{
    // copy address from addr to ptr via memcpy
    // memcpy avoids potential UB pitfalls such as unaligned pointer casts
    // compiler should optimize the "memcpy" out of the program (therefore safe and no performance hit)
    dblock_index_t *ptr = NULL;
    memcpy(&ptr, &addr, sizeof(void*)); 
    return ptr;
}

fs_retcode_t save_filesystem(FILE* file, filesystem_t *fs)
{
    if (!fs || !file) return INVALID_INPUT;

    fwrite(&fs->inode_count, sizeof(fs->inode_count), 1, file); // write the inode count
    fwrite(&fs->available_inode, sizeof(fs->available_inode), 1, file); // write the next available inode
    fwrite(&fs->dblock_count, sizeof(fs->dblock_count), 1, file); // write the dblock count

    fwrite(fs->inodes, sizeof(inode_t), fs->inode_count, file); // write the inodes to file
    
    size_t block_bitmask_size = DBLOCK_MASK_SIZE(fs->dblock_count);
    fwrite(fs->dblock_bitmask, sizeof(byte), block_bitmask_size, file); // write the dblock bit masks

    fwrite(fs->dblocks, DATA_BLOCK_SIZE, fs->dblock_count, file); // write the data blocks

    return SUCCESS;
}

fs_retcode_t load_filesystem(FILE* file, filesystem_t *fs)
{
    if (!fs || !file) return INVALID_INPUT;
    // read the inode count 
    if (fread(&fs->inode_count, sizeof(fs->inode_count), 1, file) != 1) return INVALID_BINARY_FORMAT;
    // read the next available inode
    if (fread(&fs->available_inode, sizeof(fs->available_inode), 1, file) != 1) return INVALID_BINARY_FORMAT; 
    // read the dblock count
    if (fread(&fs->dblock_count, sizeof(fs->dblock_count), 1, file) != 1) return INVALID_BINARY_FORMAT; 

    fs->inodes = malloc(fs->inode_count * sizeof(inode_t));
    // read the inodes
    if (fread(fs->inodes, sizeof(inode_t), fs->inode_count, file) != fs->inode_count) return INVALID_BINARY_FORMAT; 

    size_t block_bitmask_size = DBLOCK_MASK_SIZE(fs->dblock_count);
    fs->dblock_bitmask = malloc(block_bitmask_size * sizeof(byte));
    // read the data blocks
    if (fread(fs->dblock_bitmask, sizeof(byte), block_bitmask_size, file) != block_bitmask_size) return INVALID_BINARY_FORMAT; 

    fs->dblocks = malloc(fs->dblock_count * DATA_BLOCK_SIZE);
    // read the data blocks
    if (fread(fs->dblocks, DATA_BLOCK_SIZE, fs->dblock_count, file) != fs->dblock_count) return INVALID_BINARY_FORMAT; 

    return SUCCESS;
}

static const char *filetype_str_table[] = {
    STR(DATA_FILE),
    STR(DIRECTORY)
};

void display_filesystem(filesystem_t *fs, fs_display_flag_t flag)
{
    if (!fs)
    {
        puts("No file system specified.");
        return;
    } 

    if (flag & DISPLAY_FS_FORMAT)
    {
        puts("File System Structure:");
        printf("\tavailable inode: %lu / %lu\n", available_inodes(fs), fs->inode_count);   
        printf("\tavailable dblock: %lu / %lu\n", available_dblocks(fs), fs->dblock_count);
    }

    if (flag & DISPLAY_INODES)
    {
        byte *inode_mask = calloc((fs->inode_count + 7) / 8, sizeof(byte));
        set_inode_mask(fs, inode_mask);
        puts("I-Node List:");
        for (size_t i = 0; i < fs->inode_count; ++i)
        {
            if (!(inode_mask[i / 8] & (1 << (i % 8))))
            {
                inode_t *inode = &fs->inodes[i];
                char filename[MAX_FILE_NAME_LEN + 1] = { 0 };
                extract_filename(inode, filename);

                if (inode->internal.file_perms)
                {
                    const char *rd_perm_str = inode->internal.file_perms & FS_READ ? "READ " : "";
                    const char *wr_perm_str = inode->internal.file_perms & FS_WRITE ? "WRITE " : "";
                    const char *x_perm_str = inode->internal.file_perms & FS_EXECUTE ? "EXECUTE " : "";
                    printf("\tinode index %lu [.type = %s .perm = %s%s%s .name = \"%s\" .size = %lu]\n", 
                        i, filetype_str_table[inode->internal.file_type],
                        rd_perm_str, wr_perm_str, x_perm_str, filename, inode->internal.file_size
                    );
                }
                else
                {
                    printf("\tinode index %lu [.type = %s .name = \"%s\" .size = %lu]\n", 
                        i, filetype_str_table[inode->internal.file_type],
                        filename, inode->internal.file_size
                    );
                }
                

                size_t file_size = inode->internal.file_size;

                if (file_size > 0)
                {
                    printf("\t\tDirect Data Blocks: ");
                    display_direct_dblock_indices(fs, inode);
                    puts("");
                    
                    if (file_size > DATA_BLOCK_SIZE * INODE_DIRECT_BLOCK_COUNT)
                    {
                        printf("\t\tIndirect Data Blocks: ");
                        display_indirect_dblock_indices(fs, inode);
                        puts("");

                        printf("\t\tIndirect Index Blocks: ");
                        display_indirect_index_indices(fs, inode);
                        puts("");
                    }
                }
            }
        }
        
        free(inode_mask);
    }

    if (flag & DISPLAY_DBLOCKS)
    {
        puts("Data Block List:");
        for (size_t idx = 0; idx < fs->dblock_count; ++idx)
        {
            size_t block_idx = idx / 8;
            size_t bit_idx = idx % 8;
            if (!(fs->dblock_bitmask[block_idx] & (1 << (7 - bit_idx))))
            {
                printf("\tdblock index %ld", idx);
                for (size_t k = 0; k < DATA_BLOCK_SIZE; ++k)
                {
                    if (k % DBLOCK_DISPLAY_LEN == 0) printf("\n\t\t");
                    printf("%02x ", fs->dblocks[idx * DATA_BLOCK_SIZE + k]);
                }
                printf("\n");
            }
        }
    }
}