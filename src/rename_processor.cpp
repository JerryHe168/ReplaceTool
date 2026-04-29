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
            result = applyNumbering(result, options.numberFormat, options.startNumber, 
                                     options.numberStep, index, options.padding,
                                     options.numberPosition, options.insertPosition);
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
        if (options.renameWithExtension) {
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

bool RenameProcessor::isNameUnique(size_t currentIndex, const std::string& testName, 
                                     const std::string& testExtension,
                                     const std::vector<FileInfo>& files) {
    std::string testFullName = testName + testExtension;
    
    for (size_t i = 0; i < files.size(); ++i) {
        if (i != currentIndex) {
            std::string fullName = files[i].newName + files[i].extension;
            if (fullName == testFullName) {
                return false;
            }
        }
    }
    
    std::string fullPath = FileUtils::combinePath(files[currentIndex].directory, testName, testExtension);
    if (FileUtils::fileExists(fullPath) && fullPath != files[currentIndex].originalPath) {
        return false;
    }
    
    return true;
}

bool RenameProcessor::checkAndResolveAllConflicts(std::vector<FileInfo>& files, ConflictResolution resolution) {
    bool hasConflict = false;
    std::map<std::string, std::vector<size_t>> nameToIndices;
    
    for (size_t i = 0; i < files.size(); ++i) {
        std::string fullNewName = files[i].newName + files[i].extension;
        nameToIndices[fullNewName].push_back(i);
    }
    
    for (const auto& pair : nameToIndices) {
        if (pair.second.size() > 1) {
            hasConflict = true;
            
            if (resolution == ConflictResolution::SKIP) {
                for (size_t j = 1; j < pair.second.size(); ++j) {
                    size_t idx = pair.second[j];
                    files[idx].newName = files[idx].name;
                }
            }
            else if (resolution == ConflictResolution::AUTO_RENAME) {
                std::string baseName = files[pair.second[0]].newName;
                
                for (size_t j = 1; j < pair.second.size(); ++j) {
                    size_t idx = pair.second[j];
                    int counter = 1;
                    std::string newName;
                    bool unique;
                    
                    do {
                        newName = getNextAutoRename(baseName, counter);
                        unique = isNameUnique(idx, newName, files[idx].extension, files);
                        counter++;
                    } while (!unique);
                    
                    files[idx].newName = newName;
                }
            }
        }
    }
    
    for (size_t i = 0; i < files.size(); ++i) {
        std::string fullPath = FileUtils::combinePath(files[i].directory, files[i].newName, files[i].extension);
        
        if (FileUtils::fileExists(fullPath) && fullPath != files[i].originalPath) {
            hasConflict = true;
            
            if (resolution == ConflictResolution::SKIP) {
                files[i].newName = files[i].name;
            }
            else if (resolution == ConflictResolution::AUTO_RENAME) {
                int counter = 1;
                std::string newName;
                bool unique;
                
                do {
                    newName = getNextAutoRename(files[i].newName, counter);
                    unique = isNameUnique(i, newName, files[i].extension, files);
                    counter++;
                } while (!unique);
                
                files[i].newName = newName;
            }
        }
    }
    
    return hasConflict;
}

void RenameProcessor::resolveConflicts(std::vector<FileInfo>& files, ConflictResolution resolution) {
    if (resolution == ConflictResolution::OVERWRITE) {
        return;
    }
    
    const int maxIterations = 100;
    int iteration = 0;
    bool hasConflict = true;
    
    while (hasConflict && iteration < maxIterations) {
        iteration++;
        hasConflict = checkAndResolveAllConflicts(files, resolution);
    }
    
    if (iteration >= maxIterations && hasConflict) {
        for (auto& file : files) {
            std::string fullPath = FileUtils::combinePath(file.directory, file.newName, file.extension);
            if (FileUtils::fileExists(fullPath) && fullPath != file.originalPath) {
                file.newName = file.name;
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
        
        PathError nameError = FileUtils::validateFileName(file.newName);
        if (nameError != PathError::OK) {
            std::string errorMsg;
            switch (nameError) {
                case PathError::INVALID_CHARACTERS:
                    errorMsg = "contains invalid characters";
                    break;
                case PathError::RESERVED_NAME:
                    errorMsg = "is a reserved name";
                    break;
                case PathError::EMPTY_PATH:
                    errorMsg = "is empty";
                    break;
                default:
                    errorMsg = "has an error";
            }
            
            if (options.verbose) {
                std::cout << "Skipping: " << oldName << " -> " << newName 
                          << " (new name " << errorMsg << ")\n";
            }
            failCount++;
            success = false;
            continue;
        }
        
        std::string newFullPath = file.getNewFullPath();
        PathError pathError = FileUtils::validateFilePath(newFullPath);
        if (pathError != PathError::OK) {
            if (options.verbose) {
                std::cout << "Skipping: " << oldName << " -> " << newName 
                          << " (new path is too long)\n";
            }
            failCount++;
            success = false;
            continue;
        }
        
        if (options.verbose) {
            std::cout << std::left << std::setw(static_cast<int>(maxNameLength + 5)) 
                      << oldName << " -> " << newName;
        }
        
        if (!options.dryRun) {
            bool result = FileUtils::renameFile(file.originalPath, newFullPath);
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
                                               int startNumber, int numberStep, int index, int padding,
                                               NumberPosition position, int insertPos) {
    int number = startNumber + (index * numberStep);
    std::string numStr = StringUtils::formatNumber(number, padding);
    
    if (!format.empty()) {
        std::string result = format;
        
        size_t fPos = result.find("[F]");
        while (fPos != std::string::npos) {
            result.replace(fPos, 3, name);
            fPos = result.find("[F]", fPos + name.length());
        }
        
        fPos = result.find("[f]");
        while (fPos != std::string::npos) {
            result.replace(fPos, 3, name);
            fPos = result.find("[f]", fPos + name.length());
        }
        
        bool hasNumberPlaceholder = false;
        size_t nPos = result.find("[N]");
        if (nPos != std::string::npos) {
            hasNumberPlaceholder = true;
            while (nPos != std::string::npos) {
                result.replace(nPos, 3, numStr);
                nPos = result.find("[N]", nPos + numStr.length());
            }
        }
        
        nPos = result.find("[n]");
        if (nPos != std::string::npos) {
            hasNumberPlaceholder = true;
            while (nPos != std::string::npos) {
                result.replace(nPos, 3, numStr);
                nPos = result.find("[n]", nPos + numStr.length());
            }
        }
        
        if (!hasNumberPlaceholder) {
            result = result + "_" + numStr;
        }
        
        return result;
    }
    
    switch (position) {
        case NumberPosition::PREFIX:
            return numStr + "_" + name;
            
        case NumberPosition::SUFFIX:
            return name + "_" + numStr;
            
        case NumberPosition::INSERT: {
            size_t charCount = FileUtils::utf8CharacterCount(name);
            int pos = insertPos;
            
            if (pos < 0) {
                pos = 0;
            } else if (static_cast<size_t>(pos) > charCount) {
                pos = static_cast<int>(charCount);
            }
            
            size_t bytePos = FileUtils::utf8BytePosition(name, static_cast<size_t>(pos));
            std::string result = name;
            result.insert(bytePos, numStr);
            return result;
        }
            
        default:
            return name + "_" + numStr;
    }
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
