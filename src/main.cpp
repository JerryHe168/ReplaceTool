#include "../include/renamer.h"
#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc, char* argv[]) {
    try {
        if (argc < 2) {
            ArgParser::printHelp();
            return 1;
        }
        
        RenameOptions options = ArgParser::parseArguments(argc, argv);
        ArgParser::validateOptions(options);
        
        if (!FileUtils::fileExists(options.targetPath)) {
            throw std::runtime_error("Path does not exist: " + options.targetPath);
        }
        
        if (!FileUtils::isDirectory(options.targetPath)) {
            throw std::runtime_error("Specified path is not a directory: " + options.targetPath);
        }
        
        std::cout << "\n" << std::string(60, '=') << "\n";
        std::cout << "Batch File Renamer Tool\n";
        std::cout << std::string(60, '=') << "\n\n";
        
        std::cout << "Target directory: " << options.targetPath << "\n";
        if (options.recursive) {
            std::cout << "Mode: Recursive (including subdirectories)\n";
        }
        
        std::cout << "Rename mode: ";
        switch (options.mode) {
            case RenameMode::ADD_PREFIX:
                std::cout << "Add prefix (\"" << options.prefix << "\")\n";
                break;
            case RenameMode::ADD_SUFFIX:
                std::cout << "Add suffix (\"" << options.suffix << "\")\n";
                break;
            case RenameMode::REPLACE:
                std::cout << "String replace (\"" << options.searchStr << "\" -> \"" << options.replaceStr << "\")\n";
                break;
            case RenameMode::REGEX_REPLACE:
                std::cout << "Regex replace (pattern: \"" << options.regexPattern << "\")\n";
                break;
            case RenameMode::NUMBERING:
                std::cout << "Numbering mode";
                if (!options.numberFormat.empty()) {
                    std::cout << " (format: \"" << options.numberFormat << "\")";
                }
                std::cout << "\n";
                if (options.padding > 0) {
                    std::cout << "  Padding: " << options.padding << " digits\n";
                }
                std::cout << "  Start number: " << options.startNumber << "\n";
                break;
            case RenameMode::UPPERCASE:
                std::cout << "Convert to uppercase\n";
                break;
            case RenameMode::LOWERCASE:
                std::cout << "Convert to lowercase\n";
                break;
            case RenameMode::CAPITALIZE:
                std::cout << "Capitalize\n";
                break;
            case RenameMode::CAMEL_CASE:
                std::cout << "camelCase\n";
                break;
            case RenameMode::SNAKE_CASE:
                std::cout << "snake_case\n";
                break;
            case RenameMode::KEBAB_CASE:
                std::cout << "kebab-case\n";
                break;
        }
        
        if (options.renameWithExtension) {
            std::cout << "Rename with extension: Yes\n";
        }
        
        if (!options.filterExtensions.empty()) {
            std::cout << "Filter by extensions: ";
            for (size_t i = 0; i < options.filterExtensions.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << options.filterExtensions[i];
            }
            std::cout << "\n";
        }
        
        if (!options.includePatterns.empty()) {
            std::cout << "Include patterns: ";
            for (size_t i = 0; i < options.includePatterns.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << options.includePatterns[i];
            }
            std::cout << "\n";
        }
        
        if (!options.includeRegex.empty()) {
            std::cout << "Include regex: " << options.includeRegex << "\n";
        }
        
        if (!options.excludeNames.empty()) {
            std::cout << "Exclude names: ";
            for (size_t i = 0; i < options.excludeNames.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << options.excludeNames[i];
            }
            std::cout << "\n";
        }
        
        if (!options.excludeExtensions.empty()) {
            std::cout << "Exclude extensions: ";
            for (size_t i = 0; i < options.excludeExtensions.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << options.excludeExtensions[i];
            }
            std::cout << "\n";
        }
        
        if (!options.excludeRegex.empty()) {
            std::cout << "Exclude regex: " << options.excludeRegex << "\n";
        }
        
        if (!options.excludeDirectories.empty()) {
            std::cout << "Exclude directories: ";
            for (size_t i = 0; i < options.excludeDirectories.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << options.excludeDirectories[i];
            }
            std::cout << "\n";
        }
        
        if (options.dryRun) {
            std::cout << "\nWARNING: Preview mode - files will NOT be actually renamed\n";
        }
        
        std::cout << "\nScanning for files...\n";
        std::vector<FileInfo> files = FileUtils::getFilesInDirectory(options.targetPath, options.recursive, options);
        
        if (files.empty()) {
            std::cout << "\nNo files found in directory.\n";
            return 0;
        }
        
        std::cout << "Found " << files.size() << " file(s).\n";
        
        if (options.verbose) {
            std::cout << "\nFile list:\n";
            for (const auto& file : files) {
                std::cout << "  " << file.name << file.extension << "\n";
            }
        }
        
        RenameProcessor::processFiles(files, options);
        
        bool success = RenameProcessor::executeRename(files, options);
        
        return success ? 0 : 1;
        
    } catch (const std::exception& e) {
        std::cerr << "\nError: " << e.what() << "\n\n";
        std::cerr << "Use -h or --help for usage information.\n\n";
        return 1;
    }
}
