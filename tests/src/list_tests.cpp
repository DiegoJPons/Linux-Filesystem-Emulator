#include "test_util.hpp"

using ListSuite = fs_internal_test;

TEST_F(ListSuite, InvalidInput)
{
    constexpr int expected_ret = 0;
    // dummy data for testing
    terminal_context_t ctx{ 
        (filesystem_t*) 0x12345678,
        (inode_t*) 0x87654321
    };

    int ret0, ret1;
    {   // begin stdout logging
        stdout_logger_lock lk{ this };

        ret0 = list(NULL, PATH("root"));
        ret1 = list(&ctx, NULL);
    }   // end stdout logging

    ASSERT_EQ(ret0, expected_ret) << "Incorrect return value for null context argument.";
    ASSERT_EQ(ret1, expected_ret) << "Incorrect return value for null path argument.";

    check_stdout(OUTPUT "Empty.txt");
}

// invalid directory in the path
TEST_F(ListSuite, InvalidPath0)
{
    constexpr size_t inode_index = 0;
    constexpr const char *path = "a/b/e/hello.txt";

    constexpr int expected_ret = -1;

    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);
    int ret;

    terminal_context_t ctx { &fs, &fs.inodes[inode_index] };

    {   // begin stdout logging
        stdout_logger_lock lk{ this };
        ret = list(&ctx, PATH(path));
    }   // end stdout logging

    ASSERT_EQ(ret, expected_ret) << "Incorrect return value";

    check_stdout(OUTPUT "DirectoryNotFound.txt");
    check_fs(INPUT "medium.bin", fs);
    free_filesystem(&fs);
}

// attempting to list file that does not exist
TEST_F(ListSuite, InvalidPath1)
{
    constexpr size_t inode_index = 0;
    constexpr const char *path = "a/b/new.txt";

    constexpr int expected_ret = -1;

    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);
    int ret;

    terminal_context_t ctx { &fs, &fs.inodes[inode_index] };

    {   // begin stdout logging
        stdout_logger_lock lk{ this };
        ret = list(&ctx, PATH(path));
    }   // end stdout logging

    ASSERT_EQ(ret, expected_ret) << "Incorrect return value";

    check_stdout(OUTPUT "ObjectNotFound.txt");
    check_fs(INPUT "medium.bin", fs);
    free_filesystem(&fs);
}

// attempting to list a file
TEST_F(ListSuite, List0)
{
    constexpr size_t inode_index = 2;
    constexpr const char *path = "hello.txt";

    constexpr int expected_ret = 0;

    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);
    int ret;

    terminal_context_t ctx { &fs, &fs.inodes[inode_index] };

    {   // begin stdout logging
        stdout_logger_lock lk{ this };
        ret = list(&ctx, PATH(path));
    }   // end stdout logging

    ASSERT_EQ(ret, expected_ret) << "Incorrect return value";

    check_stdout(OUTPUT "List0.txt");
    check_fs(INPUT "medium.bin", fs);
    free_filesystem(&fs);
}

// attempting to list a directory
TEST_F(ListSuite, List1)
{
    constexpr size_t inode_index = 0;
    constexpr const char *path = ".";

    constexpr int expected_ret = 0;

    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);
    int ret;

    terminal_context_t ctx { &fs, &fs.inodes[inode_index] };

    {   // begin stdout logging
        stdout_logger_lock lk{ this };
        ret = list(&ctx, PATH(path));
    }   // end stdout logging

    ASSERT_EQ(ret, expected_ret) << "Incorrect return value";

    check_stdout(OUTPUT "List1.txt");
    check_fs(INPUT "medium.bin", fs);
    free_filesystem(&fs);
}
