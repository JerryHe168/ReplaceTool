#include "../include/renamer.h"
#include <filesystem>
#include <stdexcept>
#include <algorithm>

namespace fs = std::filesystem;

std::string FileInfo::getFullPath() const {
    return originalPath;
}

std::string FileInfo::getNewFullPath() const {
    return FileUtils::combinePath(directory, newName, extension);
}

bool FileUtils::matchesExtensions(const std::string& extension, const std::vector<std::string>& extList) {
    if (extList.empty()) {
        return true;
    }
    
    std::string extLower = StringUtils::toLower(extension);
    
    for (const auto& ext : extList) {
        std::string patternLower = StringUtils::toLower(ext);
        if (extLower == patternLower || (patternLower[0] != '.' && "." + extLower == patternLower)) {
            return true;
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
        if (extLower == patternLower || (patternLower[0] != '.' && "." + extLower == patternLower)) {
            return false;
        }
    }
    
    if (!options.excludeRegex.empty()) {
        if (matchesRegex(fullName, options.excludeRegex)) {
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
        include = include && matchesRegex(fullName, options.includeRegex);
    }
    
    return include;
}

bool shouldExcludeDirectory(const std::string& dirName, const std::vector<std::string>& excludeDirs) {
    for (const auto& excludeDir : excludeDirs) {
        if (dirName == excludeDir) {
            return true;
        }
    }
    return false;
}

std::vector<FileInfo> FileUtils::getFilesInDirectory(const std::string& path, bool recursive) {
    std::vector<FileInfo> files;
    
    if (!fs::exists(path)) {
        throw std::runtime_error("Path does not exist: " + path);
    }
    
    try {
        if (recursive) {
            for (const auto& entry : fs::recursive_directory_iterator(path)) {
                if (fs::is_regular_file(entry.status())) {
                    FileInfo info;
                    info.originalPath = entry.path().string();
                    info.directory = extractDirectory(info.originalPath);
                    info.name = extractFileName(info.originalPath);
                    info.extension = extractExtension(info.originalPath);
                    info.newName = info.name;
                    files.push_back(info);
                }
            }
        } else {
            for (const auto& entry : fs::directory_iterator(path)) {
                if (fs::is_regular_file(entry.status())) {
                    FileInfo info;
                    info.originalPath = entry.path().string();
                    info.directory = extractDirectory(info.originalPath);
                    info.name = extractFileName(info.originalPath);
                    info.extension = extractExtension(info.originalPath);
                    info.newName = info.name;
                    files.push_back(info);
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

std::vector<FileInfo> FileUtils::getFilesInDirectory(const std::string& path, bool recursive, const RenameOptions& options) {
    std::vector<FileInfo> files;
    
    if (!fs::exists(path)) {
        throw std::runtime_error("Path does not exist: " + path);
    }
    
    try {
        if (recursive) {
            for (const auto& entry : fs::recursive_directory_iterator(path)) {
                if (fs::is_regular_file(entry.status())) {
                    FileInfo info;
                    info.originalPath = entry.path().string();
                    info.directory = extractDirectory(info.originalPath);
                    info.name = extractFileName(info.originalPath);
                    info.extension = extractExtension(info.originalPath);
                    info.newName = info.name;
                    
                    if (!options.excludeDirectories.empty()) {
                        fs::path entryPath = entry.path();
                        bool inExcludedDir = false;
                        for (auto it = entryPath.begin(); it != entryPath.end(); ++it) {
                            if (shouldExcludeDirectory(it->string(), options.excludeDirectories)) {
                                inExcludedDir = true;
                                break;
                            }
                        }
                        if (inExcludedDir) {
                            continue;
                        }
                    }
                    
                    if (shouldIncludeFile(info.name, info.extension, info.directory, options)) {
                        files.push_back(info);
                    }
                }
            }
        } else {
            for (const auto& entry : fs::directory_iterator(path)) {
                if (fs::is_regular_file(entry.status())) {
                    FileInfo info;
                    info.originalPath = entry.path().string();
                    info.directory = extractDirectory(info.originalPath);
                    info.name = extractFileName(info.originalPath);
                    info.extension = extractExtension(info.originalPath);
                    info.newName = info.name;
                    
                    if (shouldIncludeFile(info.name, info.extension, info.directory, options)) {
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
