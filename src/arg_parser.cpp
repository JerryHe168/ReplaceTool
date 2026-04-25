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
    std::cout << "   Note: --prefix and --suffix can be used together\n";
    std::cout << "   Example: --prefix \"photo_\" --suffix \"_2024\"\n\n";
    
    std::cout << "2. Replace Mode:\n";
    std::cout << "   --replace <find> <replace>  Find and replace string in filename\n";
    std::cout << "   --regex <pattern> <replace>  Use regular expression for replacement\n";
    std::cout << "   Example: --replace \"old\" \"new\"\n";
    std::cout << "            --regex \"(\\\\d+)\" \"number_$1\"\n\n";
    
    std::cout << "3. Numbering Mode:\n";
    std::cout << "   --numbering            Enable numbering mode\n";
    std::cout << "   --start <number>       Starting number (default: 1)\n";
    std::cout << "   --step <number>        Number increment step (default: 1)\n";
    std::cout << "   --padding <digits>     Zero padding width (e.g., 3 for 001, 002)\n";
    std::cout << "   --pos-prefix           Place number as prefix before filename\n";
    std::cout << "   --pos-suffix           Place number as suffix after filename (default)\n";
    std::cout << "   --pos-insert <index>   Insert number at specified index position\n";
    std::cout << "                          Example: \"photo\" with --pos-insert 3 -> \"ph001oto\"\n";
    std::cout << "   --format <format>      Custom format, variables available:\n";
    std::cout << "                          [N] or [n] - Number\n";
    std::cout << "                          [F] or [f] - Original filename\n";
    std::cout << "   Important: [F]/[f] is replaced first, then [N]/[n] placeholders\n";
    std::cout << "   If no number placeholder ([N]/[n]) is found, number is appended with underscore\n";
    std::cout << "   Example: --numbering --padding 3\n";
    std::cout << "            --numbering --format \"photo_[N]\"\n";
    std::cout << "            --numbering --format \"[F]_backup_[N]\"\n";
    std::cout << "            --numbering --pos-prefix --padding 2\n\n";
    
    std::cout << "4. Case Conversion Mode:\n";
    std::cout << "   --uppercase            Convert to UPPERCASE\n";
    std::cout << "   --lowercase            Convert to lowercase\n";
    std::cout << "   --capitalize           Capitalize first letter\n";
    std::cout << "   --camel-case           Convert to camelCase (helloWorld)\n";
    std::cout << "   --snake-case           Convert to snake_case (hello_world)\n";
    std::cout << "   --kebab-case           Convert to kebab-case (hello-world)\n\n";
    
    std::cout << "=== Conflict Resolution Options ===\n\n";
    std::cout << "   --skip-conflict        Skip files with conflicting names (default)\n";
    std::cout << "   --overwrite            Overwrite existing files (use with caution)\n";
    std::cout << "   --auto-rename          Auto-rename conflicting files (e.g., file.txt -> file (1).txt)\n\n";
    
    std::cout << "=== File Filter Options ===\n\n";
    std::cout << "   --ext <ext1,ext2...>   Include only files with specified extensions\n";
    std::cout << "                          Example: --ext \".jpg,.png,.gif\"\n\n";
    std::cout << "   --include <pattern>    Include files containing pattern in name\n";
    std::cout << "                          Can be used multiple times\n";
    std::cout << "                          Example: --include \"IMG\" --include \"photo\"\n\n";
    std::cout << "   --include-regex <regex>  Include files matching regex pattern\n";
    std::cout << "                          Example: --include-regex \"IMG_\\\\d+\"\n\n";
    std::cout << "   --exclude-name <name>  Exclude specific filename\n";
    std::cout << "                          Can be used multiple times\n";
    std::cout << "                          Example: --exclude-name \"thumbs.db\"\n\n";
    std::cout << "   --exclude-ext <ext>    Exclude files with specified extension\n";
    std::cout << "                          Can be used multiple times\n";
    std::cout << "                          Example: --exclude-ext \".tmp\"\n\n";
    std::cout << "   --exclude-regex <regex>  Exclude files matching regex pattern\n";
    std::cout << "                          Example: --exclude-regex \"^\\\\.\" (exclude hidden files)\n\n";
    std::cout << "   --exclude-dir <name>   Exclude specific subdirectory\n";
    std::cout << "                          Can be used multiple times\n";
    std::cout << "                          Example: --exclude-dir \"backup\" --exclude-dir \"temp\"\n\n";
    
    std::cout << "=== General Options ===\n\n";
    std::cout << "   -r, --recursive        Process files in subdirectories recursively\n";
    std::cout << "   --include-ext          Include extension in renaming\n";
    std::cout << "   -n, --dry-run          Preview mode, do not actually rename\n";
    std::cout << "   -v, --verbose          Show detailed output\n";
    std::cout << "   -h, --help             Show this help message\n\n";
    
    std::cout << "=== Examples ===\n\n";
    std::cout << "1. Add prefix to all photos:\n";
    std::cout << "   renamer --prefix \"vacation_\" ./photos\n\n";
    
    std::cout << "2. Add both prefix and suffix:\n";
    std::cout << "   renamer --prefix \"photo_\" --suffix \"_edited\" ./photos\n\n";
    
    std::cout << "3. Numbering with custom step and position:\n";
    std::cout << "   renamer --numbering --start 100 --step 5 --padding 3 --pos-prefix ./photos\n\n";
    
    std::cout << "4. Process only image files:\n";
    std::cout << "   renamer --lowercase --ext \".jpg,.png,.gif\" ./files\n\n";
    
    std::cout << "5. Include files matching pattern and exclude temp files:\n";
    std::cout << "   renamer --prefix \"doc_\" --include \"report\" --exclude-ext \".tmp\" ./files\n\n";
    
    std::cout << "6. Convert to lowercase with auto-rename on conflict:\n";
    std::cout << "   renamer --lowercase --auto-rename ./files\n\n";
    
    std::cout << "7. Recursive processing with preview:\n";
    std::cout << "   renamer -r --prefix \"backup_\" -n ./documents\n\n";
    
    std::cout << std::string(60, '=') << "\n\n";
}

std::vector<std::string> splitByComma(const std::string& str) {
    std::vector<std::string> result;
    std::string current;
    
    for (char c : str) {
        if (c == ',') {
            if (!current.empty()) {
                result.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    
    if (!current.empty()) {
        result.push_back(current);
    }
    
    return result;
}

RenameOptions ArgParser::parseArguments(int argc, char* argv[]) {
    RenameOptions options;
    bool modeSet = false;
    bool prefixSet = false;
    bool suffixSet = false;
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
            options.prefix = argv[++i];
            prefixSet = true;
            if (suffixSet) {
                options.mode = RenameMode::ADD_PREFIX_SUFFIX;
            } else {
                options.mode = RenameMode::ADD_PREFIX;
            }
            modeSet = true;
        }
        else if (arg == "--suffix") {
            if (i + 1 >= argc) {
                throw std::runtime_error("--suffix requires a string argument");
            }
            options.suffix = argv[++i];
            suffixSet = true;
            if (prefixSet) {
                options.mode = RenameMode::ADD_PREFIX_SUFFIX;
            } else {
                options.mode = RenameMode::ADD_SUFFIX;
            }
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
        else if (arg == "--step") {
            if (i + 1 >= argc) {
                throw std::runtime_error("--step requires a number argument");
            }
            try {
                options.numberStep = std::stoi(argv[++i]);
            } catch (...) {
                throw std::runtime_error("--step argument must be a valid number");
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
        else if (arg == "--pos-prefix") {
            options.numberPosition = NumberPosition::PREFIX;
        }
        else if (arg == "--pos-suffix") {
            options.numberPosition = NumberPosition::SUFFIX;
        }
        else if (arg == "--pos-insert") {
            if (i + 1 >= argc) {
                throw std::runtime_error("--pos-insert requires an index argument");
            }
            try {
                options.insertPosition = std::stoi(argv[++i]);
                options.numberPosition = NumberPosition::INSERT;
            } catch (...) {
                throw std::runtime_error("--pos-insert argument must be a valid number");
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
        else if (arg == "--skip-conflict") {
            options.conflictResolution = ConflictResolution::SKIP;
        }
        else if (arg == "--overwrite") {
            options.conflictResolution = ConflictResolution::OVERWRITE;
        }
        else if (arg == "--auto-rename") {
            options.conflictResolution = ConflictResolution::AUTO_RENAME;
        }
        else if (arg == "--ext") {
            if (i + 1 >= argc) {
                throw std::runtime_error("--ext requires extensions argument");
            }
            options.filterExtensions = splitByComma(argv[++i]);
        }
        else if (arg == "--include") {
            if (i + 1 >= argc) {
                throw std::runtime_error("--include requires a pattern argument");
            }
            options.includePatterns.push_back(argv[++i]);
        }
        else if (arg == "--include-regex") {
            if (i + 1 >= argc) {
                throw std::runtime_error("--include-regex requires a regex argument");
            }
            options.includeRegex = argv[++i];
        }
        else if (arg == "--exclude-name") {
            if (i + 1 >= argc) {
                throw std::runtime_error("--exclude-name requires a filename argument");
            }
            options.excludeNames.push_back(argv[++i]);
        }
        else if (arg == "--exclude-ext") {
            if (i + 1 >= argc) {
                throw std::runtime_error("--exclude-ext requires an extension argument");
            }
            options.excludeExtensions.push_back(argv[++i]);
        }
        else if (arg == "--exclude-regex") {
            if (i + 1 >= argc) {
                throw std::runtime_error("--exclude-regex requires a regex argument");
            }
            options.excludeRegex = argv[++i];
        }
        else if (arg == "--exclude-dir") {
            if (i + 1 >= argc) {
                throw std::runtime_error("--exclude-dir requires a directory name argument");
            }
            options.excludeDirectories.push_back(argv[++i]);
        }
        else if (arg == "-r" || arg == "--recursive") {
            options.recursive = true;
        }
        else if (arg == "--include-ext") {
            options.renameWithExtension = true;
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
    if (options.mode == RenameMode::REGEX_REPLACE && !options.regexPattern.empty()) {
        try {
            std::regex test(options.regexPattern);
        } catch (const std::regex_error& e) {
            throw std::runtime_error("Invalid regex pattern: " + std::string(e.what()));
        }
    }
    
    if (!options.includeRegex.empty()) {
        try {
            std::regex test(options.includeRegex);
        } catch (const std::regex_error& e) {
            throw std::runtime_error("Invalid include regex: " + std::string(e.what()));
        }
    }
    
    if (!options.excludeRegex.empty()) {
        try {
            std::regex test(options.excludeRegex);
        } catch (const std::regex_error& e) {
            throw std::runtime_error("Invalid exclude regex: " + std::string(e.what()));
        }
    }
}
