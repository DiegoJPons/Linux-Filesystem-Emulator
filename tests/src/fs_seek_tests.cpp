#include "test_util.hpp"

using FSSeekSuite = fs_internal_test;

TEST_F(FSSeekSuite, InvalidInput0)
{
    int ret0, ret1;
    fs_file_t file = (fs_file_t) -1;

    {   // begin logging stdout
        stdout_logger_lock lk{ this };
        ret0 = fs_seek(NULL, FS_SEEK_CURRENT, 0);
        ret1 = fs_seek(file, (seek_mode_t) 100, 0);
    }   // stop logging stdout
    ASSERT_EQ(ret0, -1) << "Incorrect return value for invalid input null fs_file_t.";
    ASSERT_EQ(ret1, -1) << "Incorrect return value for invalid seek mode value.";

    check_stdout(OUTPUT "Empty.txt");
}

// testing FS_SEEK_CURRENT
// no overflow
TEST_F(FSSeekSuite, SeekCurrent0)
{
    constexpr size_t inode_index = 1;
    constexpr size_t offset = 0;
    constexpr seek_mode_t seek_mode = seek_mode_t::FS_SEEK_CURRENT;
    constexpr int seek_amt = 32;

    constexpr size_t expected_offset = 32;

    int ret;
    filesystem_t fs;
    load_fs(INPUT "medium_text.bin", fs);

    inode_t *inode = &fs.inodes[inode_index];
    fs_file file{ &fs, inode, offset };

    {   // begin logging stdout
        stdout_logger_lock lk{ this };
        ret = fs_seek(&file, seek_mode, seek_amt);
    }   // stop logging stdout

    ASSERT_EQ(ret, 0) << "Incorrect return value.";
    ASSERT_EQ(file.fs, &fs) << "File fs pointer should not be modified.";
    ASSERT_EQ(file.inode, inode) << "File inode pointer should not be modified.";
    ASSERT_EQ(file.offset, expected_offset) << "File offset is incorrect.";
    
    check_stdout(OUTPUT "Empty.txt");
    check_fs(INPUT "medium_text.bin", fs);

    free_filesystem(&fs);
}

// testing FS_SEEK_CURRENT
// overflow 
TEST_F(FSSeekSuite, SeekCurrent1)
{
    constexpr size_t inode_index = 1;
    constexpr size_t offset = 600;
    constexpr seek_mode_t seek_mode = seek_mode_t::FS_SEEK_CURRENT;
    constexpr int seek_amt = 32;

    constexpr size_t expected_offset = 614;

    int ret;
    filesystem_t fs;
    load_fs(INPUT "medium_text.bin", fs);

    inode_t *inode = &fs.inodes[inode_index];
    fs_file file{ &fs, inode, offset };

    {   // begin logging stdout
        stdout_logger_lock lk{ this };
        ret = fs_seek(&file, seek_mode, seek_amt);
    }   // stop logging stdout

    ASSERT_EQ(ret, 0) << "Incorrect return value.";
    ASSERT_EQ(file.fs, &fs) << "File fs pointer should not be modified.";
    ASSERT_EQ(file.inode, inode) << "File inode pointer should not be modified.";
    ASSERT_EQ(file.offset, expected_offset) << "File offset is incorrect.";
    
    check_stdout(OUTPUT "Empty.txt");
    check_fs(INPUT "medium_text.bin", fs);

    free_filesystem(&fs);
}

// testing FS_SEEK_CURRENT
// negative offset
TEST_F(FSSeekSuite, SeekCurrent2)
{
    constexpr size_t inode_index = 1;
    constexpr size_t offset = 600;
    constexpr seek_mode_t seek_mode = seek_mode_t::FS_SEEK_CURRENT;
    constexpr int seek_amt = -20;

    constexpr size_t expected_offset = 580;

    int ret;
    filesystem_t fs;
    load_fs(INPUT "medium_text.bin", fs);

    inode_t *inode = &fs.inodes[inode_index];
    fs_file file{ &fs, inode, offset };

    {   // begin logging stdout
        stdout_logger_lock lk{ this };
        ret = fs_seek(&file, seek_mode, seek_amt);
    }   // stop logging stdout

    ASSERT_EQ(ret, 0) << "Incorrect return value.";
    ASSERT_EQ(file.fs, &fs) << "File fs pointer should not be modified.";
    ASSERT_EQ(file.inode, inode) << "File inode pointer should not be modified.";
    ASSERT_EQ(file.offset, expected_offset) << "File offset is incorrect.";
    
    check_stdout(OUTPUT "Empty.txt");
    check_fs(INPUT "medium_text.bin", fs);

    free_filesystem(&fs);
}

// testing FS_SEEK_CURRENT
// zero offset
TEST_F(FSSeekSuite, SeekCurrent3)
{
    constexpr size_t inode_index = 1;
    constexpr size_t offset = 600;
    constexpr seek_mode_t seek_mode = seek_mode_t::FS_SEEK_CURRENT;
    constexpr int seek_amt = 0;

    constexpr size_t expected_offset = 600;

    int ret;
    filesystem_t fs;
    load_fs(INPUT "medium_text.bin", fs);

    inode_t *inode = &fs.inodes[inode_index];
    fs_file file{ &fs, inode, offset };

    {   // begin logging stdout
        stdout_logger_lock lk{ this };
        ret = fs_seek(&file, seek_mode, seek_amt);
    }   // stop logging stdout

    ASSERT_EQ(ret, 0) << "Incorrect return value.";
    ASSERT_EQ(file.fs, &fs) << "File fs pointer should not be modified.";
    ASSERT_EQ(file.inode, inode) << "File inode pointer should not be modified.";
    ASSERT_EQ(file.offset, expected_offset) << "File offset is incorrect.";
    
    check_stdout(OUTPUT "Empty.txt");
    check_fs(INPUT "medium_text.bin", fs);

    free_filesystem(&fs);
}

// testing FS_SEEK_CURRENT
// no overflow
TEST_F(FSSeekSuite, SeekStart0)
{
    constexpr size_t inode_index = 1;
    constexpr size_t offset = 600;
    constexpr seek_mode_t seek_mode = seek_mode_t::FS_SEEK_START;
    constexpr int seek_amt = 0;

    constexpr size_t expected_offset = 0;

    int ret;
    filesystem_t fs;
    load_fs(INPUT "medium_text.bin", fs);

    inode_t *inode = &fs.inodes[inode_index];
    fs_file file{ &fs, inode, offset };

    {   // begin logging stdout
        stdout_logger_lock lk{ this };
        ret = fs_seek(&file, seek_mode, seek_amt);
    }   // stop logging stdout

    ASSERT_EQ(ret, 0) << "Incorrect return value.";
    ASSERT_EQ(file.fs, &fs) << "File fs pointer should not be modified.";
    ASSERT_EQ(file.inode, inode) << "File inode pointer should not be modified.";
    ASSERT_EQ(file.offset, expected_offset) << "File offset is incorrect.";
    
    check_stdout(OUTPUT "Empty.txt");
    check_fs(INPUT "medium_text.bin", fs);

    free_filesystem(&fs);
}

// testing FS_SEEK_CURRENT
// overflow
TEST_F(FSSeekSuite, SeekStart1)
{
    constexpr size_t inode_index = 1;
    constexpr size_t offset = 12;
    constexpr seek_mode_t seek_mode = seek_mode_t::FS_SEEK_START;
    constexpr int seek_amt = 700;

    constexpr size_t expected_offset = 614;

    int ret;
    filesystem_t fs;
    load_fs(INPUT "medium_text.bin", fs);

    inode_t *inode = &fs.inodes[inode_index];
    fs_file file{ &fs, inode, offset };

    {   // begin logging stdout
        stdout_logger_lock lk{ this };
        ret = fs_seek(&file, seek_mode, seek_amt);
    }   // stop logging stdout

    ASSERT_EQ(ret, 0) << "Incorrect return value.";
    ASSERT_EQ(file.fs, &fs) << "File fs pointer should not be modified.";
    ASSERT_EQ(file.inode, inode) << "File inode pointer should not be modified.";
    ASSERT_EQ(file.offset, expected_offset) << "File offset is incorrect.";
    
    check_stdout(OUTPUT "Empty.txt");
    check_fs(INPUT "medium_text.bin", fs);

    free_filesystem(&fs);
}

// testing FS_SEEK_CURRENT
// negative offset
// expect error return value
TEST_F(FSSeekSuite, SeekStart2)
{
    constexpr size_t inode_index = 1;
    constexpr size_t offset = 12;
    constexpr seek_mode_t seek_mode = seek_mode_t::FS_SEEK_START;
    constexpr int seek_amt = -12;

    constexpr size_t expected_offset = 12;

    int ret;
    filesystem_t fs;
    load_fs(INPUT "medium_text.bin", fs);

    inode_t *inode = &fs.inodes[inode_index];
    fs_file file{ &fs, inode, offset };

    {   // begin logging stdout
        stdout_logger_lock lk{ this };
        ret = fs_seek(&file, seek_mode, seek_amt);
    }   // stop logging stdout

    ASSERT_EQ(ret, -1) << "Incorrect return value.";

    ASSERT_EQ(file.fs, &fs) << "File fs pointer should not be modified.";
    ASSERT_EQ(file.inode, inode) << "File inode pointer should not be modified.";
    ASSERT_EQ(file.offset, expected_offset) << "File offset is incorrect.";
    
    check_stdout(OUTPUT "Empty.txt");
    check_fs(INPUT "medium_text.bin", fs);

    free_filesystem(&fs);
}

// testing FS_SEEK_CURRENT
// zero offset
TEST_F(FSSeekSuite, SeekStart3)
{
    constexpr size_t inode_index = 1;
    constexpr size_t offset = 39;
    constexpr seek_mode_t seek_mode = seek_mode_t::FS_SEEK_START;
    constexpr int seek_amt = 0;

    constexpr size_t expected_offset = 0;

    int ret;
    filesystem_t fs;
    load_fs(INPUT "medium_text.bin", fs);

    inode_t *inode = &fs.inodes[inode_index];
    fs_file file{ &fs, inode, offset };

    {   // begin logging stdout
        stdout_logger_lock lk{ this };
        ret = fs_seek(&file, seek_mode, seek_amt);
    }   // stop logging stdout

    ASSERT_EQ(ret, 0) << "Incorrect return value.";
    ASSERT_EQ(file.fs, &fs) << "File fs pointer should not be modified.";
    ASSERT_EQ(file.inode, inode) << "File inode pointer should not be modified.";
    ASSERT_EQ(file.offset, expected_offset) << "File offset is incorrect.";
    
    check_stdout(OUTPUT "Empty.txt");
    check_fs(INPUT "medium_text.bin", fs);

    free_filesystem(&fs);
}

// testing FS_SEEK_END
// no overflow
TEST_F(FSSeekSuite, SeekEnd0)
{
    constexpr size_t inode_index = 1;
    constexpr size_t offset = 32;
    constexpr seek_mode_t seek_mode = seek_mode_t::FS_SEEK_END;
    constexpr int seek_amt = 0;

    constexpr size_t expected_offset = 614;

    int ret;
    filesystem_t fs;
    load_fs(INPUT "medium_text.bin", fs);

    inode_t *inode = &fs.inodes[inode_index];
    fs_file file{ &fs, inode, offset };

    {   // begin logging stdout
        stdout_logger_lock lk{ this };
        ret = fs_seek(&file, seek_mode, seek_amt);
    }   // stop logging stdout

    ASSERT_EQ(ret, 0) << "Incorrect return value.";
    ASSERT_EQ(file.fs, &fs) << "File fs pointer should not be modified.";
    ASSERT_EQ(file.inode, inode) << "File inode pointer should not be modified.";
    ASSERT_EQ(file.offset, expected_offset) << "File offset is incorrect.";
    
    check_stdout(OUTPUT "Empty.txt");
    check_fs(INPUT "medium_text.bin", fs);

    free_filesystem(&fs);
}

// testing FS_SEEK_END
// overflow
TEST_F(FSSeekSuite, SeekEnd1)
{
    constexpr size_t inode_index = 1;
    constexpr size_t offset = 32;
    constexpr seek_mode_t seek_mode = seek_mode_t::FS_SEEK_END;
    constexpr int seek_amt = 122;

    constexpr size_t expected_offset = 614;

    int ret;
    filesystem_t fs;
    load_fs(INPUT "medium_text.bin", fs);

    inode_t *inode = &fs.inodes[inode_index];
    fs_file file{ &fs, inode, offset };

    {   // begin logging stdout
        stdout_logger_lock lk{ this };
        ret = fs_seek(&file, seek_mode, seek_amt);
    }   // stop logging stdout

    ASSERT_EQ(ret, 0) << "Incorrect return value.";
    ASSERT_EQ(file.fs, &fs) << "File fs pointer should not be modified.";
    ASSERT_EQ(file.inode, inode) << "File inode pointer should not be modified.";
    ASSERT_EQ(file.offset, expected_offset) << "File offset is incorrect.";
    
    check_stdout(OUTPUT "Empty.txt");
    check_fs(INPUT "medium_text.bin", fs);

    free_filesystem(&fs);
}

// testing FS_SEEK_END
// negative offset
TEST_F(FSSeekSuite, SeekEnd2)
{
    constexpr size_t inode_index = 1;
    constexpr size_t offset = 32;
    constexpr seek_mode_t seek_mode = seek_mode_t::FS_SEEK_END;
    constexpr int seek_amt = -20;

    constexpr size_t expected_offset = 594;

    int ret;
    filesystem_t fs;
    load_fs(INPUT "medium_text.bin", fs);

    inode_t *inode = &fs.inodes[inode_index];
    fs_file file{ &fs, inode, offset };

    {   // begin logging stdout
        stdout_logger_lock lk{ this };
        ret = fs_seek(&file, seek_mode, seek_amt);
    }   // stop logging stdout

    ASSERT_EQ(ret, 0) << "Incorrect return value.";
    ASSERT_EQ(file.fs, &fs) << "File fs pointer should not be modified.";
    ASSERT_EQ(file.inode, inode) << "File inode pointer should not be modified.";
    ASSERT_EQ(file.offset, expected_offset) << "File offset is incorrect.";
    
    check_stdout(OUTPUT "Empty.txt");
    check_fs(INPUT "medium_text.bin", fs);

    free_filesystem(&fs);
}
