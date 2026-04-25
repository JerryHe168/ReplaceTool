#include "../include/renamer.h"
#include <iostream>
#include <iomanip>
#include <stdexcept>

std::string RenameProcessor::processFileName(const std::string& name, const RenameOptions& options, int index) {
    std::string result = name;
    
    switch (options.mode) {
        case RenameMode::ADD_PREFIX:
            result = addPrefix(result, options.prefix);
            break;
            
        case RenameMode::ADD_SUFFIX:
            result = addSuffix(result, options.suffix);
            break;
            
        case RenameMode::REPLACE:
            result = StringUtils::replaceAll(result, options.searchStr, options.replaceStr);
            break;
            
        case RenameMode::REGEX_REPLACE:
            result = StringUtils::regexReplace(result, options.regexPattern, options.regexReplace);
            break;
            
        case RenameMode::NUMBERING:
            result = applyNumbering(result, options.numberFormat, options.startNumber, index, options.padding);
            break;
            
        case RenameMode::UPPERCASE:
        case RenameMode::LOWERCASE:
        case RenameMode::CAPITALIZE:
        case RenameMode::CAMEL_CASE:
        case RenameMode::SNAKE_CASE:
        case RenameMode::KEBAB_CASE:
            result = applyCaseTransformation(result, options.mode);
            break;
            
        default:
            break;
    }
    
    return result;
}

void RenameProcessor::processFiles(std::vector<FileInfo>& files, const RenameOptions& options) {
    for (size_t i = 0; i < files.size(); ++i) {
        if (options.includeExtensions) {
            std::string fullName = files[i].name + files[i].extension;
            std::string newFullName = processFileName(fullName, options, static_cast<int>(i));
            
            size_t dotPos = newFullName.find_last_of('.');
            if (dotPos != std::string::npos && dotPos > 0) {
                files[i].newName = newFullName.substr(0, dotPos);
                files[i].extension = newFullName.substr(dotPos);
            } else {
                files[i].newName = newFullName;
            }
        } else {
            files[i].newName = processFileName(files[i].name, options, static_cast<int>(i));
        }
    }
}

bool RenameProcessor::executeRename(std::vector<FileInfo>& files, bool dryRun, bool verbose) {
    bool success = true;
    int successCount = 0;
    int failCount = 0;
    
    std::cout << "\n" << std::string(60, '=') << "\n";
    if (dryRun) {
        std::cout << "Preview Mode (files will NOT be renamed)\n";
    } else {
        std::cout << "Executing Rename Operation\n";
    }
    std::cout << std::string(60, '=') << "\n\n";
    
    size_t maxNameLength = 0;
    for (const auto& file : files) {
        maxNameLength = std::max(maxNameLength, (file.name + file.extension).length());
    }
    
    for (const auto& file : files) {
        std::string oldName = file.name + file.extension;
        std::string newName = file.newName + file.extension;
        
        if (oldName == newName) {
            if (verbose) {
                std::cout << "Skipping: " << oldName << " (no change needed)\n";
            }
            continue;
        }
        
        if (verbose) {
            std::cout << std::left << std::setw(static_cast<int>(maxNameLength + 5)) 
                      << oldName << " -> " << newName;
        }
        
        if (!dryRun) {
            bool result = FileUtils::renameFile(file.originalPath, file.getNewFullPath());
            if (result) {
                if (verbose) {
                    std::cout << " [OK]\n";
                }
                successCount++;
            } else {
                if (verbose) {
                    std::cout << " [FAILED - target exists or permission denied]\n";
                }
                failCount++;
                success = false;
            }
        } else {
            if (verbose) {
                std::cout << " [Preview]\n";
            }
            successCount++;
        }
    }
    
    std::cout << "\n" << std::string(60, '-') << "\n";
    std::cout << "Operation Complete:\n";
    std::cout << "  Total files: " << files.size() << "\n";
    std::cout << "  Success: " << successCount << "\n";
    if (failCount > 0) {
        std::cout << "  Failed: " << failCount << "\n";
    }
    if (dryRun) {
        std::cout << "  * This was a preview, no files were actually modified\n";
    }
    
    return success;
}

std::string RenameProcessor::addPrefix(const std::string& name, const std::string& prefix) {
    return prefix + name;
}

std::string RenameProcessor::addSuffix(const std::string& name, const std::string& suffix) {
    return name + suffix;
}

std::string RenameProcessor::applyNumbering(const std::string& name, const std::string& format, 
                                               int startNumber, int index, int padding) {
    int number = startNumber + index;
    std::string numStr = StringUtils::formatNumber(number, padding);
    
    if (format.empty()) {
        return name + "_" + numStr;
    }
    
    std::string result = format;
    result = StringUtils::replaceAll(result, "[N]", numStr);
    result = StringUtils::replaceAll(result, "[n]", numStr);
    
    if (result.find("[N]") == std::string::npos && 
        result.find("[n]") == std::string::npos &&
        result.find("#") == std::string::npos) {
        result = result + numStr;
    }
    
    while (result.find("#") != std::string::npos) {
        result = StringUtils::replaceAll(result, "#", numStr);
    }
    
    result = StringUtils::replaceAll(result, "[F]", name);
    result = StringUtils::replaceAll(result, "[f]", name);
    
    return result;
}

std::string RenameProcessor::applyCaseTransformation(const std::string& name, RenameMode mode) {
    switch (mode) {
        case RenameMode::UPPERCASE:
            return StringUtils::toUpper(name);
            
        case RenameMode::LOWERCASE:
            return StringUtils::toLower(name);
            
        case RenameMode::CAPITALIZE:
            return StringUtils::capitalize(name);
            
        case RenameMode::CAMEL_CASE:
            return StringUtils::toCamelCase(name);
            
        case RenameMode::SNAKE_CASE:
            return StringUtils::toSnakeCase(name);
            
        case RenameMode::KEBAB_CASE:
            return StringUtils::toKebabCase(name);
            
        default:
            return name;
    }
}
