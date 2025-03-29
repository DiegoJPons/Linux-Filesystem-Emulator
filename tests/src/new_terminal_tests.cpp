#include "test_util.hpp"

using NewTerminalSuite = fs_internal_test;

// test for null pointer input
TEST_F(NewTerminalSuite, InvalidInput0)
{
    // dummy data for testing
    terminal_context_t ctx{ 
        (filesystem_t*) 0x12345678,
        (inode_t*) 0x87654321
    };

    new_terminal(NULL, &ctx);

    ASSERT_EQ( ctx.fs, (filesystem_t*) 0x12345678 );
    ASSERT_EQ( ctx.working_directory, (inode_t*) 0x87654321 );
}

TEST_F(NewTerminalSuite, Terminal0)
{
    filesystem_t fs;
    load_fs(INPUT "tiny_full.bin", fs);

    terminal_context_t term;
    new_terminal(&fs, &term);
    ASSERT_EQ(term.fs, &fs);
    ASSERT_EQ(term.working_directory, &fs.inodes[0]);

    check_fs(INPUT "tiny_full.bin", fs);
    free_filesystem(&fs);
}
