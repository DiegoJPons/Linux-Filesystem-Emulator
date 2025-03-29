#include "test_util.hpp"

using NewDirectorySuite = fs_internal_test;

TEST_F(NewDirectorySuite, InvalidInput)
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

        ret0 = new_directory(NULL, PATH("root"));
        ret1 = new_directory(&ctx, NULL);
    }   // end stdout logging

    ASSERT_EQ(ret0, expected_ret) << "Incorrect return value for null context argument.";
    ASSERT_EQ(ret1, expected_ret) << "Incorrect return value for null path argument.";

    check_stdout(OUTPUT "Empty.txt");
}

// invalid directory in the path
TEST_F(NewDirectorySuite, InvalidPath0)
{
    constexpr size_t inode_index = 0;
    constexpr const char *path = "a/b/e/hello";

    constexpr int expected_ret = -1;

    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);
    int ret;

    terminal_context_t ctx { &fs, &fs.inodes[inode_index] };

    {   // begin stdout logging
        stdout_logger_lock lk{ this };
        ret = new_directory(&ctx, PATH(path));
    }   // end stdout logging

    ASSERT_EQ(ret, expected_ret) << "Incorrect return value";

    check_stdout(OUTPUT "DirectoryNotFound.txt");
    check_fs(INPUT "medium.bin", fs);
    free_filesystem(&fs);
}

// attempting to create existing directory
TEST_F(NewDirectorySuite, InvalidPath1)
{
    constexpr size_t inode_index = 0;
    constexpr const char *path = "a/b/c";

    constexpr int expected_ret = -1;

    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);
    int ret;

    terminal_context_t ctx { &fs, &fs.inodes[inode_index] };

    {   // begin stdout logging
        stdout_logger_lock lk{ this };
        ret = new_directory(&ctx, PATH(path));
    }   // end stdout logging

    ASSERT_EQ(ret, expected_ret) << "Incorrect return value";

    check_stdout(OUTPUT "DirectoryExist.txt");
    check_fs(INPUT "medium.bin", fs);
    free_filesystem(&fs);
}

// failure due to unable to allocate dblock
TEST_F(NewDirectorySuite, AllocationFailure0)
{
    constexpr size_t inode_index = 0;
    constexpr const char *path = "a/b/new";

    constexpr int expected_ret = -1;

    filesystem_t fs;
    load_fs(INPUT "full_medium.bin", fs);
    int ret;

    terminal_context_t ctx { &fs, &fs.inodes[inode_index] };

    {   // begin stdout logging
        stdout_logger_lock lk{ this };
        ret = new_directory(&ctx, PATH(path));
    }   // end stdout logging

    ASSERT_EQ(ret, expected_ret) << "Incorrect return value";

    check_stdout(OUTPUT "FailedDBlockAlloc.txt");
    check_fs(INPUT "full_medium.bin", fs);
    free_filesystem(&fs);
}

// failure due to unable to allocate inode
TEST_F(NewDirectorySuite, AllocationFailure1)
{
    constexpr size_t inode_index = 0;
    constexpr const char *path = "a/b/c/new";

    constexpr int expected_ret = -1;

    filesystem_t fs;
    load_fs(INPUT "small_full_inode.bin", fs);
    int ret;

    terminal_context_t ctx { &fs, &fs.inodes[inode_index] };

    {   // begin stdout logging
        stdout_logger_lock lk{ this };
        ret = new_directory(&ctx, PATH(path));
    }   // end stdout logging

    ASSERT_EQ(ret, expected_ret) << "Incorrect return value";

    check_stdout(OUTPUT "FailedINodeAlloc.txt");
    check_fs(INPUT "small_full_inode.bin", fs);
    free_filesystem(&fs);
}

// failure due to unable to competely allocate dblocks 
// can only allocate 1/2 of the necessary dblocks for the operation
TEST_F(NewDirectorySuite, AllocationFailure2)
{
    constexpr size_t inode_index = 0;
    constexpr const char *path = "a/d"; 

    constexpr int expected_ret = -1;

    filesystem_t fs;
    load_fs(INPUT "medium_near_full_dblock.bin", fs);
    int ret;

    terminal_context_t ctx { &fs, &fs.inodes[inode_index] };

    {   // begin stdout logging
        stdout_logger_lock lk{ this };
        ret = new_directory(&ctx, PATH(path));
    }   // end stdout logging

    ASSERT_EQ(ret, expected_ret) << "Incorrect return value";

    check_stdout(OUTPUT "FailedDBlockAlloc.txt");
    check_fs(INPUT "medium_near_full_dblock.bin", fs);
    free_filesystem(&fs);
}

// success in creating new directory
// does not write in a tombstone
TEST_F(NewDirectorySuite, NewDirectory0)
{
    constexpr size_t inode_index = 1;
    constexpr const char *path = "b/c/new";

    constexpr int expected_ret = 0;

    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);
    int ret;

    terminal_context_t ctx { &fs, &fs.inodes[inode_index] };

    {   // begin stdout logging
        stdout_logger_lock lk{ this };
        ret = new_directory(&ctx, PATH(path));
    }   // end stdout logging

    ASSERT_EQ(ret, expected_ret) << "Incorrect return value";

    check_stdout(OUTPUT "Empty.txt");
    check_fs(OUTPUT "NewDirectory0.bin", fs);
    free_filesystem(&fs);
}

// success in creating new file
// does write in a tombstone
TEST_F(NewDirectorySuite, NewDirectory1)
{
    constexpr size_t inode_index = 0;
    constexpr const char *path = "a/new";

    constexpr int expected_ret = 0;

    filesystem_t fs;
    load_fs(INPUT "medium_tombstone.bin", fs);
    int ret;

    terminal_context_t ctx { &fs, &fs.inodes[inode_index] };

    {   // begin stdout logging
        stdout_logger_lock lk{ this };
        ret = new_directory(&ctx, PATH(path));
    }   // end stdout logging

    ASSERT_EQ(ret, expected_ret) << "Incorrect return value";

    check_stdout(OUTPUT "Empty.txt");
    check_fs(OUTPUT "NewDirectory1.bin", fs);
    free_filesystem(&fs);
}