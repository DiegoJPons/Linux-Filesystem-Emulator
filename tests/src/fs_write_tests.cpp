#include "test_util.hpp"

using FSWriteSuite = fs_internal_test;

TEST_F(FSWriteSuite, InvalidInput)
{
    size_t output_ret;
    {
        stdout_logger_lock lk{ this };
        output_ret = fs_write(NULL, NULL, 0);
    }
    ASSERT_EQ( output_ret, 0 );
    check_stdout(OUTPUT "Empty.txt");
}

// write from beginning of file
// does not expand the size of the file
TEST_F(FSWriteSuite, SimpleWrite0)
{
    constexpr size_t buffer_size = 80;
    constexpr size_t offset = 0;
    constexpr size_t expected_ret = 80;
    constexpr size_t inode_index = 1;
    constexpr size_t expected_file_size = 614;

    filesystem_t fs;
    load_fs(INPUT "medium_text.bin", fs);

    inode_t *inode = &fs.inodes[inode_index];
    struct fs_file file {
        &fs,
        inode,
        offset
    };
    char buffer[buffer_size] = { 0 };
    memset(buffer, 0x24, buffer_size);
    size_t output_ret;
   
    { // begin logging stdout
        stdout_logger_lock lk{ this };
        output_ret = fs_write(&file, buffer, buffer_size);
    } // stop logging stdout

    ASSERT_EQ(output_ret, expected_ret) << "Return value does not match the expected.";
    ASSERT_EQ(inode->internal.file_size, expected_file_size) << "File size is not correct.";

    // now check if the file object is updated correctly
    ASSERT_EQ(file.fs, &fs) << "Filesystem address was incorrectly changed in fs_file_t object.";
    ASSERT_EQ(file.inode, inode) << "INode address was incorrectly changed in fs_file_t object.";
    size_t expected_final_offset = offset + expected_ret;
    ASSERT_EQ(file.offset, expected_final_offset) << "New file offset is incorrect.";

    check_stdout(OUTPUT "Empty.txt");
    check_fs(OUTPUT "SimpleWrite0.bin", fs);

    free_filesystem(&fs);
}

// write from some point in the file
// does not expand the size of the file 
TEST_F(FSWriteSuite, SimpleWrite1)
{
    constexpr size_t buffer_size = 47;
    constexpr size_t offset = 22;
    constexpr size_t expected_ret = 47;
    constexpr size_t inode_index = 2;
    constexpr size_t expected_file_size = 189;

    filesystem_t fs;
    load_fs(INPUT "medium_text.bin", fs);

    inode_t *inode = &fs.inodes[inode_index];
    struct fs_file file {
        &fs,
        inode,
        offset
    };
    char buffer[buffer_size] = { 0 };
    memset(buffer, 0x30, buffer_size);
    size_t output_ret;
   
    { // begin logging stdout
        stdout_logger_lock lk{ this };
        output_ret = fs_write(&file, buffer, buffer_size);
    } // stop logging stdout

    ASSERT_EQ(output_ret, expected_ret) << "Return value does not match the expected.";
    ASSERT_EQ(inode->internal.file_size, expected_file_size) << "File size is not correct.";

    // now check if the file object is updated correctly
    ASSERT_EQ(file.fs, &fs) << "Filesystem address was incorrectly changed in fs_file_t object.";
    ASSERT_EQ(file.inode, inode) << "INode address was incorrectly changed in fs_file_t object.";
    size_t expected_final_offset = offset + expected_ret;
    ASSERT_EQ(file.offset, expected_final_offset) << "New file offset is incorrect.";

    check_stdout(OUTPUT "Empty.txt");
    check_fs(OUTPUT "SimpleWrite1.bin", fs);

    free_filesystem(&fs);
}

// write from the end of the file
// expand the size of the file
TEST_F(FSWriteSuite, WriteFromEOF0)
{
    constexpr size_t buffer_size = 32;
    constexpr size_t offset = 614;
    constexpr size_t expected_ret = 32;
    constexpr size_t inode_index = 1;
    constexpr size_t expected_file_size = offset + expected_ret;

    filesystem_t fs;
    load_fs(INPUT "medium_text.bin", fs);

    inode_t *inode = &fs.inodes[inode_index];
    struct fs_file file {
        &fs,
        inode,
        offset
    };
    char buffer[buffer_size] = { 0 };
    memset(buffer, 0x41, buffer_size);
    size_t output_ret;
   
    { // begin logging stdout
        stdout_logger_lock lk{ this };
        output_ret = fs_write(&file, buffer, buffer_size);
    } // stop logging stdout

    ASSERT_EQ(output_ret, expected_ret) << "Return value does not match the expected.";
    ASSERT_EQ(inode->internal.file_size, expected_file_size) << "File size is not correct.";

    // now check if the file object is updated correctly
    ASSERT_EQ(file.fs, &fs) << "Filesystem address was incorrectly changed in fs_file_t object.";
    ASSERT_EQ(file.inode, inode) << "INode address was incorrectly changed in fs_file_t object.";
    size_t expected_final_offset = offset + expected_ret;
    ASSERT_EQ(file.offset, expected_final_offset) << "New file offset is incorrect.";

    check_stdout(OUTPUT "Empty.txt");
    check_fs(OUTPUT "WriteFromEOF0.bin", fs);

    free_filesystem(&fs);
}

// write from the end of the file
// expand the size of the file
TEST_F(FSWriteSuite, WriteExpandFile0)
{
    constexpr size_t buffer_size = 32;
    constexpr size_t offset = 600;
    constexpr size_t expected_ret = 32;
    constexpr size_t inode_index = 1;
    constexpr size_t expected_file_size = offset + expected_ret;

    filesystem_t fs;
    load_fs(INPUT "medium_text.bin", fs);

    inode_t *inode = &fs.inodes[inode_index];
    struct fs_file file {
        &fs,
        inode,
        offset
    };
    char buffer[buffer_size] = { 0 };
    memset(buffer, 0x44, buffer_size);
    size_t output_ret;
   
    { // begin logging stdout
        stdout_logger_lock lk{ this };
        output_ret = fs_write(&file, buffer, buffer_size);
    } // stop logging stdout

    ASSERT_EQ(output_ret, expected_ret) << "Return value does not match the expected.";
    ASSERT_EQ(inode->internal.file_size, expected_file_size) << "File size is not correct.";

    // now check if the file object is updated correctly
    ASSERT_EQ(file.fs, &fs) << "Filesystem address was incorrectly changed in fs_file_t object.";
    ASSERT_EQ(file.inode, inode) << "INode address was incorrectly changed in fs_file_t object.";
    size_t expected_final_offset = offset + expected_ret;
    ASSERT_EQ(file.offset, expected_final_offset) << "New file offset is incorrect.";

    check_stdout(OUTPUT "Empty.txt");
    check_fs(OUTPUT "WriteExpandFile0.bin", fs);

    free_filesystem(&fs);
}