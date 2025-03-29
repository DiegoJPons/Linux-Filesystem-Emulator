#include "test_util.hpp"

using INodeReadDataSuite = fs_internal_test;

constexpr inline std::size_t OVERFLOW = 128;

// check for basic invalid inputs
TEST_F(INodeReadDataSuite, InvalidInput)
{
    filesystem_t fs;
    new_filesystem(&fs, 1, 1);
    inode_t *root = &fs.inodes[0];
    size_t i;

    ASSERT_EQ( inode_read_data(&fs, NULL, 0, NULL, 0, &i), INVALID_INPUT );

    ASSERT_EQ( inode_read_data(NULL, root, 0, NULL, 0, &i), INVALID_INPUT );

    ASSERT_EQ( inode_read_data(&fs, root, 0, NULL, 0, NULL), INVALID_INPUT );

    free_filesystem(&fs);
}

// reading from direct dblock
TEST_F(INodeReadDataSuite, ReadData0)
{
    filesystem_t fs;
    load_fs(INPUT "medium_text.bin", fs);

    char expected[] = "Hi. My name is $@#%^$@.";
    char output[std::size(expected) - 1 + OVERFLOW] = { 0 };

    inode_t *hi_file = &fs.inodes[1];
    size_t bytes_read = 234232;
    ASSERT_EQ(inode_read_data(&fs, hi_file, 0, output, std::size(expected) - 1, &bytes_read), SUCCESS);
    ASSERT_EQ(bytes_read, std::size(expected) - 1) << "Incorrect number of bytes read from file.";

    // now check the message
    size_t idx = 0;
    for (; idx < std::size(expected) - 1; ++idx)
        ASSERT_EQ(expected[idx], output[idx]) << "Incorrect for index " << idx;
    for (; idx < std::size(output); ++idx)
        ASSERT_EQ(output[idx], 0) << "Wrote into overflow buffer.";

    check_fs(INPUT "medium_text.bin", fs); // no changes shouldve been made to the file system

    free_filesystem(&fs);
}

// reading from indirect dblock
TEST_F(INodeReadDataSuite, ReadData1)
{
    filesystem_t fs;
    load_fs(INPUT "medium_text.bin", fs);

    char expected[] = " assignment, and I think that is very cool. Anyway, I am rambling at this point. I am suppose to fill this text file with enough characters so that the data is written to the indirect data blocks. I hope this is enough.";
    char output[std::size(expected) - 1 + OVERFLOW] = { 0 };

    inode_t *hi_file = &fs.inodes[1];
    size_t bytes_read = 234232;
    ASSERT_EQ(inode_read_data(&fs, hi_file, 256, output, std::size(expected) - 1, &bytes_read), SUCCESS);
    ASSERT_EQ(bytes_read, std::size(expected) - 1) << "Incorrect number of bytes read from file.";

    // now check the message
    size_t idx = 0;
    for (; idx < std::size(expected) - 1; ++idx)
        ASSERT_EQ(expected[idx], output[idx]) << "Incorrect for index " << idx;
    for (; idx < std::size(output); ++idx)
        ASSERT_EQ(output[idx], 0) << "Wrote into overflow buffer.";

    check_fs(INPUT "medium_text.bin", fs); // no changes shouldve been made to the file system

    free_filesystem(&fs);
}

// reading direct and indirect dblock
TEST_F(INodeReadDataSuite, ReadData2)
{
    filesystem_t fs;
    load_fs(INPUT "medium_text.bin", fs);

    char expected[] = "Hi. My name is $@#%^$@. It is a pleasure to meet you, but unfortunately, I cannot talk for long. Anyway, how are you doing? This assignment is sure long, but I personally find it to be very interesting. Anyway, keep at it! There is a lot to learn from this assignment, and I think that is very cool. Anyway, I am rambling at this point. I am suppose to fill this text file with enough characters so that the data is written to the indirect data blocks. I hope this is enough. If not I will be very disappointed because I will have to type another paragraph (or maybe more). Hoping this is more than 256 characters!";
    char output[std::size(expected) - 1 + OVERFLOW] = { 0 };

    inode_t *hi_file = &fs.inodes[1];
    size_t bytes_read = 234232;
    ASSERT_EQ(inode_read_data(&fs, hi_file, 0, output, 100000000, &bytes_read), SUCCESS);
    ASSERT_EQ(bytes_read, std::size(expected) - 1) << "Incorrect number of bytes read from file.";
    
    // now check the message
    size_t idx = 0;
    for (; idx < std::size(expected) - 1; ++idx)
        ASSERT_EQ(expected[idx], output[idx]) << "Incorrect for index " << idx;
    for (; idx < std::size(output); ++idx)
        ASSERT_EQ(output[idx], 0) << "Wrote into overflow buffer.";

    check_fs(INPUT "medium_text.bin", fs); // no changes shouldve been made to the file system

    free_filesystem(&fs);
}