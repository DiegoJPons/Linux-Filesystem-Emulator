#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filesys.h"
#include "debug.h"
#include "utility.h"

#define DBLOCK_MASK_SIZE(blk_count) (((blk_count) + 7) / (sizeof(byte) * 8))

#define INDIRECT_DBLOCK_INDEX_COUNT (DATA_BLOCK_SIZE / sizeof(dblock_index_t) - 1)
#define INDIRECT_DBLOCK_MAX_DATA_SIZE ( DATA_BLOCK_SIZE * INDIRECT_DBLOCK_INDEX_COUNT )

#define DIRECTORY_ENTRY_SIZE (sizeof(inode_index_t) + MAX_FILE_NAME_LEN)
#define DIRECTORY_ENTRIES_PER_DATABLOCK (DATA_BLOCK_SIZE / DIRECTORY_ENTRY_SIZE)

// ----------------------- UTILITY FUNCTION ----------------------- //

// marks the nth dblock as being used 
static void mark_dblock_as_used(byte *dblock_bitmask, size_t n)
{
    dblock_bitmask[n / 8] &= ~(1 << (7 - n % 8));
}

static void mark_dblock_as_unused(byte *dblock_bitmask, size_t n)
{
    dblock_bitmask[n / 8] |= 1 << (7 - n % 8);
}

// ----------------------- CORE FUNCTION ----------------------- //

fs_retcode_t new_filesystem(filesystem_t *fs, size_t inode_total, size_t dblock_total)
{
    if (!fs) return INVALID_INPUT;
    if (inode_total == 0 || dblock_total == 0) return INVALID_INPUT;

    // allocate the inodes
    inode_t *inodes = calloc(inode_total, sizeof(inode_t));
    if (!inodes) return SYSTEM_ERROR;

    for (size_t i = 0; i < inode_total - 1; ++i) inodes[i].next_free_inode = i + 1;
    inodes[inode_total - 1].next_free_inode = 0;

    // allocate the dblocks
    byte *dblocks = calloc(dblock_total, DATA_BLOCK_SIZE);
    if (!dblocks) return SYSTEM_ERROR;

    // allocate the bitmask for the dblock availability
    size_t bit_mask_byte_size = DBLOCK_MASK_SIZE(dblock_total);
    byte *dblock_bitmask = malloc(bit_mask_byte_size * sizeof(byte));
    if (!dblock_bitmask) return SYSTEM_ERROR;
    memset(dblock_bitmask, 0xFF, bit_mask_byte_size);
    
    // initialize root directory
    inodes[0].internal.file_type = DIRECTORY;
    inodes[0].internal.file_perms = FS_READ | FS_WRITE | FS_EXECUTE;
    // we will set this the size of one directory entry
    inodes[0].internal.file_size = DIRECTORY_ENTRY_SIZE;
    inodes[0].internal.direct_data[0] = 0; // point to the first data block
    strcpy(inodes[0].internal.file_name, "root");
    size_t available_inode = 1; // next available inode is index 1

    // we will now claim a data block directly, namely the first data block and we will fill it with the following:
    // we will manually populate the data for this data block.
    dblock_bitmask[0] = 0x7F;

    // first copy the inode index
    // however we exploit how we calloced the memory so it is already set to 0
    // now we set the '.' directory
    dblocks[sizeof(inode_index_t)] = '.'; 

    // finally write the data when there is no errors
    fs->available_inode = inode_total > 1 ? available_inode : 0;
    fs->inodes = inodes;
    fs->inode_count = inode_total;
    fs->dblock_bitmask = dblock_bitmask;
    fs->dblocks = dblocks;
    fs->dblock_count = dblock_total;

    return SUCCESS;
}

void free_filesystem(filesystem_t *fs)
{
    if (!fs) return;
    free(fs->inodes);
    free(fs->dblock_bitmask);
    free(fs->dblocks);
}

size_t available_inodes(filesystem_t *fs)
{
    if (!fs) return 0;
    size_t count = 0;
    inode_index_t iter = fs->available_inode;
    while (iter != 0)
    {
        ++count;
        iter = fs->inodes[iter].next_free_inode;
    } 
    return count;
}

size_t available_dblocks(filesystem_t *fs)
{
    if (!fs) return 0;
    size_t count = 0;
    for (size_t i = 0; i < fs->dblock_count; ++i)
    {
        size_t block_idx = i / 8;
        size_t bit_idx = i % 8;
        if (fs->dblock_bitmask[block_idx] & (1 << (7 - bit_idx))) ++count;
    }
    return count;
}

fs_retcode_t claim_available_inode(filesystem_t *fs, inode_index_t *index)
{
    if (!fs || !index) return INVALID_INPUT;

    inode_index_t idx = fs->available_inode;
    if (!idx) return INODE_UNAVAILABLE;
    fs->available_inode = fs->inodes[idx].next_free_inode;
    *index = idx;
    return SUCCESS;
}

fs_retcode_t claim_available_dblock(filesystem_t *fs, dblock_index_t *index)
{
    if (!fs || !index) return INVALID_INPUT;

    for (size_t i = 0; i < fs->dblock_count; ++i)
    {
        size_t block_idx = i / 8;
        size_t bit_idx = i % 8;
        // check if dblock is available via the mask
        if (fs->dblock_bitmask[block_idx] & (1 << (7 - bit_idx)))
        {
            // claim the data block
            *index = i;
            mark_dblock_as_used(fs->dblock_bitmask, i);
            return SUCCESS;
        }
    }
    return DBLOCK_UNAVAILABLE;
}

fs_retcode_t release_inode(filesystem_t *fs, inode_t *inode)
{
    if (!fs || !inode) return INVALID_INPUT;

    // determine if inode is within the inode list. if not error
    // if (inode < fs->inodes || inode >= fs->inodes + fs->inode_count) return INVALID_INPUT;
    // root inode cannot be released
    if (inode == &fs->inodes[0]) return INVALID_INPUT;
    
    // add inode to the free "list"
    inode->next_free_inode = fs->available_inode;
    fs->available_inode = inode - fs->inodes; // inode - fs->inodes is index of inode

    return SUCCESS;
}

fs_retcode_t release_dblock(filesystem_t *fs, byte *dblock)
{
    if (!fs || !dblock) return INVALID_INPUT;

    // determine the index of dblock in fs. then check if valid
    ptrdiff_t dblock_diff = dblock - fs->dblocks;
    if (dblock_diff % DATA_BLOCK_SIZE != 0) return INVALID_INPUT;
    ptrdiff_t dblock_idx = dblock_diff / DATA_BLOCK_SIZE;
    // if (dblock_idx < 0 || dblock_idx >= (long) fs->dblock_count) return INVALID_INPUT;

    // enable bit in the bitmask marking availablity
    mark_dblock_as_unused(fs->dblock_bitmask, dblock_idx);

    return SUCCESS;
}
