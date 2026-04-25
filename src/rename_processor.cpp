#include "../include/renamer.h"
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <set>
#include <map>

std::string RenameProcessor::processFileName(const std::string& name, const RenameOptions& options, int index) {
    std::string result = name;
    
    switch (options.mode) {
        case RenameMode::ADD_PREFIX:
            result = addPrefix(result, options.prefix);
            break;
            
        case RenameMode::ADD_SUFFIX:
            result = addSuffix(result, options.suffix);
            break;
            
        case RenameMode::ADD_PREFIX_SUFFIX:
            result = addPrefixAndSuffix(result, options.prefix, options.suffix);
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
    
    resolveConflicts(files, options.conflictResolution);
}

std::string RenameProcessor::getNextAutoRename(const std::string& name, int counter) {
    return name + " (" + std::to_string(counter) + ")";
}

void RenameProcessor::resolveConflicts(std::vector<FileInfo>& files, ConflictResolution resolution) {
    std::map<std::string, std::vector<size_t>> nameToIndices;
    
    for (size_t i = 0; i < files.size(); ++i) {
        std::string fullNewName = files[i].newName + files[i].extension;
        nameToIndices[fullNewName].push_back(i);
    }
    
    for (const auto& pair : nameToIndices) {
        if (pair.second.size() > 1) {
            std::string baseName = files[pair.second[0]].newName;
            std::string extension = files[pair.second[0]].extension;
            
            for (size_t j = 1; j < pair.second.size(); ++j) {
                size_t idx = pair.second[j];
                
                switch (resolution) {
                    case ConflictResolution::SKIP:
                        files[idx].newName = files[idx].name;
                        break;
                        
                    case ConflictResolution::OVERWRITE:
                        break;
                        
                    case ConflictResolution::AUTO_RENAME:
                        int counter = 1;
                        std::string newName;
                        bool unique;
                        do {
                            newName = getNextAutoRename(baseName, counter);
                            unique = true;
                            for (const auto& f : files) {
                                if (f.newName == newName) {
                                    unique = false;
                                    break;
                                }
                            }
                            counter++;
                        } while (!unique);
                        
                        files[idx].newName = newName;
                        break;
                }
            }
        }
    }
    
    std::set<std::string> existingFiles;
    for (const auto& file : files) {
        std::string fullPath = FileUtils::combinePath(file.directory, file.newName, file.extension);
        if (FileUtils::fileExists(fullPath) && fullPath != file.originalPath) {
            existingFiles.insert(file.newName + file.extension);
        }
    }
    
    if (!existingFiles.empty()) {
        for (auto& file : files) {
            std::string fullNewName = file.newName + file.extension;
            if (existingFiles.find(fullNewName) != existingFiles.end()) {
                switch (resolution) {
                    case ConflictResolution::SKIP:
                        file.newName = file.name;
                        break;
                        
                    case ConflictResolution::OVERWRITE:
                        break;
                        
                    case ConflictResolution::AUTO_RENAME:
                        int counter = 1;
                        std::string newName;
                        bool unique;
                        do {
                            newName = getNextAutoRename(file.newName, counter);
                            std::string testPath = FileUtils::combinePath(file.directory, newName, file.extension);
                            unique = !FileUtils::fileExists(testPath);
                            counter++;
                        } while (!unique);
                        
                        file.newName = newName;
                        break;
                }
            }
        }
    }
}

bool RenameProcessor::executeRename(std::vector<FileInfo>& files, const RenameOptions& options) {
    bool success = true;
    int successCount = 0;
    int failCount = 0;
    int skipCount = 0;
    
    std::cout << "\n" << std::string(60, '=') << "\n";
    if (options.dryRun) {
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
            if (options.verbose) {
                std::cout << "Skipping: " << oldName << " (no change needed or conflict resolution)\n";
            }
            skipCount++;
            continue;
        }
        
        if (options.verbose) {
            std::cout << std::left << std::setw(static_cast<int>(maxNameLength + 5)) 
                      << oldName << " -> " << newName;
        }
        
        if (!options.dryRun) {
            bool result = FileUtils::renameFile(file.originalPath, file.getNewFullPath());
            if (result) {
                if (options.verbose) {
                    std::cout << " [OK]\n";
                }
                successCount++;
            } else {
                if (options.verbose) {
                    std::cout << " [FAILED - target exists or permission denied]\n";
                }
                failCount++;
                success = false;
            }
        } else {
            if (options.verbose) {
                std::cout << " [Preview]\n";
            }
            successCount++;
        }
    }
    
    std::cout << "\n" << std::string(60, '-') << "\n";
    std::cout << "Operation Complete:\n";
    std::cout << "  Total files: " << files.size() << "\n";
    std::cout << "  Success: " << successCount << "\n";
    if (skipCount > 0) {
        std::cout << "  Skipped: " << skipCount << "\n";
    }
    if (failCount > 0) {
        std::cout << "  Failed: " << failCount << "\n";
    }
    if (options.dryRun) {
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

std::string RenameProcessor::addPrefixAndSuffix(const std::string& name, const std::string& prefix, const std::string& suffix) {
    return prefix + name + suffix;
}

std::string RenameProcessor::applyNumbering(const std::string& name, const std::string& format, 
                                               int startNumber, int index, int padding) {
    int number = startNumber + index;
    std::string numStr = StringUtils::formatNumber(number, padding);
    
    if (format.empty()) {
        return name + "_" + numStr;
    }
    
    std::string result = format;
    
    result = StringUtils::replaceAll(result, "[F]", name);
    result = StringUtils::replaceAll(result, "[f]", name);
    
    bool hasNumberPlaceholder = false;
    if (result.find("[N]") != std::string::npos || 
        result.find("[n]") != std::string::npos) {
        hasNumberPlaceholder = true;
    }
    
    result = StringUtils::replaceAll(result, "[N]", numStr);
    result = StringUtils::replaceAll(result, "[n]", numStr);
    
    if (!hasNumberPlaceholder) {
        result = result + "_" + numStr;
    }
    
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
