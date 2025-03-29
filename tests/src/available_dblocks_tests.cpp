#include "test_util.hpp"

using AvailableDBlocksSuite = fs_internal_test;

TEST_F(AvailableDBlocksSuite, Test0)
{
    constexpr size_t expected_val = 0;

    size_t output_val = available_dblocks(NULL);

    ASSERT_EQ(expected_val, output_val);
}

TEST_F(AvailableDBlocksSuite, Test1)
{
    constexpr size_t expected_val = 10;

    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);

    size_t output_val = available_dblocks(&fs);

    ASSERT_EQ(expected_val, output_val);
    free_filesystem(&fs);
}

TEST_F(AvailableDBlocksSuite, Test2)
{
    constexpr size_t expected_val = 18;

    filesystem_t fs;
    load_fs(INPUT "large.bin", fs);

    size_t output_val = available_dblocks(&fs);

    ASSERT_EQ(expected_val, output_val);
    free_filesystem(&fs);
}

TEST_F(AvailableDBlocksSuite, Test3)
{
    constexpr size_t expected_val = 1;

    filesystem_t fs;
    load_fs(INPUT "medium_near_full_dblock.bin", fs);

    size_t output_val = available_dblocks(&fs);

    ASSERT_EQ(expected_val, output_val);
    free_filesystem(&fs);
}

TEST_F(AvailableDBlocksSuite, Test4)
{
    constexpr size_t expected_val = 0;

    filesystem_t fs;
    load_fs(INPUT "full_medium.bin", fs);

    size_t output_val = available_dblocks(&fs);

    ASSERT_EQ(expected_val, output_val);
    free_filesystem(&fs);
}