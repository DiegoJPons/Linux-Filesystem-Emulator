#include "test_util.hpp"

using ClaimAvailableINodeSuite = fs_internal_test;

// test invalid input
TEST_F(ClaimAvailableINodeSuite, InvalidInput)
{
    constexpr fs_retcode_t expected_retcode = INVALID_INPUT;

    filesystem_t fs;
    inode_index_t idx;
    auto output_retcode0 = claim_available_inode(NULL, &idx);
    auto output_retcode1 = claim_available_inode(&fs, NULL);

    ASSERT_EQ(expected_retcode, output_retcode0) << "Return values do not match for fs = NULL case!";
    ASSERT_EQ(expected_retcode, output_retcode1) << "Return values do not match for index = NULL test case!";
}

TEST_F(ClaimAvailableINodeSuite, SimpleClaim0)
{
    constexpr fs_retcode_t expected_retcode = SUCCESS;
    constexpr inode_index_t expected_inode_index = 6;
    
    filesystem_t fs;
    load_fs(INPUT "medium_tombstone.bin", fs);

    inode_index_t output_inode_index = 0;
    auto output_retcode = claim_available_inode(&fs, &output_inode_index);

    ASSERT_EQ(output_retcode, expected_retcode) << "Return value do not match!";
    ASSERT_EQ(output_inode_index, expected_inode_index) << "INode index value do not match!";

    check_fs(OUTPUT "SimpleClaim0.bin", fs);
    free_filesystem(&fs);
}

TEST_F(ClaimAvailableINodeSuite, SimpleClaim1)
{
    constexpr fs_retcode_t expected_retcode = SUCCESS;
    constexpr inode_index_t expected_inode_index = 10;
    
    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);

    inode_index_t output_inode_index = 0;
    auto output_retcode = claim_available_inode(&fs, &output_inode_index);

    ASSERT_EQ(output_retcode, expected_retcode) << "Return value do not match!";
    ASSERT_EQ(output_inode_index, expected_inode_index) << "INode index value do not match!";

    check_fs(OUTPUT "SimpleClaim1.bin", fs);
    free_filesystem(&fs);
}

// there is no inodes available to be allocated
TEST_F(ClaimAvailableINodeSuite, INodeUnavailable0)
{
    constexpr fs_retcode_t expected_retcode = INODE_UNAVAILABLE;
    
    filesystem_t fs;
    load_fs(INPUT "tiny_full.bin", fs);

    inode_index_t output_inode_index = 0;
    auto output_retcode = claim_available_inode(&fs, &output_inode_index);

    ASSERT_EQ(output_retcode, expected_retcode) << "Return value do not match!";

    check_fs(INPUT "tiny_full.bin", fs);
    free_filesystem(&fs);
}

// complex, keep allocated until empty
TEST_F(ClaimAvailableINodeSuite, ComplexClaim0)
{
    constexpr size_t num_inodes_to_claim = 36;
    constexpr size_t actual_inode_count = 31;
    
    inode_index_t expected_claimed_list[actual_inode_count] = { 
        17, 22, 19, 25, 21, 20, 18, 28, 
        29,  5, 14,  4, 24, 31, 30,  2,
         3,  1, 10, 15, 27, 23, 11,  8,
         6,  7,  9, 16, 13, 26, 12
    };
    inode_index_t output_claimed_list[actual_inode_count];
    for (auto&& idx : output_claimed_list) idx = -1; // set to dummy values

    filesystem_t fs;
    load_fs(INPUT "empty_random_inode_fragmented.bin", fs);

    for (size_t i = 0; i < actual_inode_count; ++i)
    {
        fs_retcode_t expected_retcode = SUCCESS;
        fs_retcode_t output_retcode = claim_available_inode(&fs, &output_claimed_list[i]);
        ASSERT_EQ(output_retcode, expected_retcode) << "Return value do not match for index " << i << "!";
    }

    for (size_t i = 0; i < num_inodes_to_claim - actual_inode_count; ++i)
    {
        fs_retcode_t expected_retcode = INODE_UNAVAILABLE;
        inode_index_t tmp;
        fs_retcode_t output_retcode = claim_available_inode(&fs, &tmp);
        ASSERT_EQ(output_retcode, expected_retcode) << "Return value do not match for index " << i + actual_inode_count << "!";
    }

    for (size_t i = 0; i < actual_inode_count; ++i)
    {
        ASSERT_EQ(output_claimed_list[i], expected_claimed_list[i]) << "INode claimed by " << i << "th call is incorrect!";
    }

    check_fs(OUTPUT "ComplexClaim0.bin", fs);
    free_filesystem(&fs);
}