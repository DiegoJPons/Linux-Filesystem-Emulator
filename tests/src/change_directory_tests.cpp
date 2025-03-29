#include "test_util.hpp"

using ChangeDirectorySuite = fs_internal_test;

TEST_F(ChangeDirectorySuite, InvalidInput)
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

        ret0 = change_directory(NULL, PATH("root"));
        ret1 = change_directory(&ctx, NULL);
    }   // end stdout logging

    ASSERT_EQ(ret0, expected_ret) << "Incorrect return value for null context argument.";
    ASSERT_EQ(ret1, expected_ret) << "Incorrect return value for null path argument.";

    ASSERT_EQ(ctx.fs, (filesystem_t*) 0x12345678) << "File system pointer should not be modified in the terminal_context_t";
    ASSERT_EQ(ctx.working_directory, (inode_t*) 0x87654321) << "Working directory should not be modified in the terminal_context_t";

    check_stdout(OUTPUT "Empty.txt");
}

// invalid directory in path
TEST_F(ChangeDirectorySuite, InvalidPath0)
{
    constexpr size_t inode_index = 0;
    constexpr const char *path = "a/b/e/f";

    constexpr int expected_ret = -1;
    constexpr size_t expected_inode_index = inode_index;

    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);
    int ret;

    terminal_context_t ctx { &fs, &fs.inodes[inode_index] };

    {   // begin stdout logging
        stdout_logger_lock lk{ this };
        ret = change_directory(&ctx, PATH(path));
    }   // end stdout logging

    ASSERT_EQ(ret, expected_ret) << "Incorrect return value";

    ASSERT_EQ(ctx.fs, &fs) << "File system pointer should not be modified in the terminal_context_t";
    ASSERT_EQ(ctx.working_directory, &fs.inodes[expected_inode_index]) << "Working directory of terminal_context_t is not correct.";

    check_stdout(OUTPUT "DirectoryNotFound.txt");
    check_fs(INPUT "medium.bin", fs);
    free_filesystem(&fs);
}

// cd into base name directory that does not exist
TEST_F(ChangeDirectorySuite, InvalidPath1)
{
    constexpr size_t inode_index = 1;
    constexpr const char *path = "b/e";

    constexpr int expected_ret = -1;
    constexpr size_t expected_inode_index = inode_index;

    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);
    int ret;

    terminal_context_t ctx { &fs, &fs.inodes[inode_index] };

    {   // begin stdout logging
        stdout_logger_lock lk{ this };
        ret = change_directory(&ctx, PATH(path));
    }   // end stdout logging

    ASSERT_EQ(ret, expected_ret) << "Incorrect return value";

    ASSERT_EQ(ctx.fs, &fs) << "File system pointer should not be modified in the terminal_context_t";
    ASSERT_EQ(ctx.working_directory, &fs.inodes[expected_inode_index]) << "Working directory of terminal_context_t is not correct.";

    check_stdout(OUTPUT "DirectoryNotFound.txt");
    check_fs(INPUT "medium.bin", fs);
    free_filesystem(&fs);
}

// cd into base name directory that is actually a data file
TEST_F(ChangeDirectorySuite, InvalidPath2)
{
    constexpr size_t inode_index = 1;
    constexpr const char *path = "b/hello.txt";

    constexpr int expected_ret = -1;
    constexpr size_t expected_inode_index = inode_index;

    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);
    int ret;

    terminal_context_t ctx { &fs, &fs.inodes[inode_index] };

    {   // begin stdout logging
        stdout_logger_lock lk{ this };
        ret = change_directory(&ctx, PATH(path));
    }   // end stdout logging

    ASSERT_EQ(ret, expected_ret) << "Incorrect return value";

    ASSERT_EQ(ctx.fs, &fs) << "File system pointer should not be modified in the terminal_context_t";
    ASSERT_EQ(ctx.working_directory, &fs.inodes[expected_inode_index]) << "Working directory of terminal_context_t is not correct.";

    check_stdout(OUTPUT "DirectoryNotFound.txt");
    check_fs(INPUT "medium.bin", fs);
    free_filesystem(&fs);
}

// cd into a different directory
TEST_F(ChangeDirectorySuite, ChangeDir0)
{
    constexpr size_t inode_index = 1;
    constexpr const char *path = "b/c";

    constexpr int expected_ret = 0;
    constexpr size_t expected_inode_index = 3;

    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);
    int ret;

    terminal_context_t ctx { &fs, &fs.inodes[inode_index] };

    {   // begin stdout logging
        stdout_logger_lock lk{ this };
        ret = change_directory(&ctx, PATH(path));
    }   // end stdout logging

    ASSERT_EQ(ret, expected_ret) << "Incorrect return value";

    ASSERT_EQ(ctx.fs, &fs) << "File system pointer should not be modified in the terminal_context_t";
    ASSERT_EQ(ctx.working_directory, &fs.inodes[expected_inode_index]) << "Working directory of terminal_context_t is not correct.";

    check_stdout(OUTPUT "Empty.txt");
    check_fs(INPUT "medium.bin", fs);
    free_filesystem(&fs);
}

// cd into the different directory indirectly
TEST_F(ChangeDirectorySuite, ChangeDir1)
{
    constexpr size_t inode_index = 1;
    constexpr const char *path = "./b/../b/c/../c/.";

    constexpr int expected_ret = 0;
    constexpr size_t expected_inode_index = 3;

    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);
    int ret;

    terminal_context_t ctx { &fs, &fs.inodes[inode_index] };

    {   // begin stdout logging
        stdout_logger_lock lk{ this };
        ret = change_directory(&ctx, PATH(path));
    }   // end stdout logging

    ASSERT_EQ(ret, expected_ret) << "Incorrect return value";

    ASSERT_EQ(ctx.fs, &fs) << "File system pointer should not be modified in the terminal_context_t";
    ASSERT_EQ(ctx.working_directory, &fs.inodes[expected_inode_index]) << "Working directory of terminal_context_t is not correct.";

    check_stdout(OUTPUT "Empty.txt");
    check_fs(INPUT "medium.bin", fs);
    free_filesystem(&fs);
}

// cd into the same directory indirectly
TEST_F(ChangeDirectorySuite, ChangeDir2)
{
    constexpr size_t inode_index = 1;
    constexpr const char *path = "./b/../b/c/../c/.././..";

    constexpr int expected_ret = 0;
    constexpr size_t expected_inode_index = inode_index;

    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);
    int ret;

    terminal_context_t ctx { &fs, &fs.inodes[inode_index] };

    {   // begin stdout logging
        stdout_logger_lock lk{ this };
        ret = change_directory(&ctx, PATH(path));
    }   // end stdout logging

    ASSERT_EQ(ret, expected_ret) << "Incorrect return value";

    ASSERT_EQ(ctx.fs, &fs) << "File system pointer should not be modified in the terminal_context_t";
    ASSERT_EQ(ctx.working_directory, &fs.inodes[expected_inode_index]) << "Working directory of terminal_context_t is not correct.";

    check_stdout(OUTPUT "Empty.txt");
    check_fs(INPUT "medium.bin", fs);
    free_filesystem(&fs);
}