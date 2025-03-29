#include "test_util.hpp"

using INodeModifyDataSuite = fs_internal_test;

// check for basic invalid inputs
TEST_F(INodeModifyDataSuite, InvalidInput)
{
    filesystem_t fs;
    new_filesystem(&fs, 1, 1);
    inode_t *root = &fs.inodes[0];

    ASSERT_EQ( inode_modify_data(NULL, root, 0, NULL, 0), INVALID_INPUT );
    ASSERT_EQ( inode_modify_data(&fs, NULL, 0, NULL, 0), INVALID_INPUT );

    free_filesystem(&fs);
}

// test insufficient data block cases
TEST_F(INodeModifyDataSuite, InsufficientDataBlock0)
{
    filesystem_t fs;
    load_fs(INPUT "tiny_full.bin", fs);

    inode_t *root = &fs.inodes[0];
    char test_message[64] = { 0 };
    ASSERT_EQ( inode_modify_data(&fs, root, root->internal.file_size, test_message, std::size(test_message)), INSUFFICIENT_DBLOCKS );

    check_fs(INPUT "tiny_full.bin", fs);

    free_filesystem(&fs);
}

TEST_F(INodeModifyDataSuite, InsufficientDataBlock1)
{
    filesystem_t fs;
    load_fs(INPUT "full_medium.bin", fs);

    inode_t *book = &fs.inodes[4];
    char test_message[64] = { 0 };
    ASSERT_EQ( inode_modify_data(&fs, book, book->internal.file_size - 32, test_message, std::size(test_message)), INSUFFICIENT_DBLOCKS );

    check_fs(INPUT "full_medium.bin", fs);

    free_filesystem(&fs);
}

TEST_F(INodeModifyDataSuite, OffsetOutOfRange)
{
    filesystem_t fs;
    load_fs(INPUT "medium_text.bin", fs);

    inode_t *hi_file = &fs.inodes[1];
    char test_message[64] = { 0 };
    ASSERT_EQ( inode_modify_data(&fs, hi_file, hi_file->internal.file_size + 32, test_message, std::size(test_message)), INVALID_INPUT );

    check_fs(INPUT "medium_text.bin", fs);

    free_filesystem(&fs);
}

// test modify the direct data blocks
// does not increase the size of the file
TEST_F(INodeModifyDataSuite, ModifyDirect0)
{
    filesystem_t fs;
    load_fs(INPUT "medium_text.bin", fs);

    inode_t *hi_file = &fs.inodes[1];
    char test_message[80] = { 0 };
    memset(test_message, 0x20, std::size(test_message));
    ASSERT_EQ( inode_modify_data(&fs, hi_file, 20, test_message, std::size(test_message)), SUCCESS );

    check_fs(OUTPUT "ModifyDirect0.bin", fs);

    free_filesystem(&fs);
}

// test modifying the indirect data blocks
// does not increase the size of the file
TEST_F(INodeModifyDataSuite, ModifyIndirect0)
{
    filesystem_t fs;
    load_fs(INPUT "medium_text.bin", fs);

    inode_t *hi_file = &fs.inodes[1];
    char test_message[80] = { 0 };
    memset(test_message, 0x20, std::size(test_message));
    ASSERT_EQ( inode_modify_data(&fs, hi_file, 280, test_message, std::size(test_message)), SUCCESS );

    check_fs(OUTPUT "ModifyIndirect0.bin", fs);

    free_filesystem(&fs);
}

// test modifying the direct and indirect data blocks
// does not increase the size of the file
TEST_F(INodeModifyDataSuite, ModifyDirectIndirect0)
{
    filesystem_t fs;
    load_fs(INPUT "medium_text.bin", fs);

    inode_t *hi_file = &fs.inodes[1];
    char test_message[80] = { 0 };
    memset(test_message, 0x20, std::size(test_message));
    ASSERT_EQ( inode_modify_data(&fs, hi_file, 240, test_message, std::size(test_message)), SUCCESS );

    check_fs(OUTPUT "ModifyDirectIndirect0.bin", fs);

    free_filesystem(&fs);
}

// test modifying the direct data blocks
// increases size of the file
TEST_F(INodeModifyDataSuite, ModifyDirect1)
{
    constexpr std::size_t offset = 60;
    constexpr std::size_t arr_size = 120;
    constexpr std::size_t inode_idx = 1;

    filesystem_t fs;
    load_fs(INPUT "large.bin", fs);

    inode_t *hi_file = &fs.inodes[inode_idx];
    char test_message[arr_size] = { 0 };
    memset(test_message, 0x20, std::size(test_message));
    ASSERT_EQ( inode_modify_data(&fs, hi_file, offset, test_message, std::size(test_message)), SUCCESS );
    ASSERT_EQ( hi_file->internal.file_size, offset + std::size(test_message) );

    check_fs(OUTPUT "ModifyDirect1.bin", fs);

    free_filesystem(&fs);
}

// test modifying the indirect data blocks
// increases the size of the file
TEST_F(INodeModifyDataSuite, ModifyIndirect1)
{
    constexpr std::size_t offset = 2000;
    constexpr std::size_t arr_size = 800;
    constexpr std::size_t inode_idx = 5;

    filesystem_t fs;
    load_fs(INPUT "large.bin", fs);

    inode_t *large_file = &fs.inodes[inode_idx];
    char test_message[arr_size] = { 0 };
    memset(test_message, 0x20, std::size(test_message));
    ASSERT_EQ( inode_modify_data(&fs, large_file, offset, test_message, std::size(test_message)), SUCCESS );
    ASSERT_EQ( large_file->internal.file_size, offset + std::size(test_message) );

    check_fs(OUTPUT "ModifyIndirect1.bin", fs);

    free_filesystem(&fs);
}

// test modifying the direct and indirect blocks
// increase the size of the file
TEST_F(INodeModifyDataSuite, ModifyDirectIndirect1)
{
    constexpr std::size_t offset = 192;
    constexpr std::size_t arr_size = 200;
    constexpr std::size_t inode_idx = 2;

    filesystem_t fs;
    load_fs(INPUT "large.bin", fs);

    inode_t *inode = &fs.inodes[inode_idx];
    char test_message[arr_size] = { 0 };
    memset(test_message, 0x0B, std::size(test_message));
    ASSERT_EQ( inode_modify_data(&fs, inode, offset, test_message, std::size(test_message)), SUCCESS );
    ASSERT_EQ( inode->internal.file_size, offset + std::size(test_message) );

    check_fs(OUTPUT "ModifyDirectIndirect1.bin", fs);

    free_filesystem(&fs);
}