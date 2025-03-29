#include "test_util.hpp"

using NewFileSuite = fs_internal_test;

TEST_F(NewFileSuite, InvalidInput)
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

        ret0 = new_file(NULL, PATH("root"), (permission_t) 0);
        ret1 = new_file(&ctx, NULL, (permission_t) 0);
    }   // end stdout logging

    ASSERT_EQ(ret0, expected_ret) << "Incorrect return value for null context argument.";
    ASSERT_EQ(ret1, expected_ret) << "Incorrect return value for null path argument.";

    check_stdout(OUTPUT "Empty.txt");
}

// invalid directory in the path
TEST_F(NewFileSuite, InvalidPath0)
{
    constexpr size_t inode_index = 0;
    constexpr const char *path = "a/b/e/hello.txt";
    constexpr permission_t perm = permission_t::FS_READ;

    constexpr int expected_ret = -1;

    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);
    int ret;

    terminal_context_t ctx { &fs, &fs.inodes[inode_index] };

    {   // begin stdout logging
        stdout_logger_lock lk{ this };
        ret = new_file(&ctx, PATH(path), perm);
    }   // end stdout logging

    ASSERT_EQ(ret, expected_ret) << "Incorrect return value";

    check_stdout(OUTPUT "DirectoryNotFound.txt");
    check_fs(INPUT "medium.bin", fs);
    free_filesystem(&fs);
}

// attempting to create existing file
TEST_F(NewFileSuite, InvalidPath1)
{
    constexpr size_t inode_index = 0;
    constexpr const char *path = "a/b/hello.txt";
    constexpr permission_t perm = permission_t::FS_READ;

    constexpr int expected_ret = -1;

    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);
    int ret;

    terminal_context_t ctx { &fs, &fs.inodes[inode_index] };

    {   // begin stdout logging
        stdout_logger_lock lk{ this };
        ret = new_file(&ctx, PATH(path), perm);
    }   // end stdout logging

    ASSERT_EQ(ret, expected_ret) << "Incorrect return value";

    check_stdout(OUTPUT "FileExist.txt");
    check_fs(INPUT "medium.bin", fs);
    free_filesystem(&fs);
}

// failure due to unable to allocate dblock
TEST_F(NewFileSuite, AllocationFailure0)
{
    constexpr size_t inode_index = 0;
    constexpr const char *path = "a/b/new.txt";
    constexpr permission_t perm = permission_t::FS_READ;

    constexpr int expected_ret = -1;

    filesystem_t fs;
    load_fs(INPUT "full_medium.bin", fs);
    int ret;

    terminal_context_t ctx { &fs, &fs.inodes[inode_index] };

    {   // begin stdout logging
        stdout_logger_lock lk{ this };
        ret = new_file(&ctx, PATH(path), perm);
    }   // end stdout logging

    ASSERT_EQ(ret, expected_ret) << "Incorrect return value";

    check_stdout(OUTPUT "FailedDBlockAlloc.txt");
    check_fs(INPUT "full_medium.bin", fs);
    free_filesystem(&fs);
}

// failure due to unable to allocate inode
TEST_F(NewFileSuite, AllocationFailure1)
{
    constexpr size_t inode_index = 0;
    constexpr const char *path = "a/b/c/new.txt";
    constexpr permission_t perm = permission_t::FS_READ;

    constexpr int expected_ret = -1;

    filesystem_t fs;
    load_fs(INPUT "small_full_inode.bin", fs);
    int ret;

    terminal_context_t ctx { &fs, &fs.inodes[inode_index] };

    {   // begin stdout logging
        stdout_logger_lock lk{ this };
        ret = new_file(&ctx, PATH(path), perm);
    }   // end stdout logging

    ASSERT_EQ(ret, expected_ret) << "Incorrect return value";

    check_stdout(OUTPUT "FailedINodeAlloc.txt");
    check_fs(INPUT "small_full_inode.bin", fs);
    free_filesystem(&fs);
}

// success in creating new file
// does not write in a tombstone
TEST_F(NewFileSuite, NewFile0)
{
    constexpr size_t inode_index = 1;
    constexpr const char *path = "b/c/new.txt";
    constexpr permission_t perm = (permission_t) 7;

    constexpr int expected_ret = 0;

    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);
    int ret;

    terminal_context_t ctx { &fs, &fs.inodes[inode_index] };

    {   // begin stdout logging
        stdout_logger_lock lk{ this };
        ret = new_file(&ctx, PATH(path), perm);
    }   // end stdout logging

    ASSERT_EQ(ret, expected_ret) << "Incorrect return value";

    check_stdout(OUTPUT "Empty.txt");
    check_fs(OUTPUT "NewFile0.bin", fs);
    free_filesystem(&fs);
}

// success in creating new file
// does write in a tombstone
TEST_F(NewFileSuite, NewFile1)
{
    constexpr size_t inode_index = 0;
    constexpr const char *path = "a/new.txt";
    constexpr permission_t perm = (permission_t) 7;

    constexpr int expected_ret = 0;

    filesystem_t fs;
    load_fs(INPUT "medium_tombstone.bin", fs);
    int ret;

    terminal_context_t ctx { &fs, &fs.inodes[inode_index] };

    {   // begin stdout logging
        stdout_logger_lock lk{ this };
        ret = new_file(&ctx, PATH(path), perm);
    }   // end stdout logging

    ASSERT_EQ(ret, expected_ret) << "Incorrect return value";

    check_stdout(OUTPUT "Empty.txt");
    check_fs(OUTPUT "NewFile1.bin", fs);
    free_filesystem(&fs);
}