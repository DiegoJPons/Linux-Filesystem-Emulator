#include "test_util.hpp"

using ReleaseINodeSuite = fs_internal_test;

// test invalid input
TEST_F(ReleaseINodeSuite, InvalidInput)
{
    constexpr fs_retcode_t expected_retcode = INVALID_INPUT;

    filesystem_t fs;
    inode_t inode;
    auto output_retcode0 = release_inode(NULL, &inode);
    auto output_retcode1 = release_inode(&fs, NULL);

    ASSERT_EQ(expected_retcode, output_retcode0) << "Return values do not match for fs = NULL case!";
    ASSERT_EQ(expected_retcode, output_retcode1) << "Return values do not match for inode = NULL test case!";
}

TEST_F(ReleaseINodeSuite, AttemptReleaseRootINode)
{
    constexpr fs_retcode_t expected_retcode = INVALID_INPUT;
    constexpr inode_index_t inode_to_free = 0;
    
    filesystem_t fs;
    load_fs(INPUT "medium_tombstone.bin", fs);

    auto output_retcode = release_inode(&fs, &fs.inodes[inode_to_free]);

    ASSERT_EQ(output_retcode, expected_retcode) << "Return value do not match!";

    check_fs(INPUT "medium_tombstone.bin", fs);
    free_filesystem(&fs);
}

TEST_F(ReleaseINodeSuite, SimpleReleaseINode0)
{
    constexpr fs_retcode_t expected_retcode = SUCCESS;
    constexpr inode_index_t inode_to_free = 4;
    
    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);

    auto output_retcode = release_inode(&fs, &fs.inodes[inode_to_free]);

    ASSERT_EQ(output_retcode, expected_retcode) << "Return value do not match!";

    check_fs(OUTPUT "SimpleReleaseInode0.bin", fs);
    free_filesystem(&fs);
}

// complex, keep allocated until empty
TEST_F(ReleaseINodeSuite, ComplexReleaseINode0)
{   
    inode_index_t inodes_to_release_list[] = { 
        30, 29, 28, 26, 24, 23, 22, 20,
        19, 16, 14, 13, 10, 9, 8, 7,
        6, 5, 3, 2, 1
    };

    filesystem_t fs;
    load_fs(INPUT "half_random_inode_fragmented.bin", fs);

    for (auto&& idx : inodes_to_release_list)
    {
        fs_retcode_t expected_retcode = SUCCESS;
        fs_retcode_t output_retcode = release_inode(&fs, &fs.inodes[idx]);
        ASSERT_EQ(output_retcode, expected_retcode) << "Return value do not match for releasing inode index " << idx << "!";
    }

    check_fs(OUTPUT "ComplexReleaseINode0.bin", fs);
    free_filesystem(&fs);
}