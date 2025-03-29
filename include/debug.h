#ifndef DEBUG_H
#define DEBUG_H

#include <stdlib.h> // for abort

#define INFO_LEVEL 3
#define LOG_EXPR_LEVEL 0

/**
 * !! DO NOT MODIFY ANYTHING BELOW THIS COMMENT !!
 * !! FEEL FREE TO MODIFY THE VALUE OF THE MACROS ABOVE !! 
 */

/*
DEBUG MACROS

these macros will write to stderr. normally when running a file in the terminal, stdout and stderr
are dumped to the terminal. if desired, you can redirect stdout or stderr to a file.

./prog > stdout_file 2> stderr_file

the above will dump the stdout to stdout_file and stderr to stderr_file. 

1. info(level, fmt_str, ...)
    prints outout to stderr if the DEBUG is defined. otherwise does nothing
    specify the level which allows for filtering of info calls via the INFO_LEVEL macro
        any `info` with a level lower than or equal to INFO_LEVEL will print
    the current levels supported for info is 0, 1, 2, and 3

2. fs_expect_success(expr)
    if we would expect `expr` to return SUCCESS, then this macro can be used. In the case
    `expr` does not return SUCCESS, it will issue a default diagnostic message to stderr. 
    The program will continue to execute.

3. fs_expect_success(expr, fmt_str, fmt_args...)
    ditto but a custom message can be outputted to stderr instead

4. fs_assert_success(expr)
    if we would expect `expr` to return SUCCESS, then this macro can be used to detect if
    `expr` ever does not return SUCCESS. If it does not return SUCCESS, then a default diagnostic
    message is issued to stderr. The continue will abort upon the assertion failure.
    
5. fs_assert_success(expr, fmt_str, fmt_args...)
    ditto but a custom message can be outputted to stderr instead
*/

#ifdef TERM_COLOR
    #define RED "\033[31m"
    #define GREEN "\033[32m"
    #define YELLOW "\033[33m"
    #define BLUE "\033[34m"
    #define MAGENTA "\033[35m"
    #define CYAN "\033[36m"
    #define WHITE "\033[37m"
    #define NO_CLR "\033[0m"
#else
    #define RED ""
    #define GREEN ""
    #define YELLOW ""
    #define BLUE ""
    #define MAGENTA ""
    #define CYAN ""
    #define WHITE ""
    #define NO_CLR ""
#endif

// concat two symbols
#define PRIMITIVE_CAT(x, y) x ## y
#define CAT(x, y) PRIMITIVE_CAT(x,y)

// stringify input
#define STR(x) #x
#define DEFER_STR(x) STR(x)

// counts the number of arguments up to 16
#define NARGS_SEQ(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, N, ...) N
#define NARGS(...) NARGS_SEQ(__VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

// evaluates an macro. useful for expanding blue painted macros
// may need to called multiple times to expand some statements
#define EVAL(...) __VA_ARGS__ 

// probe
#define SECOND_ARG(x, n, ...) n
#define CHECK(...) SECOND_ARG(__VA_ARGS__, 0,)
#define PROBE(x) x,1,

// probe to detect if the arguments start with a 1
#define IS_1_1 PROBE(~)
// if is IS_1_XXX, then it evaulates to nothing which is passed to CHECK which will return second arg of (, 0) which is 0
// if is IS_1_1, then it evaluates to (~,1) which is passed to CHECK will will return the second arg of (~, 1, 0) which is 1
// so if x == 1, IS_1 is 1, if x == anything else, then IS_1 is 0
#define IS_1(x) CHECK(PRIMITIVE_CAT(IS_1_, x))

// comparison macros
#define LE_0_0 1
#define LE_0_1 1
#define LE_0_2 1
#define LE_0_3 1
#define LE_1_0 0
#define LE_1_1 1
#define LE_1_2 1
#define LE_1_3 1
#define LE_2_0 0
#define LE_2_1 0
#define LE_2_2 1
#define LE_2_3 1
#define LE_3_0 0
#define LE_3_1 0
#define LE_3_2 0
#define LE_3_3 1

#define PRIMITIVE_CMP_LE(a, b) LE_ ## a ## _ ## b
#define CMP_LE(a, b) PRIMITIVE_CMP_LE(a, b)

// info macro
#ifdef DEBUG
#ifndef INFO_LEVEL
    #define INFO_LEVEL 3
#endif

#define INFO_LEVEL_0 GREEN
#define INFO_LEVEL_1 CYAN
#define INFO_LEVEL_2 MAGENTA
#define INFO_LEVEL_3 BLUE
#define INFO_LEVEL_CLR(x) PRIMITIVE_CAT(INFO_LEVEL_, x)

#define __info_internal_1(level, fmt_str, ...) \
do { \
    fprintf(stderr, INFO_LEVEL_CLR(level) "[" STR(level) "] %s:%s@%d: " NO_CLR fmt_str "\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
} while (0)
#define __info_internal_0(level, fmt_str, ...)

#define __info_level_0(fmt_str, ...) CAT(__info_internal_, CMP_LE(0, INFO_LEVEL))(0, fmt_str, ##__VA_ARGS__);
#define __info_level_1(fmt_str, ...) CAT(__info_internal_, CMP_LE(1, INFO_LEVEL))(1, fmt_str, ##__VA_ARGS__);
#define __info_level_2(fmt_str, ...) CAT(__info_internal_, CMP_LE(2, INFO_LEVEL))(2, fmt_str, ##__VA_ARGS__);
#define __info_level_3(fmt_str, ...) CAT(__info_internal_, CMP_LE(3, INFO_LEVEL))(3, fmt_str, ##__VA_ARGS__);

#define info(level, fmt_str, ...) CAT(__info_level_, level)(fmt_str, ##__VA_ARGS__);
#else
#define info(...)
#endif

// log_expr
#ifdef DEBUG
#ifndef LOG_EXPR_LEVEL
    #define LOG_EXPR_LEVEL 3
#endif
#define log_expr(expr) \
do { \
    info(LOG_EXPR_LEVEL, "Calling %s", STR(expr)); \
    expr; \
} while (0)
#else
#define log_expr(expr) expr
#endif


#ifdef DEBUG

// fs_expect_success macro

#define _fs_expect_success_no_args(expr) \
do { \
    fs_retcode_t ret = expr; \
    if (ret != SUCCESS) \
        fprintf(stderr, RED "%s:%s@%d: " NO_CLR "Statement \"" STR(expr) "\" did not return SUCCESS. Returned code %d: %s" "\n", __FILE__, __FUNCTION__, __LINE__, ret, fs_retcode_string_table[ret]); \
} while (0)

#define _fs_expect_success_args(expr, str, ...) \
do { \
    if ((expr) != SUCCESS) \
        fprintf(stderr, RED "%s:%s@%d: " NO_CLR str "\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
} while (0)

#define FSEXPECT_0 _fs_expect_success_args
#define FSEXPECT_1 _fs_expect_success_no_args

#define fs_expect_success(expr, ...) CAT(FSEXPECT_, IS_1( NARGS(expr, ##__VA_ARGS__) ))(expr, ##__VA_ARGS__)

// fs_assert_success_macro

#define _fs_assert_success_no_args(expr) \
do { \
    fs_retcode_t ret = expr; \
    if (ret != SUCCESS) { \
        fprintf(stderr, RED "%s:%s@%d: " NO_CLR "Assertion failed. Statement \"" STR(expr) "\" did not return SUCCESS. Returned code %d: %s. ABORTING." "\n", __FILE__, __FUNCTION__, __LINE__, ret, fs_retcode_string_table[ret]); \
        abort(); \
    } \
} while (0)

#define _fs_assert_success_args(expr, str, ...) \
do { \
    fs_retcode_t ret = expr; \
    if (ret != SUCCESS) { \
        fprintf(stderr, RED "%s:%s@%d: " NO_CLR "Assertion failed. " str "ABORTING." "\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        abort(); \
    } \
} while (0)

#define FSASSERT_0 _fs_assert_success_args
#define FSASSERT_1 _fs_assert_success_no_args

#define fs_assert_success(expr, ...) CAT(FSASSERT_, IS_1( NARGS(expr, ##__VA_ARGS__) ))(expr, ##__VA_ARGS__)

#else
#define fs_expect_success(expr, ...) expr
#define fs_assert_success(expr, ...) expr
#endif

#endif