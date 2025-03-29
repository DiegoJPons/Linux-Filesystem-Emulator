#include "test_util.hpp"

using FSReadSuite = fs_internal_test;

constexpr size_t OVERFLOW = 128;

TEST_F(FSReadSuite, InvalidInput)
{
    size_t output_ret;
    {
        stdout_logger_lock lk{ this };
        output_ret = fs_read(NULL, NULL, 0);
    }
    ASSERT_EQ( output_ret, 0 );
    check_stdout(OUTPUT "Empty.txt");
}

TEST_F(FSReadSuite, SimpleRead0)
{
    constexpr size_t buffer_size = 80;
    constexpr size_t offset = 0;
    constexpr size_t expected_size = 80;
    constexpr size_t inode_index = 1;

    filesystem_t fs;
    load_fs(INPUT "medium_text.bin", fs);

    inode_t *inode = &fs.inodes[inode_index];
    struct fs_file file {
        &fs,
        inode,
        offset
    };
    char buffer[buffer_size + OVERFLOW] = { 0 };
    size_t output_size;
   
    { // begin logging stdout
        stdout_logger_lock lk{ this };
        output_size = fs_read(&file, buffer, buffer_size);
    } // stop logging stdout

    ASSERT_EQ(output_size, expected_size) << "Bytes read does not match the expected.";

    const char *expected = "Hi. My name is $@#%^$@. It is a pleasure to meet you, but unfortunately, I canno";
    
    size_t i = 0;
    for (; i < expected_size; ++i) 
        ASSERT_EQ(buffer[i], expected[i]) << "Mismatched byte for index " << i;
    for (size_t j = 0; j < OVERFLOW; ++j, ++i) 
        ASSERT_EQ(buffer[i], 0) << "Wrote into overflow buffer.";

    // now check if the file object is updated correctly
    ASSERT_EQ(file.fs, &fs) << "Filesystem address was incorrectly changed in fs_file_t object.";
    ASSERT_EQ(file.inode, inode) << "INode address was incorrectly changed in fs_file_t object.";
    size_t expected_final_offset = offset + expected_size;
    ASSERT_EQ(file.offset, expected_final_offset) << "New file offset is incorrect.";

    check_stdout(OUTPUT "Empty.txt");
    check_fs(INPUT "medium_text.bin", fs);

    free_filesystem(&fs);
}

TEST_F(FSReadSuite, SimpleRead1)
{
    constexpr size_t buffer_size = 42;
    constexpr size_t offset = 80;
    constexpr size_t expected_size = 42;
    constexpr size_t inode_index = 1;

    filesystem_t fs;
    load_fs(INPUT "medium_text.bin", fs);

    inode_t *inode = &fs.inodes[inode_index];
    struct fs_file file {
        &fs,
        inode,
        offset
    };
    char buffer[buffer_size + OVERFLOW] = { 0 };
    size_t output_size;

    { // begin logging stdout
        stdout_logger_lock lk{ this };
        output_size = fs_read(&file, buffer, buffer_size);
    } // stop logging stdout
    
    ASSERT_EQ(output_size, expected_size) << "Bytes read does not match the expected.";

    const char *expected = "t talk for long. Anyway, how are you doing";
    
    size_t i = 0;
    for (; i < expected_size; ++i) 
        ASSERT_EQ(buffer[i], expected[i]) << "Mismatched byte for index " << i;
    for (size_t j = 0; j < OVERFLOW; ++j, ++i) 
        ASSERT_EQ(buffer[i], 0) << "Wrote into overflow buffer.";

    // now check if the file object is updated correctly
    ASSERT_EQ(file.fs, &fs) << "Filesystem address was incorrectly changed in fs_file_t object.";
    ASSERT_EQ(file.inode, inode) << "INode address was incorrectly changed in fs_file_t object.";
    size_t expected_final_offset = offset + expected_size;
    ASSERT_EQ(file.offset, expected_final_offset) << "New file offset is incorrect.";

    check_stdout(OUTPUT "Empty.txt");
    check_fs(INPUT "medium_text.bin", fs);

    free_filesystem(&fs);
}

TEST_F(FSReadSuite, ReadUntilEOF0)
{
    constexpr size_t buffer_size = 42;
    constexpr size_t offset = 580;
    constexpr size_t expected_size = 34;
    constexpr size_t inode_index = 1;

    filesystem_t fs;
    load_fs(INPUT "medium_text.bin", fs);

    inode_t *inode = &fs.inodes[inode_index];
    struct fs_file file {
        &fs,
        inode,
        offset
    };
    char buffer[buffer_size + OVERFLOW] = { 0 };
    size_t output_size;
    
    { // begin logging stdout
        stdout_logger_lock lk{ this };
        output_size = fs_read(&file, buffer, buffer_size);
    } // stop logging stdout

    ASSERT_EQ(output_size, expected_size) << "Bytes read does not match the expected.";

    const char *expected = " this is more than 256 characters!";
    
    size_t i = 0;
    for (; i < expected_size; ++i) 
        ASSERT_EQ(buffer[i], expected[i]) << "Mismatched byte for index " << i;
    for (size_t j = 0; j < OVERFLOW; ++j, ++i) 
        ASSERT_EQ(buffer[i], 0) << "Wrote into overflow buffer.";

    // now check if the file object is updated correctly
    ASSERT_EQ(file.fs, &fs) << "Filesystem address was incorrectly changed in fs_file_t object.";
    ASSERT_EQ(file.inode, inode) << "INode address was incorrectly changed in fs_file_t object.";
    size_t expected_final_offset = offset + expected_size;
    ASSERT_EQ(file.offset, expected_final_offset) << "New file offset is incorrect.";

    check_stdout(OUTPUT "Empty.txt");
    check_fs(INPUT "medium_text.bin", fs);

    free_filesystem(&fs);
}

TEST_F(FSReadSuite, ReadFullFile0)
{
    constexpr size_t buffer_size = 1024;
    constexpr size_t offset = 0;
    constexpr size_t expected_size = 614;
    constexpr size_t inode_index = 1;

    filesystem_t fs;
    load_fs(INPUT "medium_text.bin", fs);

    inode_t *inode = &fs.inodes[inode_index];
    struct fs_file file {
        &fs,
        inode,
        offset
    };
    char buffer[buffer_size + OVERFLOW] = { 0 };
    size_t output_size;
    
    { // begin logging stdout
        stdout_logger_lock lk{ this };
        output_size = fs_read(&file, buffer, buffer_size);
    } // stop logging stdout
    
    ASSERT_EQ(output_size, expected_size) << "Bytes read does not match the expected.";

    const char *expected = "Hi. My name is $@#%^$@. It is a pleasure to meet you, but unfortunately, I cannot talk for long. Anyway, how are you doing? This assignment is sure long, but I personally find it to be very interesting. Anyway, keep at it! There is a lot to learn from this assignment, and I think that is very cool. Anyway, I am rambling at this point. I am suppose to fill this text file with enough characters so that the data is written to the indirect data blocks. I hope this is enough. If not I will be very disappointed because I will have to type another paragraph (or maybe more). Hoping this is more than 256 characters!";
    
    size_t i = 0;
    for (; i < expected_size; ++i) 
        ASSERT_EQ(buffer[i], expected[i]) << "Mismatched byte for index " << i;
    for (size_t j = 0; j < OVERFLOW; ++j, ++i) 
        ASSERT_EQ(buffer[i], 0) << "Wrote into overflow buffer.";

    // now check if the file object is updated correctly
    ASSERT_EQ(file.fs, &fs) << "Filesystem address was incorrectly changed in fs_file_t object.";
    ASSERT_EQ(file.inode, inode) << "INode address was incorrectly changed in fs_file_t object.";
    size_t expected_final_offset = offset + expected_size;
    ASSERT_EQ(file.offset, expected_final_offset) << "New file offset is incorrect.";

    check_stdout(OUTPUT "Empty.txt");
    check_fs(INPUT "medium_text.bin", fs);

    free_filesystem(&fs);
}