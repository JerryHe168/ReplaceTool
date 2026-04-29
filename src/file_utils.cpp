#include "../include/renamer.h"
#include <filesystem>
#include <stdexcept>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;

std::string FileInfo::getFullPath() const {
    return originalPath;
}

std::string FileInfo::getNewFullPath() const {
    return FileUtils::combinePath(directory, newName, extension);
}

bool shouldExcludeDirectory(const std::string& dirName, const std::vector<std::string>& excludeDirs) {
    for (const auto& excludeDir : excludeDirs) {
        if (dirName == excludeDir) {
            return true;
        }
    }
    return false;
}

bool FileUtils::matchesExtensions(const std::string& extension, const std::vector<std::string>& extList) {
    if (extList.empty()) {
        return true;
    }
    
    std::string extLower = StringUtils::toLower(extension);
    
    for (const auto& ext : extList) {
        std::string patternLower = StringUtils::toLower(ext);
        
        if (extLower == patternLower) {
            return true;
        }
        
        if (patternLower[0] != '.') {
            std::string patternWithDot = "." + patternLower;
            if (extLower == patternWithDot) {
                return true;
            }
        }
        
        if (extLower[0] == '.') {
            std::string extWithoutDot = extLower.substr(1);
            if (extWithoutDot == patternLower) {
                return true;
            }
        }
    }
    
    return false;
}

bool FileUtils::matchesPattern(const std::string& name, const std::vector<std::string>& patterns) {
    if (patterns.empty()) {
        return true;
    }
    
    for (const auto& pattern : patterns) {
        if (name.find(pattern) != std::string::npos) {
            return true;
        }
    }
    
    return false;
}

bool FileUtils::matchesRegex(const std::string& name, const std::string& regexPattern) {
    if (regexPattern.empty()) {
        return true;
    }
    
    try {
        std::regex regex(regexPattern);
        return std::regex_search(name, regex);
    } catch (const std::regex_error&) {
        return true;
    }
}

bool FileUtils::shouldIncludeFile(const std::string& name, const std::string& extension, 
                                    const std::string& directory, const RenameOptions& options) {
    std::string fullName = name + extension;
    
    for (const auto& excludeName : options.excludeNames) {
        if (fullName == excludeName || name == excludeName) {
            return false;
        }
    }
    
    for (const auto& excludeExt : options.excludeExtensions) {
        std::string extLower = StringUtils::toLower(extension);
        std::string patternLower = StringUtils::toLower(excludeExt);
        
        if (extLower == patternLower) {
            return false;
        }
        
        if (patternLower[0] != '.') {
            std::string patternWithDot = "." + patternLower;
            if (extLower == patternWithDot) {
                return false;
            }
        }
        
        if (extLower[0] == '.') {
            std::string extWithoutDot = extLower.substr(1);
            if (extWithoutDot == patternLower) {
                return false;
            }
        }
    }
    
    if (!options.excludeRegex.empty()) {
        try {
            std::regex regex(options.excludeRegex);
            if (std::regex_search(fullName, regex)) {
                return false;
            }
        } catch (const std::regex_error&) {
            return false;
        }
    }
    
    bool include = true;
    
    if (!options.filterExtensions.empty()) {
        include = include && matchesExtensions(extension, options.filterExtensions);
    }
    
    if (!options.includePatterns.empty()) {
        include = include && matchesPattern(name, options.includePatterns);
    }
    
    if (!options.includeRegex.empty()) {
        try {
            std::regex regex(options.includeRegex);
            include = include && std::regex_search(fullName, regex);
        } catch (const std::regex_error&) {
            return false;
        }
    }
    
    return include;
}

void FileUtils::iterateDirectoryRecursive(const fs::path& currentPath,
                                            std::vector<FileInfo>& files,
                                            FileFilterFunc fileFilter,
                                            const std::vector<std::string>& excludeDirs) {
    try {
        for (const auto& entry : fs::directory_iterator(currentPath)) {
            if (fs::is_directory(entry.status())) {
                if (!fs::is_symlink(entry.path())) {
                    std::string dirName = entry.path().filename().string();
                    bool isExcluded = false;
                    for (const auto& excluded : excludeDirs) {
                        if (dirName == excluded) {
                            isExcluded = true;
                            break;
                        }
                    }
                    if (!isExcluded) {
                        iterateDirectoryRecursive(entry.path(), files, fileFilter, excludeDirs);
                    }
                }
            } else if (fs::is_regular_file(entry.status())) {
                FileInfo info;
                info.originalPath = entry.path().string();
                info.directory = extractDirectory(info.originalPath);
                info.name = extractFileName(info.originalPath);
                info.extension = extractExtension(info.originalPath);
                info.newName = info.name;
                
                if (!fileFilter || fileFilter(info.name, info.extension, info.directory)) {
                    files.push_back(info);
                }
            }
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Error reading directory: " + std::string(e.what()));
    }
}

std::vector<FileInfo> FileUtils::iterateDirectory(const std::string& path, bool recursive,
                                                    FileFilterFunc fileFilter,
                                                    const std::vector<std::string>& excludeDirs) {
    std::vector<FileInfo> files;
    
    if (!fs::exists(path)) {
        throw std::runtime_error("Path does not exist: " + path);
    }
    
    if (!fs::is_directory(path)) {
        throw std::runtime_error("Path is not a directory: " + path);
    }
    
    try {
        if (recursive) {
            iterateDirectoryRecursive(path, files, fileFilter, excludeDirs);
        } else {
            for (const auto& entry : fs::directory_iterator(path)) {
                if (fs::is_regular_file(entry.status())) {
                    FileInfo info;
                    info.originalPath = entry.path().string();
                    info.directory = extractDirectory(info.originalPath);
                    info.name = extractFileName(info.originalPath);
                    info.extension = extractExtension(info.originalPath);
                    info.newName = info.name;
                    
                    if (!fileFilter || fileFilter(info.name, info.extension, info.directory)) {
                        files.push_back(info);
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Error reading directory: " + std::string(e.what()));
    }
    
    std::sort(files.begin(), files.end(), [](const FileInfo& a, const FileInfo& b) {
        return a.name < b.name;
    });
    
    return files;
}

std::vector<FileInfo> FileUtils::getFilesInDirectory(const std::string& path, bool recursive) {
    std::vector<std::string> emptyExcludeDirs;
    return iterateDirectory(path, recursive, nullptr, emptyExcludeDirs);
}

std::vector<FileInfo> FileUtils::getFilesInDirectory(const std::string& path, bool recursive, const RenameOptions& options) {
    auto fileFilter = [&options](const std::string& name, const std::string& ext, const std::string& dir) {
        return shouldIncludeFile(name, ext, dir, options);
    };
    
    return iterateDirectory(path, recursive, fileFilter, options.excludeDirectories);
}

std::string FileUtils::extractDirectory(const std::string& path) {
    fs::path p(path);
    return p.parent_path().string();
}

std::string FileUtils::extractFileName(const std::string& path) {
    fs::path p(path);
    std::string filename = p.stem().string();
    return filename;
}

std::string FileUtils::extractExtension(const std::string& path) {
    fs::path p(path);
    std::string ext = p.extension().string();
    return ext;
}

std::string FileUtils::combinePath(const std::string& directory, const std::string& name, const std::string& extension) {
    fs::path p(directory);
    std::string fullName = name + extension;
    p /= fullName;
    return p.string();
}

bool FileUtils::renameFile(const std::string& oldPath, const std::string& newPath) {
    try {
        if (oldPath == newPath) {
            return true;
        }
        
        if (fs::exists(newPath)) {
            return false;
        }
        
        fs::rename(oldPath, newPath);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool FileUtils::fileExists(const std::string& path) {
    return fs::exists(path);
}

bool FileUtils::isDirectory(const std::string& path) {
    return fs::is_directory(path);
}

size_t FileUtils::getPathLength(const std::string& path) {
    return path.length();
}

size_t FileUtils::getMaxPathLength() {
#ifdef _WIN32
    return 260;
#else
    return 4096;
#endif
}

PathError FileUtils::validateFileName(const std::string& name) {
    if (name.empty()) {
        return PathError::EMPTY_PATH;
    }
    
#ifdef _WIN32
    static const char* windowsReservedNames[] = {
        "CON", "PRN", "AUX", "NUL",
        "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
        "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
    };
    
    std::string nameUpper = StringUtils::toUpper(name);
    size_t dotPos = nameUpper.find('.');
    std::string baseName = (dotPos != std::string::npos) ? nameUpper.substr(0, dotPos) : nameUpper;
    
    for (const char* reserved : windowsReservedNames) {
        if (baseName == reserved) {
            return PathError::RESERVED_NAME;
        }
    }
    
    for (char c : name) {
        if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' ||
            c == '"' || c == '<' || c == '>' || c == '|') {
            return PathError::INVALID_CHARACTERS;
        }
    }
#endif
    
    return PathError::OK;
}

PathError FileUtils::validateFilePath(const std::string& path) {
    size_t maxLen = getMaxPathLength();
    if (path.length() >= maxLen) {
        return PathError::TOO_LONG;
    }
    
    return PathError::OK;
}

std::string FileUtils::sanitizeFileName(const std::string& name, char replacement) {
    std::string result = name;
    
#ifdef _WIN32
    for (size_t i = 0; i < result.length(); ++i) {
        char c = result[i];
        if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' ||
            c == '"' || c == '<' || c == '>' || c == '|') {
            result[i] = replacement;
        }
    }
#endif
    
    return result;
}

bool FileUtils::isSymlink(const std::string& path) {
    try {
        return fs::is_symlink(path);
    } catch (const std::exception&) {
        return false;
    }
}

size_t FileUtils::utf8CharacterCount(const std::string& str) {
    size_t count = 0;
    for (size_t i = 0; i < str.length(); ++i) {
        unsigned char c = static_cast<unsigned char>(str[i]);
        if (c < 0x80) {
            count++;
        } else if (c < 0xC2) {
        } else if (c < 0xE0) {
            count++;
            i++;
        } else if (c < 0xF0) {
            count++;
            i += 2;
        } else if (c < 0xF8) {
            count++;
            i += 3;
        }
    }
    return count;
}

size_t FileUtils::utf8BytePosition(const std::string& str, size_t charIndex) {
    size_t bytePos = 0;
    size_t charCount = 0;
    
    while (bytePos < str.length() && charCount < charIndex) {
        unsigned char c = static_cast<unsigned char>(str[bytePos]);
        if (c < 0x80) {
            bytePos++;
        } else if (c < 0xC2) {
            bytePos++;
        } else if (c < 0xE0) {
            bytePos += 2;
        } else if (c < 0xF0) {
            bytePos += 3;
        } else if (c < 0xF8) {
            bytePos += 4;
        } else {
            bytePos++;
        }
        charCount++;
    }
    
    return bytePos;
}
