#include "filesys.h"

#include <string.h>
#include <assert.h>

#include "utility.h"
#include "debug.h"

#define INDIRECT_DBLOCK_INDEX_COUNT (DATA_BLOCK_SIZE / sizeof(dblock_index_t) - 1)
#define INDIRECT_DBLOCK_MAX_DATA_SIZE ( DATA_BLOCK_SIZE * INDIRECT_DBLOCK_INDEX_COUNT )

#define NEXT_INDIRECT_INDEX_OFFSET (DATA_BLOCK_SIZE - sizeof(dblock_index_t))

// ----------------------- UTILITY FUNCTION ----------------------- //

size_t fill_direct_nodes(filesystem_t *fs, inode_t *inode, void *data, size_t n) {
    size_t written = 0;
    size_t used_dblocks = inode->internal.file_size / 64;
    for(size_t i = used_dblocks; i < 4 && written < n; i++) {
        if(inode->internal.direct_data[i] == 0) {
            dblock_index_t new_block;
            claim_available_dblock(fs, &new_block);
            inode->internal.direct_data[i] = new_block;
       }
       
        byte *block = fs->dblocks + (inode->internal.direct_data[i] * 64);
        size_t start = (i == used_dblocks) ? (inode->internal.file_size % 64) : 0;
        size_t write = (n - written < (64 - start)) ? (n - written) : (64 - start);
       
        memcpy(block + start, ((byte*)data) + written, write);
        written += write;
        inode->internal.file_size += write;
    }
    return written;
}

size_t fill_indirect_nodes(filesystem_t *fs, inode_t *inode, void *data, size_t n, size_t start){
    size_t written = 0;
   
    if (inode->internal.indirect_dblock == 0) {
        dblock_index_t new_iblock;
        claim_available_dblock(fs, &new_iblock);
        inode->internal.indirect_dblock = new_iblock;
    }
    while(written < n) {
        size_t used_dblocks = (inode->internal.file_size / 64) - 4;
        size_t iblock_num = used_dblocks / 15;
        size_t blocks_in_current_iblock = used_dblocks % 15;

        dblock_index_t current = inode->internal.indirect_dblock;
        for(size_t i = 0; i < iblock_num; i++){
            byte *block = fs->dblocks + (current * 64);
            dblock_index_t *dpointers = cast_dblock_ptr(block);
            if(dpointers[15] == 0) {
                dblock_index_t new_iblock;
                claim_available_dblock(fs, &new_iblock);
                dpointers[15] = new_iblock;
            }
            current = dpointers[15];
        }

        byte *block = fs->dblocks + (current * 64);
        dblock_index_t *index = cast_dblock_ptr(block);

        if(index[blocks_in_current_iblock] == 0) {
            dblock_index_t new_block;
            claim_available_dblock(fs, &new_block);
            index[blocks_in_current_iblock] = new_block;
        }
        byte *data_block = fs->dblocks + (index[blocks_in_current_iblock] * 64);

        size_t offset = inode->internal.file_size % 64;
        size_t write = (n - written < (64 - offset)) ? (n - written) : (64 - offset);
        
        memcpy(data_block + offset, ((byte*)data) + written + start, write);
        written += write;
        inode->internal.file_size += write;
   }
   return written;
}

void read_and_copy_byte(filesystem_t *fs, inode_t *inode, size_t offset, byte *buffer) {
    size_t block_num = offset / 64;
    size_t offset_in_block = offset % 64;
   
    if (block_num < 4) {
        dblock_index_t dblock = inode->internal.direct_data[block_num];
        byte *val = fs->dblocks + (dblock * 64) + offset_in_block;
        *buffer = *val;
    }
    else {
        size_t used_dblocks = block_num - 4;
        size_t iblock_num = used_dblocks / 15;
        size_t blocks_in_current_iblock = used_dblocks % 15;

        dblock_index_t current = inode->internal.indirect_dblock;

        for (size_t i = 0; i < iblock_num; i++) {
            dblock_index_t *dpointers = cast_dblock_ptr(fs->dblocks + current * 64);
            current = dpointers[15];  
        }
        dblock_index_t *index = cast_dblock_ptr(fs->dblocks + current * 64);
        dblock_index_t data_block = index[blocks_in_current_iblock];
        byte *val = fs->dblocks + (data_block * 64) + offset_in_block;
        *buffer = *val;
    }
}

void modify_byte(filesystem_t *fs, inode_t *inode, size_t offset, byte *buffer) {
    size_t block_num = offset / 64;
    size_t offset_in_block = offset % 64;
   
    if (block_num < 4) {
        dblock_index_t dblock = inode->internal.direct_data[block_num];
        byte *val = fs->dblocks + (dblock * 64) + offset_in_block;
        *val = *((byte*)buffer);
    }
    else {
        size_t used_dblocks = block_num - 4;
        size_t iblock_num = used_dblocks / 15;
        size_t blocks_in_current_iblock = used_dblocks % 15;

        dblock_index_t current = inode->internal.indirect_dblock;

        for (size_t i = 0; i < iblock_num; i++) {
            dblock_index_t *dpointers = cast_dblock_ptr(fs->dblocks + current * 64);
            current = dpointers[15];  
        }
        dblock_index_t *index = cast_dblock_ptr(fs->dblocks + current * 64);
        dblock_index_t data_block = index[blocks_in_current_iblock];
        byte *val = fs->dblocks + (data_block * 64) + offset_in_block;
        *val = *((byte*)buffer);
    }
}

void free_direct_blocks(filesystem_t *fs, inode_t *inode, size_t n){
    
    if(inode->internal.file_size - n == 0) {
        for(size_t i = 0; i < 4; i++) {
            dblock_index_t block_index = inode->internal.direct_data[i];
            if (block_index != 0) {
            byte *block = fs->dblocks + block_index * 64;
            release_dblock(fs, block);
            }
        }
    }

    while(n > 0) {
        size_t last_byte_index = (inode->internal.file_size - 1) / 64;
        dblock_index_t block_index = inode->internal.direct_data[last_byte_index];
        byte *block = fs->dblocks + block_index * 64;
        size_t offset = (inode->internal.file_size - last_byte_index * 64) - 1;

        for(int i = offset; i >= 0 &&  n > 0; i--, n--) {
            inode->internal.file_size -= 1;
        }
        if(n > 0) {
            release_dblock(fs, block);
        }
    }
}

void free_indirect_blocks(filesystem_t *fs, inode_t *inode, size_t n){
    
    size_t num_iblocks = calculate_index_dblock_amount(inode->internal.file_size);
    size_t num_dblocks = calculate_necessary_dblock_amount(inode->internal.file_size) - 4 - num_iblocks;
    size_t num_dblocks_in_last_iblock = (num_dblocks % 15 == 0 && num_iblocks > 0) ? 15 : num_dblocks % 15;

    while(n > 0) {
        dblock_index_t current = inode->internal.indirect_dblock;
        if(num_iblocks != 0) {
            for(size_t i = 0; i < num_iblocks - 1; i++){
                byte *block = fs->dblocks + (current * 64);
                dblock_index_t *dpointers = cast_dblock_ptr(block);
                current = dpointers[15];
            }
        }

        byte *block = fs->dblocks + (current * 64);
        dblock_index_t *index = cast_dblock_ptr(block);

        for(int i = num_dblocks_in_last_iblock - 1; i >= 0 && n > 0; i--) {
            byte *data_block = fs->dblocks + (index[i] * 64);
            size_t offset = (i == (int)num_dblocks_in_last_iblock) ? (inode->internal.file_size - (((num_iblocks - 1) * 960) + (256) + ((num_dblocks_in_last_iblock - 1) * 64))) - 1 : 63;
            size_t lower_bound = 0;
            if(n < 64) {
                lower_bound = 64 - n;
            }
            for(int j = offset + 1; j > (int)lower_bound && n > 0; j--, n--) {
                inode->internal.file_size -= 1;
            }
            if((int)n > 0 || lower_bound == 0) {
                release_dblock(fs, data_block);
                num_dblocks -= 1;
            }
        }
            if(n > 0 || inode->internal.file_size == 256) {
                release_dblock(fs, block);
            }
        num_dblocks_in_last_iblock = 15;
        num_iblocks  = (num_iblocks > 1) ? num_iblocks - 1: 0;
        }

    if(inode->internal.file_size == 256) {
        dblock_index_t current = inode->internal.indirect_dblock;
        byte *block = fs->dblocks + current * 64;
        release_dblock(fs, block);
    }
}
    
// ----------------------- CORE FUNCTION ----------------------- //

fs_retcode_t inode_write_data(filesystem_t *fs, inode_t *inode, void *data, size_t n)
{
    if(fs == NULL || inode == NULL) {
        return INVALID_INPUT;
    }

    size_t num_dblocks_needed = calculate_necessary_dblock_amount(inode->internal.file_size + n) - calculate_necessary_dblock_amount(inode->internal.file_size);
    if(available_dblocks(fs) < num_dblocks_needed){
        return INSUFFICIENT_DBLOCKS;
    }
    
    size_t remaining_dblocks_bytes = (inode->internal.file_size < 256) ? (256 - inode->internal.file_size) : 0;
    size_t bytes_to_write = (remaining_dblocks_bytes < n) ? remaining_dblocks_bytes : n;

    if(remaining_dblocks_bytes != 0) {
        size_t written = fill_direct_nodes(fs, inode, data, bytes_to_write);
        if(written < n) {
            fill_indirect_nodes(fs, inode, data, n - written, written);
        }
    }
    else {
        fill_indirect_nodes(fs, inode, data, n, inode->internal.file_size);
    }
    return SUCCESS;
}

fs_retcode_t inode_read_data(filesystem_t *fs, inode_t *inode, size_t offset, void *buffer, size_t n, size_t *bytes_read)
{

    printf("\nOFFSET: %zu, BYTES_TO_READ: %zu\n", offset, n);
    if(fs == NULL || inode == NULL || bytes_read == NULL) {
        return INVALID_INPUT;
    }

    if(offset >= inode->internal.file_size) {
        *bytes_read = 0;
        return SUCCESS;
    }
    size_t bytes_to_read = (offset + n < inode->internal.file_size) ? n : inode->internal.file_size - offset;

    for(size_t i = 0; i < bytes_to_read; i++) {
        read_and_copy_byte(fs, inode, offset + i, buffer + i);
    }
    *bytes_read = bytes_to_read;
    return SUCCESS;
}

fs_retcode_t inode_modify_data(filesystem_t *fs, inode_t *inode, size_t offset, void *buffer, size_t n)
{
    if(fs == NULL || inode == NULL || offset > inode->internal.file_size) {
        return INVALID_INPUT;
    }

    size_t num_dblocks_needed = calculate_necessary_dblock_amount(inode->internal.file_size + n) - calculate_necessary_dblock_amount(inode->internal.file_size);
    if(available_dblocks(fs) < num_dblocks_needed){
        return INSUFFICIENT_DBLOCKS;
    }

    size_t modify_existing_num = (offset + n <= inode->internal.file_size) ? n : inode->internal.file_size - offset;

    for(size_t i = 0; i < modify_existing_num; i++) {
        modify_byte(fs, inode, offset + i, buffer + i);
    }

    size_t remaining_bytes = n - modify_existing_num;
    inode_write_data(fs, inode, buffer + modify_existing_num, remaining_bytes);

    return SUCCESS;
}

fs_retcode_t inode_shrink_data(filesystem_t *fs, inode_t *inode, size_t new_size)
{
    if(fs == NULL || inode == NULL || new_size > inode->internal.file_size) {
        return INVALID_INPUT;
    }
    size_t bytes_to_remove = inode->internal.file_size - new_size;
    if(inode->internal.file_size > 256) {
        size_t bytes_to_remove_indirect = (inode->internal.file_size - bytes_to_remove > 256) ? bytes_to_remove : inode->internal.file_size  - (256 - new_size);
        free_indirect_blocks(fs, inode, bytes_to_remove_indirect);
        bytes_to_remove -= bytes_to_remove_indirect;
    }

    if(bytes_to_remove > 0){
        free_direct_blocks(fs, inode, bytes_to_remove);
    } 

    inode->internal.file_size = new_size;
    return SUCCESS;
    
    //check to see if inputs are in valid range

    //Calculate how many blocks to remove

    //helper function to free all indirect blocks

    //remove the remaining direct dblocks

    //update filesize and return
}

fs_retcode_t inode_release_data(filesystem_t *fs, inode_t *inode)
{
   if(fs == NULL || inode == NULL) {
        return INVALID_INPUT;
    }

    for(size_t i = 0; i < 4; i++) {
        dblock_index_t block_index = inode->internal.direct_data[i];
        if (block_index != 0) {
        byte *block = fs->dblocks + block_index * 64;
        release_dblock(fs, block);
        }
    }

    size_t num_iblocks = calculate_index_dblock_amount(inode->internal.file_size);
    size_t num_dblocks = calculate_necessary_dblock_amount(inode->internal.file_size) - 4 - num_iblocks;
    size_t num_dblocks_in_last_iblock = (num_dblocks % 15 == 0 && num_iblocks > 0) ? 15 : num_dblocks % 15;

    while(num_iblocks > 0) {
        dblock_index_t current = inode->internal.indirect_dblock;
        for(size_t i = 0; i < num_iblocks - 1; i++){
            byte *block = fs->dblocks + (current * 64);
            dblock_index_t *dpointers = cast_dblock_ptr(block);
            current = dpointers[15];
    }
        byte *block = fs->dblocks + (current * 64);
        dblock_index_t *index = cast_dblock_ptr(block);
        for(size_t i = 0; i < num_dblocks_in_last_iblock; i++) {
            byte *data_block = fs->dblocks + (index[i] * 64);
            release_dblock(fs, data_block);
        }
        release_dblock(fs, block);

        num_iblocks -= 1;
        num_dblocks -= num_dblocks_in_last_iblock;
        num_dblocks_in_last_iblock = (num_dblocks % 15 == 0 && num_iblocks > 0) ? 15 : num_dblocks % 15;
        }
    inode->internal.file_size = 0;
    return SUCCESS;
}
