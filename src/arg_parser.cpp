#include "../include/renamer.h"
#include <iostream>
#include <stdexcept>
#include <cstring>

void ArgParser::printHelp() {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "           Batch File Renamer Tool v1.0\n";
    std::cout << std::string(60, '=') << "\n\n";
    
    std::cout << "Usage: renamer [options] <directory_path>\n\n";
    
    std::cout << "=== Rename Modes (choose exactly one) ===\n\n";
    
    std::cout << "1. Add Prefix/Suffix Mode:\n";
    std::cout << "   --prefix <string>      Add prefix before filename\n";
    std::cout << "   --suffix <string>      Add suffix before extension\n";
    std::cout << "   Example: --prefix \"photo_\" --suffix \"_2024\"\n\n";
    
    std::cout << "2. Replace Mode:\n";
    std::cout << "   --replace <find> <replace>  Find and replace string in filename\n";
    std::cout << "   --regex <pattern> <replace>  Use regular expression for replacement\n";
    std::cout << "   Example: --replace \"old\" \"new\"\n";
    std::cout << "            --regex \"(\\\\d+)\" \"number_$1\"\n\n";
    
    std::cout << "3. Numbering Mode:\n";
    std::cout << "   --numbering            Enable numbering mode\n";
    std::cout << "   --start <number>       Starting number (default: 1)\n";
    std::cout << "   --padding <digits>     Zero padding width (e.g., 3 for 001, 002)\n";
    std::cout << "   --format <format>      Custom format, variables available:\n";
    std::cout << "                          [N] or # - Number\n";
    std::cout << "                          [F] - Original filename\n";
    std::cout << "   Example: --numbering --padding 3\n";
    std::cout << "            --numbering --format \"photo_[N]\"\n";
    std::cout << "            --numbering --format \"[F]_#[N]\"\n\n";
    
    std::cout << "4. Case Conversion Mode:\n";
    std::cout << "   --uppercase            Convert to UPPERCASE\n";
    std::cout << "   --lowercase            Convert to lowercase\n";
    std::cout << "   --capitalize           Capitalize first letter\n";
    std::cout << "   --camel-case           Convert to camelCase (helloWorld)\n";
    std::cout << "   --snake-case           Convert to snake_case (hello_world)\n";
    std::cout << "   --kebab-case           Convert to kebab-case (hello-world)\n\n";
    
    std::cout << "=== General Options ===\n\n";
    std::cout << "   -r, --recursive        Process files in subdirectories recursively\n";
    std::cout << "   --include-ext          Include extension in renaming\n";
    std::cout << "   -n, --dry-run          Preview mode, do not actually rename\n";
    std::cout << "   -v, --verbose          Show detailed output\n";
    std::cout << "   -h, --help             Show this help message\n\n";
    
    std::cout << "=== Examples ===\n\n";
    std::cout << "1. Add prefix to all photos:\n";
    std::cout << "   renamer --prefix \"vacation_\" ./photos\n\n";
    
    std::cout << "2. Replace \"DSC\" with \"photo\":\n";
    std::cout << "   renamer --replace \"DSC\" \"photo\" ./photos\n\n";
    
    std::cout << "3. Sequential numbering (001, 002, 003...):\n";
    std::cout << "   renamer --numbering --padding 3 ./photos\n\n";
    
    std::cout << "4. Convert to lowercase:\n";
    std::cout << "   renamer --lowercase ./files\n\n";
    
    std::cout << "5. Use regex to extract and reorganize:\n";
    std::cout << "   renamer --regex \"IMG_(\\\\d+)\" \"Image_$1_edited\" ./photos\n\n";
    
    std::cout << "6. Recursive processing with preview:\n";
    std::cout << "   renamer -r --prefix \"backup_\" -n ./documents\n\n";
    
    std::cout << std::string(60, '=') << "\n\n";
}

RenameOptions ArgParser::parseArguments(int argc, char* argv[]) {
    RenameOptions options;
    bool modeSet = false;
    std::string path;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printHelp();
            exit(0);
        }
        
        if (arg == "--prefix") {
            if (i + 1 >= argc) {
                throw std::runtime_error("--prefix requires a string argument");
            }
            options.mode = RenameMode::ADD_PREFIX;
            options.prefix = argv[++i];
            modeSet = true;
        }
        else if (arg == "--suffix") {
            if (i + 1 >= argc) {
                throw std::runtime_error("--suffix requires a string argument");
            }
            options.mode = RenameMode::ADD_SUFFIX;
            options.suffix = argv[++i];
            modeSet = true;
        }
        else if (arg == "--replace") {
            if (i + 2 >= argc) {
                throw std::runtime_error("--replace requires find and replace arguments");
            }
            options.mode = RenameMode::REPLACE;
            options.searchStr = argv[++i];
            options.replaceStr = argv[++i];
            modeSet = true;
        }
        else if (arg == "--regex") {
            if (i + 2 >= argc) {
                throw std::runtime_error("--regex requires pattern and replacement arguments");
            }
            options.mode = RenameMode::REGEX_REPLACE;
            options.regexPattern = argv[++i];
            options.regexReplace = argv[++i];
            modeSet = true;
        }
        else if (arg == "--numbering") {
            options.mode = RenameMode::NUMBERING;
            modeSet = true;
        }
        else if (arg == "--start") {
            if (i + 1 >= argc) {
                throw std::runtime_error("--start requires a number argument");
            }
            try {
                options.startNumber = std::stoi(argv[++i]);
            } catch (...) {
                throw std::runtime_error("--start argument must be a valid number");
            }
        }
        else if (arg == "--padding") {
            if (i + 1 >= argc) {
                throw std::runtime_error("--padding requires a number argument");
            }
            try {
                options.padding = std::stoi(argv[++i]);
            } catch (...) {
                throw std::runtime_error("--padding argument must be a valid number");
            }
        }
        else if (arg == "--format") {
            if (i + 1 >= argc) {
                throw std::runtime_error("--format requires a format string");
            }
            options.numberFormat = argv[++i];
        }
        else if (arg == "--uppercase") {
            options.mode = RenameMode::UPPERCASE;
            modeSet = true;
        }
        else if (arg == "--lowercase") {
            options.mode = RenameMode::LOWERCASE;
            modeSet = true;
        }
        else if (arg == "--capitalize") {
            options.mode = RenameMode::CAPITALIZE;
            modeSet = true;
        }
        else if (arg == "--camel-case") {
            options.mode = RenameMode::CAMEL_CASE;
            modeSet = true;
        }
        else if (arg == "--snake-case") {
            options.mode = RenameMode::SNAKE_CASE;
            modeSet = true;
        }
        else if (arg == "--kebab-case") {
            options.mode = RenameMode::KEBAB_CASE;
            modeSet = true;
        }
        else if (arg == "-r" || arg == "--recursive") {
            options.recursive = true;
        }
        else if (arg == "--include-ext") {
            options.includeExtensions = true;
        }
        else if (arg == "-n" || arg == "--dry-run") {
            options.dryRun = true;
        }
        else if (arg == "-v" || arg == "--verbose") {
            options.verbose = true;
        }
        else if (arg[0] == '-') {
            throw std::runtime_error("Unknown option: " + arg);
        }
        else {
            if (!path.empty()) {
                throw std::runtime_error("Only one directory path can be specified");
            }
            path = arg;
        }
    }
    
    if (!modeSet) {
        throw std::runtime_error("A rename mode must be specified. Use -h for help.");
    }
    
    if (path.empty()) {
        throw std::runtime_error("A directory path must be specified. Use -h for help.");
    }
    
    options.targetPath = path;
    return options;
}

void ArgParser::validateOptions(const RenameOptions& options) {
    if (options.mode == RenameMode::REGEX_REPLACE) {
        try {
            std::regex test(options.regexPattern);
        } catch (const std::regex_error& e) {
            throw std::runtime_error("Invalid regex pattern: " + std::string(e.what()));
        }
    }
}
