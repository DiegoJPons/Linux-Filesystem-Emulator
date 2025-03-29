#include "test_util.hpp"

using INodeWriteDataSuite = fs_internal_test;

// test case to see if invalid inputs are handled correctly
TEST_F(INodeWriteDataSuite, InvalidInput)
{
    EXPECT_EQ( inode_write_data(NULL, NULL, NULL, 0), INVALID_INPUT );

    filesystem_t fs;
    new_filesystem(&fs, 1, 1);
    EXPECT_EQ( inode_write_data(&fs, NULL, NULL, 0), INVALID_INPUT );

    free_filesystem(&fs);
}

// test cases on an full small file to see if the insufficient block error is returned appropriately
// also checkes that no change is done to the file system in case of this error.
TEST_F(INodeWriteDataSuite, InsufficientBlock0)
{
    filesystem_t fs;
    load_fs(INPUT "tiny_full.bin", fs);

    inode_t *root = &fs.inodes[0];
    char test_message[64] = { 0 };
    EXPECT_EQ( inode_write_data(&fs, root, test_message, std::size(test_message)), INSUFFICIENT_DBLOCKS );

    check_fs(INPUT "tiny_full.bin", fs);
    free_filesystem(&fs);
}

// ditto except with a medium dblock exhausted filesystem
TEST_F(INodeWriteDataSuite, InsufficientBlock1)
{
    filesystem_t fs;
    load_fs(INPUT "full_medium.bin", fs);

    inode_t *root = &fs.inodes[0];
    char test_message[64] = { 0 };
    EXPECT_EQ( inode_write_data(&fs, root, test_message, std::size(test_message)), INSUFFICIENT_DBLOCKS );

    check_fs(INPUT "full_medium.bin", fs);
    free_filesystem(&fs);
}

// checks if the function writes to direct data
// this one writes to an empty file
TEST_F(INodeWriteDataSuite, WriteDirect0)
{
    filesystem_t fs;
    load_fs(INPUT "large.bin", fs);

    inode_t *empty_inode = &fs.inodes[6];
    char test_message[20];
    memset(test_message, 0x20, std::size(test_message));
    EXPECT_EQ( inode_write_data(&fs, empty_inode, test_message, std::size(test_message)), SUCCESS );

    check_fs(OUTPUT "WriteDirect0.bin", fs);
    free_filesystem(&fs);
}

// checks if the function writes to direct data with data block allocation
TEST_F(INodeWriteDataSuite, WriteDirect1)
{
    filesystem_t fs;
    load_fs(INPUT "large.bin", fs);

    inode_t *hi_file = &fs.inodes[1];
    char test_message[128]; // should allocate two data blocks
    memset(test_message, 0x20, std::size(test_message));
    EXPECT_EQ( inode_write_data(&fs, hi_file, test_message, std::size(test_message)), SUCCESS );

    check_fs(OUTPUT "WriteDirect1.bin", fs);
    free_filesystem(&fs);
}

// checks if the function writes to indirect dblocks 
TEST_F(INodeWriteDataSuite, WriteIndirect0)
{
    filesystem_t fs;
    load_fs(INPUT "large.bin", fs);
    
    inode_t *zero_file = &fs.inodes[2];
    char test_message[130]; // should allocate 3 data blocks, 1 index data block
    memset(test_message, 0x20, std::size(test_message));
    EXPECT_EQ( inode_write_data(&fs, zero_file, test_message, std::size(test_message)), SUCCESS );

    check_fs(OUTPUT "WriteIndirect0.bin", fs);
    free_filesystem(&fs);
}

// checks large write to indirect dblocks
TEST_F(INodeWriteDataSuite, WriteIndirect1)
{
    filesystem_t fs;
    load_fs(INPUT "large.bin", fs);
    
    inode_t *large_file = &fs.inodes[5];
    char test_message[1024]; // should allocate 3 data blocks, 1 index data block
    memset(test_message, 0x20, std::size(test_message));
    EXPECT_EQ( inode_write_data(&fs, large_file, test_message, std::size(test_message)), SUCCESS );

    check_fs(OUTPUT "WriteIndirect1.bin", fs);
    free_filesystem(&fs);
}

// checks writes from direct dblocks to indirect dblocks
TEST_F(INodeWriteDataSuite, WriteDirectIndirect)
{
    filesystem_t fs;
    load_fs(INPUT "large.bin", fs);
    
    inode_t *sys_file = &fs.inodes[4];
    char test_message[1024]; // should allocate 3 data blocks, 1 index data block
    memset(test_message, 0x20, std::size(test_message));
    EXPECT_EQ( inode_write_data(&fs, sys_file, test_message, std::size(test_message)), SUCCESS );

    check_fs(OUTPUT "WriteDirectIndirect.bin", fs);
    free_filesystem(&fs);
}