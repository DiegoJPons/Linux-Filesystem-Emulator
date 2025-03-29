#include "test_util.hpp"

using FSOpenSuite = fs_internal_test;

TEST_F(FSOpenSuite, InvalidOpenInput)
{
    fs_file_t file;

    { // begin logging stdout
        stdout_logger_lock lk{ this };
        file = fs_open(NULL, PATH("Test"));
    } // stop logging stdout
    ASSERT_EQ(file, nullptr);

    terminal_context_t context;
    ASSERT_EQ(fs_open(&context, NULL), nullptr);
    check_stdout(OUTPUT "Empty.txt");
}

// test unsuccessful traversal error message
TEST_F(FSOpenSuite, UnsuccessfulOpenTraversal0)
{
    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);

    terminal_context_t context = { &fs, &fs.inodes[0] };
    fs_file_t file;

    { // begin logging stdout
        stdout_logger_lock lk{ this };
        file = fs_open(&context, PATH("a/b/c/d/e"));
    } // stop logging stdout

    ASSERT_EQ(file, nullptr);

    check_stdout(OUTPUT "DirectoryNotFound.txt");

    free_filesystem(&fs);
}

// attempting to open a directory
TEST_F(FSOpenSuite, AttemptOpenDirectory)
{
    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);

    terminal_context_t context = { &fs, &fs.inodes[0] };
    fs_file_t file;

    { // begin logging stdout
        stdout_logger_lock lk{ this };
        file = fs_open(&context, PATH("a/b/c"));
    } // stop logging stdout
    
    ASSERT_EQ(file, nullptr);
    check_stdout(OUTPUT "InvalidFileType.txt");

    free_filesystem(&fs);
}

// attempting to open a file that does not exist
TEST_F(FSOpenSuite, AttemptOpenFileNotFound0)
{
    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);

    terminal_context_t context = { &fs, &fs.inodes[0] };
    fs_file_t file;

    { // begin logging stdout
        stdout_logger_lock lk{ this };
        file = fs_open(&context, PATH("a/b/c/d"));
    } // stop logging stdout

    ASSERT_EQ(file, nullptr);

    check_stdout(OUTPUT "FileNotFound.txt");

    free_filesystem(&fs);
}

TEST_F(FSOpenSuite, AttemptOpenFileNotFound1)
{
    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);

    terminal_context_t context = { &fs, &fs.inodes[0] };
    fs_file_t file;

    { // begin logging stdout
        stdout_logger_lock lk{ this };
        file = fs_open(&context, PATH("a/b/c/.."));
    } // stop logging stdout

    ASSERT_EQ(file, nullptr);

    check_stdout(OUTPUT "InvalidFileType.txt");

    free_filesystem(&fs);
}

// attempting to traverse through a path but runs into a file
// not a directory
TEST_F(FSOpenSuite, UnsuccessfulOpenTraversal1)
{
    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);

    terminal_context_t context = { &fs, &fs.inodes[0] };
    fs_file_t file;

    { // begin logging stdout   
        stdout_logger_lock lk{ this };
        file = fs_open(&context, PATH("a/password/b"));
    } // stop logging stdout 

    ASSERT_EQ(file, nullptr);

    check_stdout(OUTPUT "DirectoryNotFound.txt");

    free_filesystem(&fs);
}

TEST_F(FSOpenSuite, UnsuccessfulOpenTraversal2)
{
    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);

    terminal_context_t context = { &fs, &fs.inodes[0] }; 
    fs_file_t file;

    { // begin logging stdout
        stdout_logger_lock lk{ this };
        file = fs_open(&context, PATH("./a/b/../../../.."));
    } // stop logging stdout

    ASSERT_EQ(file, nullptr);

    check_stdout(OUTPUT "DirectoryNotFound.txt");

    free_filesystem(&fs);
}

TEST_F(FSOpenSuite, SuccessfulOpenTraverse0)
{
    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);

    terminal_context_t context = { &fs, &fs.inodes[0] };
    fs_file_t file;

    { // begin logging stdout
        stdout_logger_lock lk{ this };
        file = fs_open(&context, PATH("a/b/hello.txt"));
    } // stop logging stdout

    ASSERT_NE(file, nullptr);

    ASSERT_EQ(file->fs, &fs);
    ASSERT_EQ(file->inode, &fs.inodes[5]);
    ASSERT_EQ(file->offset, 0);

    check_stdout(OUTPUT "Empty.txt");
    free_filesystem(&fs);
}

TEST_F(FSOpenSuite, SuccessfulOpenTraversal1)
{
    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);

    terminal_context_t context = { &fs, &fs.inodes[0] };
    fs_file_t file;

    { // begin logging stdout
        stdout_logger_lock lk{ this };
        file = fs_open(&context, PATH("book.txt"));
    } // stop logging stdout

    ASSERT_NE(file, nullptr);

    ASSERT_EQ(file->fs, &fs);
    ASSERT_EQ(file->inode, &fs.inodes[4]);
    ASSERT_EQ(file->offset, 0);

    check_stdout(OUTPUT "Empty.txt");
    free_filesystem(&fs);
}

TEST_F(FSOpenSuite, SuccessfulOpenTraversal2)
{
    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);

    terminal_context_t context = { &fs, &fs.inodes[0] };
    fs_file_t file;

    { // begin logging stdout
        stdout_logger_lock lk{ this };
        file = fs_open(&context, PATH("a/b/c/./.././../../book2.txt"));
    } // stop logging stdout

    ASSERT_NE(file, nullptr);

    ASSERT_EQ(file->fs, &fs);
    ASSERT_EQ(file->inode, &fs.inodes[6]);
    ASSERT_EQ(file->offset, 0);

    check_stdout(OUTPUT "Empty.txt");
    free_filesystem(&fs);
}