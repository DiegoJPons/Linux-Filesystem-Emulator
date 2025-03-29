cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0076 NEW)
project(hw3 LANGUAGES C CXX)
option(BUILD_CODEGRADE_TESTS "Build test suites into separate executables" OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
include_directories(include)
link_directories(lib)

# vcpkg
# find_package(GTest)

# local tests
if (NOT BUILD_CODEGRADE_TESTS)

    # build the normal executable
    add_executable(hw3_main 
        src/filesys.c 
        src/utility.c
        src/inode_manip.c 
        src/file_operations.c
        src/hw3.c
    )
    target_compile_options(hw3_main PUBLIC -g -D DEBUG -Wall -Wextra -Wshadow -Wdouble-promotion -Wformat=2 -Wundef -Werror -Wno-unused-parameter -Wno-shadow -D_POSIX_C_SOURCE=202503L)
    target_link_libraries(hw3_main PUBLIC m)

    # terminal program
    add_executable(terminal
        src/filesys.c
        src/utility.c 
        src/inode_manip.c 
        src/file_operations.c
        src/terminal.cpp
    )
    target_compile_options(terminal PUBLIC -g -D DEBUG -Wall -Wextra -Wshadow -Wdouble-promotion -Wformat=2 -Wundef -Werror -Wno-unused-parameter -Wno-shadow -D_POSIX_C_SOURCE=202503L)    
    target_compile_definitions(terminal PUBLIC DEBUG)
    target_link_libraries(terminal PUBLIC m)

endif()

# set(GTEST_SUITES 
#     "new_filesystem_tests"
#     "available_inodes_tests"
#     "available_dblocks_tests"
#     "claim_available_inode_tests"
#     "claim_available_dblock_tests"
#     "release_inode_tests"
#     "release_dblock_tests"
#     "inode_write_data_tests" 
#     "inode_read_data_tests"
#     "inode_modify_data_tests"
#     "inode_shrink_data_tests"
#     "new_terminal_tests"
#     "fs_open_tests"
#     "fs_read_tests"
#     "fs_write_tests"
#     "fs_seek_tests"
#     "new_file_tests"
#     "new_directory_tests"
#     "remove_file_tests"
#     "remove_directory_tests"
#     "change_directory_tests"
#     "get_path_string_tests"
#     "list_tests"
#     "tree_tests"
# )
# foreach(TEST IN LISTS GTEST_SUITES)
#     add_executable(${TEST}
#         src/filesys.c
#         src/utility.c
#         src/inode_manip.c
#         src/file_operations.c
#         tests/src/test_util.cpp
#         tests/src/${TEST}.cpp
#     )
#     target_compile_options(${TEST} PUBLIC -g -D DEBUG -Wall -Wextra -Wshadow -Wdouble-promotion -Wformat=2 -Wundef -Werror -Wno-unused-parameter -Wno-shadow)
#     target_include_directories(${TEST} PUBLIC tests/include)
#     target_link_libraries(${TEST} PUBLIC m gtest gtest_main pthread)
# endforeach()

add_executable(part0_tests
    src/filesys.c
    src/utility.c
    tests/src/test_util.cpp
    tests/src/new_filesystem_tests.cpp
    tests/src/available_inodes_tests.cpp
    tests/src/available_dblocks_tests.cpp
    tests/src/claim_available_inode_tests.cpp
    tests/src/claim_available_dblock_tests.cpp
    tests/src/release_inode_tests.cpp
    tests/src/release_dblock_tests.cpp
)
target_compile_options(part0_tests PUBLIC -g -Wall -Wextra -Wshadow -Wdouble-promotion -Wformat=2 -Wundef -Werror -Wno-unused-parameter -Wno-shadow)
target_include_directories(part0_tests PUBLIC tests/include)
target_link_libraries(part0_tests PUBLIC m gtest gtest_main pthread)


add_executable(part1_tests 
    src/filesys.c
    src/utility.c
    src/inode_manip.c
    tests/src/test_util.cpp
    tests/src/inode_write_data_tests.cpp
    tests/src/inode_read_data_tests.cpp
    tests/src/inode_modify_data_tests.cpp
    tests/src/inode_shrink_data_tests.cpp
)
target_compile_options(part1_tests PUBLIC -g -D DEBUG -Wall -Wextra -Wshadow -Wdouble-promotion -Wformat=2 -Wundef -Werror -Wno-unused-parameter -Wno-shadow)
target_include_directories(part1_tests PUBLIC tests/include)
target_link_libraries(part1_tests PUBLIC m gtest gtest_main pthread)

add_executable(part2_tests
    src/filesys.c
    src/utility.c
    src/inode_manip.c
    src/file_operations.c
    tests/src/test_util.cpp
    tests/src/new_terminal_tests.cpp
    tests/src/fs_open_tests.cpp
    tests/src/fs_read_tests.cpp
    tests/src/fs_write_tests.cpp
    tests/src/fs_seek_tests.cpp
)
target_compile_options(part2_tests PUBLIC -g -D DEBUG -Wall -Wextra -Wshadow -Wdouble-promotion -Wformat=2 -Wundef -Werror -Wno-unused-parameter -Wno-shadow)
target_include_directories(part2_tests PUBLIC tests/include)
target_link_libraries(part2_tests PUBLIC m gtest gtest_main pthread)

add_executable(part3_tests
    src/filesys.c
    src/utility.c
    src/inode_manip.c
    src/file_operations.c
    tests/src/test_util.cpp
    tests/src/new_file_tests.cpp
    tests/src/new_directory_tests.cpp
    tests/src/remove_file_tests.cpp
    tests/src/remove_directory_tests.cpp
    tests/src/change_directory_tests.cpp
    tests/src/get_path_string_tests.cpp
    tests/src/list_tests.cpp
    tests/src/tree_tests.cpp
)
target_compile_options(part3_tests PUBLIC -g -D DEBUG -Wall -Wextra -Wshadow -Wdouble-promotion -Wformat=2 -Wundef -Werror -Wno-unused-parameter -Wno-shadow)
target_include_directories(part3_tests PUBLIC tests/include)
target_link_libraries(part3_tests PUBLIC m gtest gtest_main pthread)
