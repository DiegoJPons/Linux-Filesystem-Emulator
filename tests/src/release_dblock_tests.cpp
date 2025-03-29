#include "test_util.hpp"

using ReleaseDBlockSuite = fs_internal_test;

// test invalid input
TEST_F(ReleaseDBlockSuite, InvalidInput)
{
    constexpr fs_retcode_t expected_retcode = INVALID_INPUT;

    filesystem_t fs;
    byte dblock;
    auto output_retcode0 = release_dblock(NULL, &dblock);
    auto output_retcode1 = release_dblock(&fs, NULL);

    ASSERT_EQ(expected_retcode, output_retcode0) << "Return values do not match for fs = NULL case!";
    ASSERT_EQ(expected_retcode, output_retcode1) << "Return values do not match for inode = NULL test case!";
}

TEST_F(ReleaseDBlockSuite, SimpleReleaseDBlock0)
{
    constexpr fs_retcode_t expected_retcode = SUCCESS;
    constexpr dblock_index_t dblock_to_free = 4;
    
    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);

    auto output_retcode = release_dblock(&fs, &fs.dblocks[dblock_to_free * DATA_BLOCK_SIZE]);

    ASSERT_EQ(output_retcode, expected_retcode) << "Return value do not match!";

    check_fs(OUTPUT "SimpleReleaseDBlock0.bin", fs);
    free_filesystem(&fs);
}

// complex, keep allocated until empty
TEST_F(ReleaseDBlockSuite, ComplexReleaseDBlock0)
{   
    inode_index_t dblocks_to_release[] = { 
        27, 10, 23, 13, 30, 2
    };

    filesystem_t fs;
    load_fs(INPUT "empty_random_inode_fragmented.bin", fs);

    for (auto&& idx : dblocks_to_release)
    {
        fs_retcode_t expected_retcode = SUCCESS;
        fs_retcode_t output_retcode = release_dblock(&fs, &fs.dblocks[idx * DATA_BLOCK_SIZE]);
        ASSERT_EQ(output_retcode, expected_retcode) << "Return value do not match for releasing D-block index " << idx << "!";
    }

    check_fs(OUTPUT "ComplexReleaseDBlock0.bin", fs);
    free_filesystem(&fs);
}