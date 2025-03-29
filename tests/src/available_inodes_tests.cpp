#include "test_util.hpp"

using AvailableInodesSuite = fs_internal_test;

TEST_F(AvailableInodesSuite, Test0)
{
    constexpr size_t expected_val = 0;

    size_t output_val = available_inodes(NULL);

    ASSERT_EQ(expected_val, output_val);
}

TEST_F(AvailableInodesSuite, Test1)
{
    constexpr size_t expected_val = 6;

    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);

    size_t output_val = available_inodes(&fs);

    ASSERT_EQ(expected_val, output_val);
    free_filesystem(&fs);
}

TEST_F(AvailableInodesSuite, Test2)
{
    constexpr size_t expected_val = 25;

    filesystem_t fs;
    load_fs(INPUT "large.bin", fs);

    size_t output_val = available_inodes(&fs);

    ASSERT_EQ(expected_val, output_val);
    free_filesystem(&fs);
}

TEST_F(AvailableInodesSuite, Test3)
{
    constexpr size_t expected_val = 20;

    filesystem_t fs;
    load_fs(INPUT "medium_near_full_dblock.bin", fs);

    size_t output_val = available_inodes(&fs);

    ASSERT_EQ(expected_val, output_val);
    free_filesystem(&fs);
}

TEST_F(AvailableInodesSuite, Test4)
{
    constexpr size_t expected_val = 0;

    filesystem_t fs;
    load_fs(INPUT "small_full_inode.bin", fs);

    size_t output_val = available_inodes(&fs);

    ASSERT_EQ(expected_val, output_val);
    free_filesystem(&fs);
}