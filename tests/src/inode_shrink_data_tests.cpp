#include "test_util.hpp"

using INodeShrinkDataSuite = fs_internal_test;

// test for basic invalid input
TEST_F(INodeShrinkDataSuite, InvalidInput)
{
    filesystem_t fs;
    new_filesystem(&fs, 1, 1);
    inode_t *root = &fs.inodes[0];

    ASSERT_EQ( inode_shrink_data(&fs, NULL, 0), INVALID_INPUT );
    ASSERT_EQ( inode_shrink_data(NULL, root, 0), INVALID_INPUT );

    free_filesystem(&fs);
}

// test for invalid input with invalid new file size
TEST_F(INodeShrinkDataSuite, NewSizeInvalidInput)
{
    constexpr std::size_t inode_idx = 1;
    constexpr std::size_t new_size = 128;

    filesystem_t fs;
    load_fs(INPUT "large.bin", fs);

    inode_t *inode = &fs.inodes[inode_idx];
    ASSERT_EQ( inode_shrink_data(&fs, inode, new_size), INVALID_INPUT );

    check_fs(INPUT "large.bin", fs);
    free_filesystem(&fs);
}

// test for shrinking direct data blocks only
TEST_F(INodeShrinkDataSuite, ShrinkDirect0)
{
    constexpr std::size_t inode_idx = 1;
    constexpr std::size_t new_size = 32;

    filesystem_t fs;
    load_fs(INPUT "large.bin", fs);

    inode_t *inode = &fs.inodes[inode_idx];
    ASSERT_EQ( inode_shrink_data(&fs, inode, new_size), SUCCESS );
    ASSERT_EQ( inode->internal.file_size, new_size );

    check_fs(OUTPUT "ShrinkDirect0.bin", fs);
    free_filesystem(&fs);
}

// test for shrinking indirect data blocks only
// does not deallocate an index data block
TEST_F(INodeShrinkDataSuite, ShrinkIndirect0)
{
    constexpr std::size_t inode_idx = 5;
    constexpr std::size_t new_size = 1792;

    filesystem_t fs;
    load_fs(INPUT "large.bin", fs);

    inode_t *inode = &fs.inodes[inode_idx];
    ASSERT_EQ( inode_shrink_data(&fs, inode, new_size), SUCCESS );
    ASSERT_EQ( inode->internal.file_size, new_size );

    check_fs(OUTPUT "ShrinkIndirect0.bin", fs);
    free_filesystem(&fs);
}

// test for shrinking indirect data blocks
// does deallocate index data block
TEST_F(INodeShrinkDataSuite, ShrinkIndirect1)
{
    constexpr std::size_t inode_idx = 5;
    constexpr std::size_t new_size = 280;

    filesystem_t fs;
    load_fs(INPUT "large.bin", fs);

    inode_t *inode = &fs.inodes[inode_idx];
    ASSERT_EQ( inode_shrink_data(&fs, inode, new_size), SUCCESS );
    ASSERT_EQ( inode->internal.file_size, new_size );

    check_fs(OUTPUT "ShrinkIndirect1.bin", fs);
    free_filesystem(&fs);
}

// test for shrinking indirect and direct data blocks
// does deallocate index data block
TEST_F(INodeShrinkDataSuite, ShrinkDirectIndirect0)
{
    constexpr std::size_t inode_idx = 5;
    constexpr std::size_t new_size = 20;

    filesystem_t fs;
    load_fs(INPUT "large.bin", fs);

    inode_t *inode = &fs.inodes[inode_idx];
    ASSERT_EQ( inode_shrink_data(&fs, inode, new_size), SUCCESS );
    ASSERT_EQ( inode->internal.file_size, new_size );

    check_fs(OUTPUT "ShrinkDirectIndirect0.bin", fs);
    free_filesystem(&fs);
}

// test for shrinking indirect and direct data blocks
// complete deallocation
TEST_F(INodeShrinkDataSuite, ShrinkComplete0)
{
    constexpr std::size_t inode_idx = 5;
    constexpr std::size_t new_size = 0;

    filesystem_t fs;
    load_fs(INPUT "large.bin", fs);

    inode_t *inode = &fs.inodes[inode_idx];
    ASSERT_EQ( inode_shrink_data(&fs, inode, new_size), SUCCESS );
    ASSERT_EQ( inode->internal.file_size, new_size );

    check_fs(OUTPUT "ShrinkComplete0.bin", fs);
    free_filesystem(&fs);
}

// test for shrinking indirect and direct data blocks
// complete deallocation
TEST_F(INodeShrinkDataSuite, ShrinkComplete1)
{
    constexpr std::size_t inode_idx = 1;
    constexpr std::size_t new_size = 0;

    filesystem_t fs;
    load_fs(INPUT "large.bin", fs);

    inode_t *inode = &fs.inodes[inode_idx];
    ASSERT_EQ( inode_shrink_data(&fs, inode, new_size), SUCCESS );
    ASSERT_EQ( inode->internal.file_size, new_size );

    check_fs(OUTPUT "ShrinkComplete1.bin", fs);
    free_filesystem(&fs);
}

using INodeReleaseDataSuite = fs_internal_test;

// test for basic invalid input
TEST_F(INodeReleaseDataSuite, InvalidInput)
{
    filesystem_t fs;
    new_filesystem(&fs, 1, 1);
    inode_t *root = &fs.inodes[0];

    ASSERT_EQ( inode_release_data(&fs, NULL), INVALID_INPUT );
    ASSERT_EQ( inode_release_data(NULL, root), INVALID_INPUT );

    free_filesystem(&fs);
}

TEST_F(INodeReleaseDataSuite, Release0)
{
    constexpr std::size_t inode_idx = 5;

    filesystem_t fs;
    load_fs(INPUT "large.bin", fs);

    inode_t *inode = &fs.inodes[inode_idx];
    ASSERT_EQ( inode_release_data(&fs, inode), SUCCESS );
    ASSERT_EQ( inode->internal.file_size, 0 );

    check_fs(OUTPUT "ShrinkComplete0.bin", fs);
    free_filesystem(&fs);
}

TEST_F(INodeReleaseDataSuite, Release1)
{
    constexpr std::size_t inode_idx = 1;

    filesystem_t fs;
    load_fs(INPUT "large.bin", fs);

    inode_t *inode = &fs.inodes[inode_idx];
    ASSERT_EQ( inode_release_data(&fs, inode), SUCCESS );
    ASSERT_EQ( inode->internal.file_size, 0 );

    check_fs(OUTPUT "ShrinkComplete1.bin", fs);
    free_filesystem(&fs);
}