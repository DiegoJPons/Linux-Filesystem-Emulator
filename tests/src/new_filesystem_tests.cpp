#include "test_util.hpp"

using NewFilesystemSuite = fs_internal_test;

// test invalid input with null fs
TEST_F(NewFilesystemSuite, InvalidInput0)
{
    constexpr fs_retcode_t expected_retcode = INVALID_INPUT;

    auto output_retcode = new_filesystem(NULL, 1, 1);

    ASSERT_EQ(expected_retcode, output_retcode) << "Return values do not match!";
}

// test invalid input with zero inode total or dblock total
TEST_F(NewFilesystemSuite, InvalidInput1)
{
    constexpr fs_retcode_t expected_retcode = INVALID_INPUT;

    filesystem_t fs;
    auto output_retcode0 = new_filesystem(&fs, 0, 1);
    auto output_retcode1 = new_filesystem(&fs, 1, 0);

    ASSERT_EQ(expected_retcode, output_retcode0) << "Return values do not match for inode_total = 0 test case!";
    ASSERT_EQ(expected_retcode, output_retcode1) << "Return values do not match for dblock_total = 0 test case!";
}

TEST_F(NewFilesystemSuite, SmallFS0)
{
    constexpr size_t inode_total = 8;
    constexpr size_t dblock_total = 8;
    constexpr fs_retcode_t expected_retcode = SUCCESS;

    filesystem_t fs;
    auto output_retcode = new_filesystem(&fs, inode_total, dblock_total);

    ASSERT_EQ(expected_retcode, output_retcode) << "Return values do not match!";
    check_fs(OUTPUT "SmallFS0.bin", fs);
    free_filesystem(&fs);
}

TEST_F(NewFilesystemSuite, MediumFS0)
{
    constexpr size_t inode_total = 32;
    constexpr size_t dblock_total = 32;
    constexpr fs_retcode_t expected_retcode = SUCCESS;

    filesystem_t fs;
    auto output_retcode = new_filesystem(&fs, inode_total, dblock_total);

    ASSERT_EQ(expected_retcode, output_retcode) << "Return values do not match!";
    check_fs(OUTPUT "MediumFS0.bin", fs);
    free_filesystem(&fs);
}

TEST_F(NewFilesystemSuite, LargeFS0)
{
    constexpr size_t inode_total = 256;
    constexpr size_t dblock_total = 256;
    constexpr fs_retcode_t expected_retcode = SUCCESS;

    filesystem_t fs;
    auto output_retcode = new_filesystem(&fs, inode_total, dblock_total);

    ASSERT_EQ(expected_retcode, output_retcode) << "Return values do not match!";
    check_fs(OUTPUT "LargeFS0.bin", fs);
    free_filesystem(&fs);
}
