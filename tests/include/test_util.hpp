#ifndef TEST_UTIL_HPP
#define TEST_UTIL_HPP

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <gtest/gtest.h>

extern "C"
{
    #include "filesys.h"
    #include "debug.h"
}

#define INPUT "input/"
#define OUTPUT "tests/output/"
#define HIDDEN_OUTPUT "tests/output/hidden/"

#define PATH(path) std::string{ path }.data()

void compare_fs_files(char *output_buf, size_t output_size, char *expected_buf, size_t expected_size);

template<typename Test>
struct stdout_logger_lock
{
    Test *test;

    stdout_logger_lock(Test *test_ptr) : test{ test_ptr }
    {
        dup2( fileno(test->stdout_file), STDOUT_FILENO ); 
    }

    ~stdout_logger_lock()
    {
        fflush(stdout);
        dup2( test->stdout_copy, STDOUT_FILENO );
    }
};

template<typename Test>
stdout_logger_lock(Test *test_ptr) -> stdout_logger_lock<Test>; 

class fs_internal_test : public testing::Test
{
public:
    FILE *stdout_file;
    FILE *stderr_file;
    int stdout_copy = -1;
    int stderr_copy = -1;
protected:
    FILE *output_file; 

    fs_internal_test() : output_file{ nullptr } {}
    ~fs_internal_test() = default;

    void SetUp() override
    {
        output_file = tmpfile();
        ASSERT_NE(output_file, nullptr) << "System Error. Failed to open temporary file.";

        stdout_file = tmpfile();
        stderr_file = tmpfile();

        ASSERT_NE(stdout_file, nullptr) << "System Error. Failed to open temporary file.";
        ASSERT_NE(stderr_file, nullptr) << "System Error. Failed to open temporary file.";

        stdout_copy = dup(STDOUT_FILENO);
    }

    void TearDown() override
    {
        fclose(output_file);
        fclose(stdout_file);
        fclose(stderr_file);

        close(stdout_copy);
        close(stderr_copy);
    }

    void load_fs(const char *fs_name, filesystem_t& fs)
    {
        FILE *fs_file = fopen(fs_name, "r");
        ASSERT_NE(fs_file, nullptr) << "File for input is not found.";
        ASSERT_EQ(load_filesystem(fs_file, &fs), SUCCESS) << "Loading the file system was not successful.";

        fclose(fs_file);
    }

    void compare_expected(const char *expected_filename)
    {
        fflush(output_file);

        FILE *expected = fopen(expected_filename, "r");
        ASSERT_NE(expected, nullptr) << "File for expected output not found.";

        struct stat expected_stat, output_stat;
        ASSERT_NE(fstat(fileno(expected), &expected_stat), -1) << "System Error. stat failed.";
        ASSERT_NE(fstat(fileno(output_file), &output_stat), -1) << "System Error. stat failed.";

        size_t expected_size = expected_stat.st_size;
        size_t output_size = output_stat.st_size;
        
        // now memory map the files for easy comparison
        char *expected_bytes = (char*) mmap(nullptr, expected_size, PROT_READ, MAP_PRIVATE, fileno(expected), 0);
        ASSERT_NE(expected_bytes, nullptr) << "System Error. mmap failed.";
        char *output_bytes = (char*) mmap(nullptr, output_size, PROT_READ, MAP_PRIVATE, fileno(output_file), 0);
        ASSERT_NE(output_bytes, nullptr) << "System Error. mmap failed.";

        compare_fs_files(output_bytes, output_size, expected_bytes, expected_size);

        munmap(expected_bytes, expected_size);
        munmap(output_bytes, output_size);
        fclose(expected);
    }

    void check_stdout(const char *expected_filename)
    {
        // set beginning of file
        fseek(stdout_file, 0, SEEK_SET);
        FILE *expected_file = fopen(expected_filename, "r");
        ASSERT_NE(expected_file, nullptr) << "File for expected stdout output not found.";

        char *output_line = NULL;
        size_t output_buf_sz = 0;
        char *expected_line = NULL;
        size_t expected_buf_sz = 0;

        size_t line = 0;
        while (true)
        {
            auto output_line_sz = getline(&output_line, &output_buf_sz, stdout_file);
            auto expected_line_sz = getline(&expected_line, &expected_buf_sz, expected_file);       
            ++line;     

            if (output_line_sz == -1 && expected_line_sz == -1) break;

            ASSERT_FALSE(output_line_sz == -1 || expected_line_sz == -1) << "Missing line " << line;
            ASSERT_STREQ(output_line, expected_line) << "Incorrect for line " << line;
        }

        fclose(expected_file);
        fseek(stdout_file, 0, SEEK_END);
    }

    void check_fs(const char *expected_filename, filesystem_t& fs)
    {
        ASSERT_EQ(save_filesystem(output_file, &fs), SUCCESS) << "Failed to save the file system.";
        compare_expected(expected_filename);
    }
};

#endif