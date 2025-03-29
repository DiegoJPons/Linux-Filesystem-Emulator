#include "test_util.hpp"

using RemoveDirectorySuite = fs_internal_test;

TEST_F(RemoveDirectorySuite, InvalidInput)
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

        ret0 = remove_directory(NULL, PATH("root"));
        ret1 = remove_directory(&ctx, NULL);
    }   // end stdout logging

    ASSERT_EQ(ret0, expected_ret) << "Incorrect return value for null context argument.";
    ASSERT_EQ(ret1, expected_ret) << "Incorrect return value for null path argument.";

    check_stdout(OUTPUT "Empty.txt");
}

// invalid directory in path
TEST_F(RemoveDirectorySuite, InvalidPath0)
{
    constexpr size_t inode_index = 0;
    constexpr const char *path = "a/b/e/d";

    constexpr int expected_ret = -1;

    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);
    int ret;

    terminal_context_t ctx { &fs, &fs.inodes[inode_index] };

    {   // begin stdout logging
        stdout_logger_lock lk{ this };
        ret = remove_directory(&ctx, PATH(path));
    }   // end stdout logging

    ASSERT_EQ(ret, expected_ret) << "Incorrect return value";

    check_stdout(OUTPUT "DirectoryNotFound.txt");
    check_fs(INPUT "medium.bin", fs);
    free_filesystem(&fs);
}

// attempting to delete directory that does not exist
TEST_F(RemoveDirectorySuite, InvalidPath1)
{
    constexpr size_t inode_index = 0;
    constexpr const char *path = "a/b/z";

    constexpr int expected_ret = -1;

    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);
    int ret;

    terminal_context_t ctx { &fs, &fs.inodes[inode_index] };

    {   // begin stdout logging
        stdout_logger_lock lk{ this };
        ret = remove_directory(&ctx, PATH(path));
    }   // end stdout logging

    ASSERT_EQ(ret, expected_ret) << "Incorrect return value";

    check_stdout(OUTPUT "DirectoryNotFound.txt");
    check_fs(INPUT "medium.bin", fs);
    free_filesystem(&fs);
}

// basename in path includes .
TEST_F(RemoveDirectorySuite, InvalidPath2)
{
    constexpr size_t inode_index = 0;
    constexpr const char *path = "a/b/.";

    constexpr int expected_ret = -1;

    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);
    int ret;

    terminal_context_t ctx { &fs, &fs.inodes[inode_index] };

    {   // begin stdout logging
        stdout_logger_lock lk{ this };
        ret = remove_directory(&ctx, PATH(path));
    }   // end stdout logging

    ASSERT_EQ(ret, expected_ret) << "Incorrect return value";

    check_stdout(OUTPUT "InvalidFilename.txt");
    check_fs(INPUT "medium.bin", fs);
    free_filesystem(&fs);
}

// attempting to remove a directory that is not empty
TEST_F(RemoveDirectorySuite, RemoveDirNotEmpty0)
{
    constexpr size_t inode_index = 1;
    constexpr const char *path = "b";

    constexpr int expected_ret = -1;

    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);
    int ret;

    terminal_context_t ctx { &fs, &fs.inodes[inode_index] };

    {   // begin stdout logging
        stdout_logger_lock lk{ this };
        ret = remove_directory(&ctx, PATH(path));
    }   // end stdout logging

    ASSERT_EQ(ret, expected_ret) << "Incorrect return value";

    check_stdout(OUTPUT "DirectoryNotEmpty.txt");
    check_fs(INPUT "medium.bin", fs);
    free_filesystem(&fs);
}

// attempting to remove the current working directory
TEST_F(RemoveDirectorySuite, RemoveCWDir0)
{
    constexpr size_t inode_index = 3;
    constexpr const char *path = "../c";

    constexpr int expected_ret = -1;

    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);
    int ret;

    terminal_context_t ctx { &fs, &fs.inodes[inode_index] };

    {   // begin stdout logging
        stdout_logger_lock lk{ this };
        ret = remove_directory(&ctx, PATH(path));
    }   // end stdout logging

    ASSERT_EQ(ret, expected_ret) << "Incorrect return value";

    check_stdout(OUTPUT "AttemptDeleteCWD.txt");
    check_fs(INPUT "medium.bin", fs);
    free_filesystem(&fs);
}

// successful remove directory, leaves a tombstone, no shrink
TEST_F(RemoveDirectorySuite, RemoveDir0)
{
    constexpr size_t inode_index = 1;
    constexpr const char *path = "b/c";

    constexpr int expected_ret = 0;

    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);
    int ret;

    terminal_context_t ctx { &fs, &fs.inodes[inode_index] };

    {   // begin stdout logging
        stdout_logger_lock lk{ this };
        ret = remove_directory(&ctx, PATH(path));
    }   // end stdout logging

    ASSERT_EQ(ret, expected_ret) << "Incorrect return value";

    check_stdout(OUTPUT "Empty.txt");
    check_fs(OUTPUT "RemoveDir0.bin", fs);
    free_filesystem(&fs);
}

// successful remove directory, leaves a tombstone, shrink
TEST_F(RemoveDirectorySuite, RemoveDir1)
{
    constexpr size_t inode_index = 0;
    constexpr const char *path = "c";

    constexpr int expected_ret = 0;

    filesystem_t fs;
    load_fs(INPUT "medium_tombstone.bin", fs);
    int ret;

    terminal_context_t ctx { &fs, &fs.inodes[inode_index] };

    {   // begin stdout logging
        stdout_logger_lock lk{ this };
        ret = remove_directory(&ctx, PATH(path));
    }   // end stdout logging

    ASSERT_EQ(ret, expected_ret) << "Incorrect return value";

    check_stdout(OUTPUT "Empty.txt");
    check_fs(OUTPUT "RemoveDir1.bin", fs);
    free_filesystem(&fs);
}