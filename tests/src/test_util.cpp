#include "test_util.hpp"

extern "C"
{
#include "filesys.h"
}

#define DBLOCK_MASK_SIZE(blk_count) (((blk_count) + 7) / (sizeof(byte) * 8))

void compare_fs_files(char *output_buf, size_t output_size, char *expected_buf, size_t expected_size)
{
    // start by comparing file sizes
    ASSERT_EQ(output_size, expected_size) << "Incorrect file sizes.";

    // now compare the initial bytes that describe the structure of the file system
    size_t expected_inode_count, output_inode_count;
    size_t expected_dblock_count, output_dblock_count;
    inode_index_t expected_available_inode_index, output_available_inode_index;

    size_t index = 0;
    
    // compare the inode counts
    memcpy(&output_inode_count, &output_buf[index], sizeof(size_t));
    memcpy(&expected_inode_count, &expected_buf[index], sizeof(size_t));
    index += sizeof(size_t);
    ASSERT_EQ(output_inode_count, expected_inode_count) << "Incorrect inode count in filesystem.";

    // compare the next available inode
    memcpy(&output_available_inode_index, &output_buf[index], sizeof(inode_index_t));
    memcpy(&expected_available_inode_index, &expected_buf[index], sizeof(inode_index_t));
    index += sizeof(inode_index_t);
    ASSERT_EQ(output_available_inode_index, expected_available_inode_index) << "Incorrect first available inode index.";

    // compare the dblock
    memcpy(&output_dblock_count, &output_buf[index], sizeof(size_t));
    memcpy(&expected_dblock_count, &expected_buf[index], sizeof(size_t));
    index += sizeof(size_t);
    ASSERT_EQ(output_dblock_count, expected_dblock_count) << "Incorrect dblock count in filesystem.";

    // now compare inodes
    for (size_t inode_idx = 0; inode_idx < expected_inode_count; ++inode_idx)
    {
        for (size_t byte_idx = 0; byte_idx < sizeof(inode_index_t); ++byte_idx)
        {
            ASSERT_EQ(output_buf[index], expected_buf[index]) 
                << "Incorrect value for byte " << byte_idx << " at inode index " << inode_idx;
            ++index;
        }
    }    

    // now compare bitmask
    size_t bitmask_size = DBLOCK_MASK_SIZE(expected_dblock_count);
    for (size_t bitmask_idx = 0; bitmask_idx < bitmask_size; ++bitmask_idx)
    {
        ASSERT_EQ(output_buf[index], expected_buf[index]) 
            << "Incorrect value for byte " << bitmask_idx << " of the data block bit mask";
        ++index; 
    }

    // now compare the data blocks
    for (size_t dblock_idx = 0; dblock_idx < expected_dblock_count; ++dblock_idx)
    {
        for (size_t dblock_byte = 0; dblock_byte < DATA_BLOCK_SIZE; ++dblock_byte)
        {
            ASSERT_EQ(output_buf[index], expected_buf[index])
                << "Incorrect value for byte " << dblock_byte << " at data block index " << dblock_idx;
            ++index;
        }
    }
}