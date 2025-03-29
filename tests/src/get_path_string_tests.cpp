#include "test_util.hpp"

using PathStringSuite = fs_internal_test;

TEST_F(PathStringSuite, InvalidInput)
{
    constexpr const char *expected_path_string = "";
    char *output_path_string = get_path_string(NULL);
    ASSERT_STREQ(output_path_string, expected_path_string) << "Path strings do not match.";

    free(output_path_string); 
}


TEST_F(PathStringSuite, PathString0)
{
    constexpr size_t inode_index = 0;
    constexpr const char* expected_path_string = "root";

    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);
    terminal_context_t ctx { &fs, &fs.inodes[inode_index] };
    char *output_path_string = get_path_string(&ctx);
    ASSERT_STREQ(output_path_string, expected_path_string) << "Path strings do not match.";

    free(output_path_string);
    free_filesystem(&fs);
}

TEST_F(PathStringSuite, PathString1)
{
    constexpr size_t inode_index = 3;
    constexpr const char* expected_path_string = "root/a/b/c";

    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);
    terminal_context_t ctx { &fs, &fs.inodes[inode_index] };
    char *output_path_string = get_path_string(&ctx);
    ASSERT_STREQ(output_path_string, expected_path_string) << "Path strings do not match.";

    free(output_path_string);
    free_filesystem(&fs);
}

TEST_F(PathStringSuite, PathString2)
{
    constexpr size_t inode_index = 7;
    constexpr const char* expected_path_string = "root/a/d";

    filesystem_t fs;
    load_fs(INPUT "medium.bin", fs);
    terminal_context_t ctx { &fs, &fs.inodes[inode_index] };
    char *output_path_string = get_path_string(&ctx);
    ASSERT_STREQ(output_path_string, expected_path_string) << "Path strings do not match.";

    free(output_path_string);
    free_filesystem(&fs);
}