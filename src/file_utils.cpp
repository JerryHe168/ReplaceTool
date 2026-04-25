#include "../include/renamer.h"
#include <filesystem>
#include <stdexcept>

namespace fs = std::filesystem;

std::string FileInfo::getFullPath() const {
    return originalPath;
}

std::string FileInfo::getNewFullPath() const {
    return FileUtils::combinePath(directory, newName, extension);
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
