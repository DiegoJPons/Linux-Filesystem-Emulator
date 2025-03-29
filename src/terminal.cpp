#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <memory>

/**
 * !! DO NOT MODIFY THIS FILE !!
 */

extern "C"
{
    #include "filesys.h"
    #include "debug.h"
}

template<typename CharT>
std::vector<std::basic_string_view<CharT>> split_string(std::basic_string<CharT>& str, CharT sep)
{
    std::vector<std::basic_string_view<CharT>> parts;
    CharT *begin = nullptr; 
    size_t len = 0;
    for (size_t i = 0; i < str.size() + 1; ++i)
    {
        CharT c = str[i];
        if (c != sep && c != 0)
        {
            if (!begin) 
            {
                begin = &str[i];
                len = 0;
            }
            len++;
        }
        else if (begin)
        {
            parts.push_back( std::basic_string_view<CharT>{ begin, len } );
            begin = nullptr;
        }
    }
    return parts;
}

template<typename... Commands>
class stdin_interpreter 
{
    std::string line;
public:
    explicit stdin_interpreter() {}

    stdin_interpreter(const stdin_interpreter&) = delete;
    stdin_interpreter(stdin_interpreter&&) = delete;
    stdin_interpreter& operator=(const stdin_interpreter&) = delete;
    stdin_interpreter& operator=(stdin_interpreter&&) = delete;
    ~stdin_interpreter() = default;

    bool help(const std::vector<std::string_view>& args);
    bool prompt();
    void start();
};

template<typename... Commands>
class source_interpreter 
{
    FILE *file;
    char *line = nullptr;
    size_t buf_size = 0;
public:
    explicit source_interpreter(char *source_file) 
    {
        file = fopen(source_file, "r");
        if (!file) throw std::invalid_argument{ "Source file does not exist" };
    }

    source_interpreter(const source_interpreter&) = delete;
    source_interpreter(source_interpreter&&) = delete;
    source_interpreter& operator=(const source_interpreter&) = delete;
    source_interpreter& operator=(source_interpreter&&) = delete;
    ~source_interpreter()
    {
        fclose(file);
        free(line);
    }

    bool prompt();
    void start();
};

// ------------------------ ENVIRONMENT VARIABLES ------------------------------ //

constexpr size_t default_inode_count = 32;
constexpr size_t default_dblock_count = 64;

class fs_env
{
private:
    filesystem_t fs;

    fs_env()
    {
        if (new_filesystem(&fs, default_inode_count, default_dblock_count) != SUCCESS)
        {
            puts("Initialization error in fs_env. Cannot recover. Aborting.");
            std::abort();
        }
    }
public:
    fs_env(const fs_env&) = delete;
    fs_env(fs_env&&) = delete;
    fs_env& operator=(const fs_env&) = delete;
    fs_env& operator=(fs_env&&) = delete;
    ~fs_env() = default;

    static fs_env& instance()
    {
        static fs_env env;
        return env;
    }

    filesystem_t& get() { return fs; }
};

class terminal_env
{
private:
    terminal_context_t ctx;

    terminal_env()
    {
        new_terminal(&fs_env::instance().get(), &ctx);
    }
public:
    terminal_env(const terminal_env&) = delete;
    terminal_env(terminal_env&&) = delete;
    terminal_env& operator=(const terminal_env&) = delete;
    terminal_env& operator=(terminal_env&&) = delete;
    ~terminal_env() = default;

    static terminal_env& instance()
    {
        static terminal_env env;
        return env;
    }

    terminal_context_t& get() { return ctx; }
};

// ------------------------ TERMINAL COMMANDS ------------------------------ //

struct load_fs_command
{
    static constexpr std::size_t help_message_len = 2;
    static const char* const help_messages[help_message_len];

    static bool exec(const std::vector<std::string_view>& args)
    {
        using namespace std::string_view_literals;
        if (args[0].compare("load"sv) != 0) return false;

        if (args.size() != 2)
        {
            puts("Incorrect number of arguments for load.");
            return true;
        }

        std::string file_name{ args[1] };
        FILE *file = fopen(file_name.data(), "r");
        if (!file)
        {
            printf("File with name %s does not exist.\n", file_name.data());
            return true;
        }
        
        filesystem_t copy;
        fs_retcode_t ret = load_filesystem(file, &copy);
        if (ret != SUCCESS) 
        {
            REPORT_RETCODE(ret);
            return true;
        }
        
        free_filesystem(&fs_env::instance().get());
        fs_env::instance().get() = copy;
        new_terminal(&fs_env::instance().get(), &terminal_env::instance().get());
        fclose(file);
        return true;
    }   
};

const char * const load_fs_command::help_messages[help_message_len] = {
    "load path_to_fs_binary",
    "\tLoads a file system from a binary file."
};

struct save_fs_command
{
    static constexpr std::size_t help_message_len = 2;
    static const char* const help_messages[help_message_len];

    static bool exec(const std::vector<std::string_view>& args)
    {
        using namespace std::string_view_literals;
        if (args[0].compare("save"sv) != 0) return false;

        if (args.size() != 2)
        {
            puts("Incorrect number of arguments for save.");
            return true;
        }

        std::string file_name{ args[1] };
        FILE *file = fopen(file_name.data(), "w");
        if (!file)
        {
            printf("Unexpected error occurred when opening file %s\n", file_name.data());
            return true;
        }
        
        save_filesystem(file, &fs_env::instance().get());
        fclose(file);
        return true;
    }  
};

const char * const save_fs_command::help_messages[help_message_len] = {
    "save path_to_new_fs_binary",
    "\tSaves a file system to a binary file."
};

struct new_fs_command
{
    static constexpr std::size_t help_message_len = 2;
    static const char* const help_messages[help_message_len];

    static bool exec(const std::vector<std::string_view>& args)
    {
        using namespace std::string_view_literals;
        if (args[0].compare("new"sv) != 0) return false;

        if (args.size() != 3)
        {
            puts("Incorrect number of arguments for new.");
            return true;
        }

        size_t inode_count, dblock_count; 
        try
        {
            inode_count = std::stoul(std::string{ args[1] });
            dblock_count = std::stoul(std::string{ args[2] });
        }
        catch (std::invalid_argument&)
        {
            puts("Argument is not an unsigned integer type.");
            return true;
        }
        
        free_filesystem(&fs_env::instance().get());
        new_filesystem(&fs_env::instance().get(), inode_count, dblock_count);
        new_terminal(&fs_env::instance().get(), &terminal_env::instance().get());
        return true;
    }  
};

const char * const new_fs_command::help_messages[help_message_len] = {
    "new num_of_inodes num_of_dblocks",
    "\tCreates a new empty file system with `num_of_inodes` inodes and `num_of_dblocks` dblocks."
};

struct display_fs_command
{
    static constexpr std::size_t help_message_len = 2;
    static const char* const help_messages[help_message_len];

    static bool exec(const std::vector<std::string_view>& args)
    {
        using namespace std::string_view_literals;
        if (args[0].compare("fs"sv) != 0) return false;

        if (args.size() != 1)
        {
            puts("Incorrect number of arguments for display.");
            return true;
        }

        display_filesystem(&fs_env::instance().get(), DISPLAY_ALL);
        return true;
    }
};

const char * const display_fs_command::help_messages[help_message_len] = {
    "fs",
    "\tDisplays all file system information."
};

struct available_command
{
    static constexpr std::size_t help_message_len = 2;
    static const char* const help_messages[help_message_len];

    static bool exec(const std::vector<std::string_view>& args)
    {
        using namespace std::string_view_literals;
        if (args[0].compare("available"sv) != 0) return false;

        display_filesystem(&fs_env::instance().get(), DISPLAY_FS_FORMAT);
        return true;
    } 
};

const char * const available_command::help_messages[help_message_len] = {
    "available",
    "\tDisplays the number of available inodes and dblocks in the file system."
};

struct ls_command
{
    static constexpr std::size_t help_message_len = 3;
    static const char* const help_messages[help_message_len];

    static bool exec(const std::vector<std::string_view>& args)
    {
        using namespace std::string_view_literals;
        if (args[0].compare("ls"sv) != 0) return false;

        if (args.size() > 2)
        {
            puts("Incorrect number of arguments for ls.");
            return true;
        }

        if (args.size() == 1) list(&terminal_env::instance().get(), std::string{ "." }.data());
        else list(&terminal_env::instance().get(), std::string{ args[1] }.data());

        return true;
    } 
};

const char * const ls_command::help_messages[help_message_len] = {
    "ls path",
    "\tIf the file at path is a directory, display the content of the directory.",
    "\tIf the file at path is a data file, display the file entry."
};

struct tree_command
{
    static constexpr std::size_t help_message_len = 3;
    static const char* const help_messages[help_message_len];

    static bool exec(const std::vector<std::string_view>& args)
    {
        using namespace std::string_view_literals;
        if (args[0].compare("tree"sv) != 0) return false;

        if (args.size() > 2)
        {
            puts("Incorrect number of arguments for tree.");
            return true;
        }

        if (args.size() == 1) tree(&terminal_env::instance().get(), std::string{ "." }.data());
        else tree(&terminal_env::instance().get(), std::string{ args[1] }.data());

        return true;
    }
};

const char * const tree_command::help_messages[help_message_len] = {
    "tree path",
    "\tIf the file at path is a directory, display the tree representation starting from the directory.",
    "\tIf the file at path is a data file, display the tree representation starting from the file."
};

struct new_file_command
{
    static constexpr std::size_t help_message_len = 3;
    static const char* const help_messages[help_message_len];

    static bool exec(const std::vector<std::string_view>& args)
    {
        using namespace std::string_view_literals;
        if (args[0].compare("newfile"sv) != 0) return false;

        if (args.size() != 3)
        {
            puts("Incorrect number of arguments for newfile.");
            return true;
        }

        std::string filename{ args[1] };
        size_t perms;

        try
        {
            perms = std::stoul(std::string{ args[2] });
        }
        catch (std::invalid_argument&)
        {
            puts("Argument for the permission is not valid.");
            return true;
        }

        new_file(&terminal_env::instance().get(), filename.data(), (permission_t) perms);
        return true;
    }
};

const char * const new_file_command::help_messages[help_message_len] = {
    "newfile path_to_new_file perms",
    "\tCreates a new empty data file at the location `path_to_new_file` with permissions `perms`.",
    "\t`perms` is the decimal integer value of the permission bitmask. READ = 1 / WRITE = 2 / EXECUTE = 4."
};


struct new_directory_command
{
    static constexpr std::size_t help_message_len = 2;
    static const char* const help_messages[help_message_len];

    static bool exec(const std::vector<std::string_view>& args)
    {
        using namespace std::string_view_literals;
        if (args[0].compare("newdir"sv) != 0) return false;

        if (args.size() != 2)
        {
            puts("Incorrect number of arguments for newdir.");
            return true;
        }

        std::string filename{ args[1] };

        new_directory(&terminal_env::instance().get(), filename.data());
        return true;
    }
};

const char * const new_directory_command::help_messages[help_message_len] = {
    "newfile path_to_new_file perms",
    "\tCreates a new empty data file at the location `path_to_new_file` with permissions `perms`."
};

struct remove_file_command
{
    static constexpr std::size_t help_message_len = 2;
    static const char* const help_messages[help_message_len];

    static bool exec(const std::vector<std::string_view>& args)
    {
        using namespace std::string_view_literals;
        if (args[0].compare("rmfile"sv) != 0) return false;

        if (args.size() != 2)
        {
            puts("Incorrect number of arguments for rmfile.");
            return true;
        }

        std::string filename{ args[1] };

        remove_file(&terminal_env::instance().get(), filename.data());
        return true;
    }
};

const char * const remove_file_command::help_messages[help_message_len] = {
    "rmfile path_to_file",
    "\tDeletes a data file at the location `path_to_file`."
};

struct remove_dir_command
{
    static constexpr std::size_t help_message_len = 2;
    static const char* const help_messages[help_message_len];

    static bool exec(const std::vector<std::string_view>& args)
    {
        using namespace std::string_view_literals;
        if (args[0].compare("rmdir"sv) != 0) return false;

        if (args.size() != 2)
        {
            puts("Incorrect number of arguments for rmdir.");
            return true;
        }

        std::string filename{ args[1] };

        remove_directory(&terminal_env::instance().get(), filename.data());
        return true;
    }
};

const char * const remove_dir_command::help_messages[help_message_len] = {
    "rmdir path_to_file",
    "\tDeletes a directory at the location `path_to_file`."
};

struct cd_command
{
    static constexpr std::size_t help_message_len = 2;
    static const char* const help_messages[help_message_len];

    static bool exec(const std::vector<std::string_view>& args)
    {
        using namespace std::string_view_literals;
        if (args[0].compare("cd"sv) != 0) return false;

        if (args.size() != 2)
        {
            puts("Incorrect number of arguments for cd.");
            return true;
        }

        std::string filename{ args[1] };

        change_directory(&terminal_env::instance().get(), filename.data());
        return true;
    }
};

const char * const cd_command::help_messages[help_message_len] = {
    "cd path_to_dir",
    "\tChanges the current working directory to `path_to_dir` relative to the current working directory."
};

struct write_command
{
    static constexpr std::size_t help_message_len = 2;
    static const char* const help_messages[help_message_len];

    static bool exec(const std::vector<std::string_view>& args)
    {
        using namespace std::string_view_literals;
        if (args[0].compare("write"sv) != 0) return false;

        if (args.size() != 2)
        {
            puts("Incorrect number of arguments for write.");
            return true;
        }

        std::string filename{ args[1] };

        fs_file_t f = fs_open(&terminal_env::instance().get(), filename.data());
        if (!f) return true;
        
        printf("Enter your input to write here: ");

        std::string input;
        std::getline(std::cin, input);
        fs_write(f, input.data(), input.size());
        fs_close(f);

        return true;
    }
};

const char * const write_command::help_messages[help_message_len] = {
    "write path_to_file",
    "\tWrite to the data file at `path_to_file`."
};

struct cat_command
{
    static constexpr std::size_t help_message_len = 2;
    static const char* const help_messages[help_message_len];

    static bool exec(const std::vector<std::string_view>& args)
    {
        using namespace std::string_view_literals;
        if (args[0].compare("cat"sv) != 0) return false;

        if (args.size() != 2)
        {
            puts("Incorrect number of arguments for cat.");
            return true;
        }

        std::string filename{ args[1] };

        fs_file_t f = fs_open(&terminal_env::instance().get(), filename.data());
        if (!f) return true;

        size_t file_sz = f->inode->internal.file_size;
        std::unique_ptr<char[]> buf{ new char[file_sz + 1]{ 0 } };
        fs_read(f, buf.get(), file_sz);
        fs_close(f);

        printf("%s\n", buf.get());

        return true;
    }
};

const char * const cat_command::help_messages[help_message_len] = {
    "cat path_to_file",
    "\tPrints the data in the data file at `path_to_file` to the terminal."
};

struct dump_command
{
    static constexpr std::size_t help_message_len = 2;
    static const char* const help_messages[help_message_len];

    constexpr static char MARKER = 0x20; // nonzero to be distinguished

    static bool exec(const std::vector<std::string_view>& args)
    { 
        using namespace std::string_view_literals;
        if (args[0].compare("dump"sv) != 0) return false;

        if (args.size() != 3)
        {
            puts("Incorrect number of arguments for dump.");
            return true;
        }

        std::string filename{ args[1] };

        fs_file_t f = fs_open(&terminal_env::instance().get(), filename.data());
        if (!f) return true;

        size_t count;
        try
        {
            count = std::stoul(std::string{ args[2] });
        }
        catch (std::invalid_argument&)
        {
            puts("Argument for the count is not valid.");
            return true;
        }

        std::unique_ptr<char[]> buf{ new char[count] };
        for (size_t i = 0; i < count; ++i)
        {
            buf[i] = MARKER;
        }
        int ret = fs_write(f, buf.get(), count);
        if (ret == 0 && count != 0) puts("Error: dump failed.");

        fs_close(f);

        return true;
    }
};

const char * const dump_command::help_messages[help_message_len] = {
    "dump path_to_file num_of_bytes",
    "\tDumps the `num_of_bytes` bytes of the value 0x20 into in the data file at `path_to_file`."
};

struct patch_command
{
    static constexpr std::size_t help_message_len = 2;
    static const char* const help_messages[help_message_len];

    // patch addr offset n value
    static bool exec(const std::vector<std::string_view>& args)
    { 
        using namespace std::string_view_literals;
        if (args[0].compare("patch"sv) != 0) return false;

        if (args.size() != 5)
        {
            puts("Incorrect number of arguments for patch.");
            return true;
        }

        std::string filename{ args[1] };

        fs_file_t f = fs_open(&terminal_env::instance().get(), filename.data());
        if (!f) return true;

        size_t offset;
        size_t n;
        size_t value;
        try
        {
            offset = std::stoul(std::string{ args[2] });
            n = std::stoul(std::string{ args[3] });
            value = std::stoul(std::string{ args[4] });
        }
        catch (std::invalid_argument&)
        {
            puts("Argument are not integers.");
            return true;
        }

        fs_seek(f, FS_SEEK_START, offset);
        std::unique_ptr<char[]> buf{ new char[n] };
        for (size_t i = 0; i < n; ++i)
        {
            buf[i] = static_cast<char>(value);
        }
        int ret = fs_write(f, buf.get(), n);
        if (ret == 0 && n != 0) puts("Error: patch failed.");
        fs_close(f);

        return true;
    }
};

const char * const patch_command::help_messages[help_message_len] = {
    "patch path_to_file offset num_of_bytes value",
    "\tDumps the `num_of_bytes` bytes of the value `value` into in the data file at `path_to_file` starting at offset `offset`."
};

template<typename Command>
void display_command()
{
    for (size_t i = 0; i < Command::help_message_len; ++i)
    {
        puts(Command::help_messages[i]);
    }    
    puts("");
}

template<typename... Commands>
bool stdin_interpreter<Commands...>::help(const std::vector<std::string_view>& args)
{
    if (args[0].compare("help") != 0) return false;

    int sink[]{ (display_command<Commands>(), 0)... };
    (void) sink;

    return true;
}

template<typename... Commands>
bool stdin_interpreter<Commands...>::prompt()
{   
    char *path_name = get_path_string(&terminal_env::instance().get());
    printf("%s > ", path_name);
    free(path_name);

    return static_cast<bool>(std::getline(std::cin, line));
}

template<typename... Commands>
void stdin_interpreter<Commands...>::start()
{
    while (prompt())
    {
        if (!line.empty())
        {
            auto args = split_string(line, ' ');
            if (args.size() == 0) continue;         

            bool found_match = (help(args) || ... || Commands::exec(args));
            if (!found_match) printf("Command %s not found\n", std::string{ args[0] }.data());
        }
    }
}

template<typename... Commands>
bool source_interpreter<Commands...>::prompt()
{   
    auto ret = getline(&line, &buf_size, file);
    if (ret == -1) return false;

    char *path_name = get_path_string(&terminal_env::instance().get());
    printf("%s > %s", path_name, line);
    free(path_name);

    return true;
}

template<typename... Commands>
void source_interpreter<Commands...>::start()
{
    while (prompt())
    {
        size_t line_len = strlen(line);
        if (line_len)
        {
            if (line[line_len - 1] == '\n') line[line_len - 1] = 0; // remove newline

            std::string line_str{ line };
            auto args = split_string(line_str, ' ');
            if (args.size() == 0) continue;         

            bool found_match = (... || Commands::exec(args));
            if (!found_match) printf("Command %s not found\n", std::string{ args[0] }.data());
        }
    }
}


int main(int argc, char *argv[])
{
    if (argc > 2)
    {
        printf("Invalid number of arguments\n");
        return 1;
    }

    if (argc == 1)
    {
        stdin_interpreter<
            load_fs_command, 
            save_fs_command, 
            new_fs_command,
            display_fs_command,
            available_command,
            ls_command,
            tree_command,
            new_file_command,
            new_directory_command,
            remove_file_command,
            remove_dir_command,
            cd_command,
            write_command,
            cat_command,
            dump_command,
            patch_command
        >{}.start();
    }
    else
    {
        source_interpreter<
            load_fs_command, 
            save_fs_command, 
            new_fs_command,
            display_fs_command,
            available_command,
            ls_command,
            tree_command,
            new_file_command,
            new_directory_command,
            remove_file_command,
            remove_dir_command,
            cd_command,
            cat_command,
            dump_command,
            patch_command
        >{ argv[1] }.start();
    }

    return 0;
}