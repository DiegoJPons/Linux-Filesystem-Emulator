#include "test_util.hpp"

using ClaimAvailableDBlockSuite = fs_internal_test;

// test invalid input
TEST_F(ClaimAvailableDBlockSuite, InvalidInput)
{
    constexpr fs_retcode_t expected_retcode = INVALID_INPUT;

    filesystem_t fs;
    dblock_index_t idx;
    auto output_retcode0 = claim_available_dblock(NULL, &idx);
    auto output_retcode1 = claim_available_dblock(&fs, NULL);

    ASSERT_EQ(expected_retcode, output_retcode0) << "Return values do not match for fs = NULL case!";
    ASSERT_EQ(expected_retcode, output_retcode1) << "Return values do not match for index = NULL test case!";
}

TEST_F(ClaimAvailableDBlockSuite, SimpleClaimDBlock0)
{
    constexpr fs_retcode_t expected_retcode = SUCCESS;
    constexpr dblock_index_t expected_dblock_index = 4;
    
    filesystem_t fs;
    load_fs(INPUT "medium_tombstone.bin", fs);

    dblock_index_t output_dblock_index = 0;
    auto output_retcode = claim_available_dblock(&fs, &output_dblock_index);

    ASSERT_EQ(output_retcode, expected_retcode) << "Return value do not match!";
    ASSERT_EQ(output_dblock_index, expected_dblock_index) << "D-Block index value do not match!";

    check_fs(OUTPUT "SimpleClaimDBlock0.bin", fs);
    free_filesystem(&fs);
}

TEST_F(ClaimAvailableDBlockSuite, SimpleClaim1)
{
    constexpr fs_retcode_t expected_retcode = SUCCESS;
    constexpr dblock_index_t expected_dblock_index = 6;
    
    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);

    dblock_index_t output_dblock_index = 0;
    auto output_retcode = claim_available_dblock(&fs, &output_dblock_index);

    ASSERT_EQ(output_retcode, expected_retcode) << "Return value do not match!";
    ASSERT_EQ(output_dblock_index, expected_dblock_index) << "D-Block index value do not match!";

    check_fs(OUTPUT "SimpleClaimDBlock1.bin", fs);
    free_filesystem(&fs);
}

// there is no inodes available to be allocated
TEST_F(ClaimAvailableDBlockSuite, DBlockUnavailable0)
{
    constexpr fs_retcode_t expected_retcode = DBLOCK_UNAVAILABLE;
    
    filesystem_t fs;
    load_fs(INPUT "full_medium.bin", fs);

    dblock_index_t output_dblock_index = 0;
    auto output_retcode = claim_available_dblock(&fs, &output_dblock_index);

    ASSERT_EQ(output_retcode, expected_retcode) << "Return value do not match!";

    check_fs(INPUT "full_medium.bin", fs);
    free_filesystem(&fs);
}

// complex, keep allocating until empty
TEST_F(ClaimAvailableDBlockSuite, DBlockComplexClaim0)
{
    constexpr size_t num_dblocks_to_claim = 20;
    constexpr size_t actual_dblock_count = 16;
    
    dblock_index_t expected_claimed_list[actual_dblock_count] = { 
        1, 3, 4, 5, 6, 8, 9, 14, 16, 17, 18, 19, 22, 25, 26, 29
    };
    dblock_index_t output_claimed_list[actual_dblock_count];
    for (auto&& idx : output_claimed_list) idx = -1; // set to dummy values

    filesystem_t fs;
    load_fs(INPUT "empty_random_inode_fragmented.bin", fs);

    for (size_t i = 0; i < actual_dblock_count; ++i)
    {
        fs_retcode_t expected_retcode = SUCCESS;
        fs_retcode_t output_retcode = claim_available_dblock(&fs, &output_claimed_list[i]);
        ASSERT_EQ(output_retcode, expected_retcode) << "Return value do not match for index " << i << "!";
    }

    for (size_t i = 0; i < num_dblocks_to_claim - actual_dblock_count; ++i)
    {
        fs_retcode_t expected_retcode = DBLOCK_UNAVAILABLE;
        dblock_index_t tmp;
        fs_retcode_t output_retcode = claim_available_dblock(&fs, &tmp);
        ASSERT_EQ(output_retcode, expected_retcode) << "Return value do not match for index " << i + actual_dblock_count;
    }

    for (size_t i = 0; i < actual_dblock_count; ++i)
    {
        ASSERT_EQ(output_claimed_list[i], expected_claimed_list[i]) << "D-Block claimed by " << i << "th call is incorrect!";
    }

    check_fs(OUTPUT "DBlockComplexClaim0.bin", fs);
    free_filesystem(&fs);
}