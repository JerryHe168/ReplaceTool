#ifndef RENAMER_H
#define RENAMER_H

#include <string>
#include <vector>
#include <regex>
#include <set>

enum class RenameMode {
    ADD_PREFIX,
    ADD_SUFFIX,
    ADD_PREFIX_SUFFIX,
    REPLACE,
    REGEX_REPLACE,
    NUMBERING,
    UPPERCASE,
    LOWERCASE,
    CAPITALIZE,
    CAMEL_CASE,
    SNAKE_CASE,
    KEBAB_CASE
};

enum class ConflictResolution {
    SKIP,
    OVERWRITE,
    AUTO_RENAME
};

struct RenameOptions {
    RenameMode mode;
    std::string prefix;
    std::string suffix;
    std::string searchStr;
    std::string replaceStr;
    std::string regexPattern;
    std::string regexReplace;
    int startNumber;
    int padding;
    std::string numberFormat;
    std::string targetPath;
    ConflictResolution conflictResolution;
    bool includeExtensions;
    bool recursive;
    bool dryRun;
    bool verbose;
    
    RenameOptions() : 
        mode(RenameMode::ADD_PREFIX),
        startNumber(1),
        padding(0),
        conflictResolution(ConflictResolution::SKIP),
        includeExtensions(false),
        recursive(false),
        dryRun(false),
        verbose(false) {}
};

struct FileInfo {
    std::string originalPath;
    std::string directory;
    std::string name;
    std::string extension;
    std::string newName;
    
    std::string getFullPath() const;
    std::string getNewFullPath() const;
};

class FileUtils {
public:
    static std::vector<FileInfo> getFilesInDirectory(const std::string& path, bool recursive);
    static std::string extractDirectory(const std::string& path);
    static std::string extractFileName(const std::string& path);
    static std::string extractExtension(const std::string& path);
    static std::string combinePath(const std::string& directory, const std::string& name, const std::string& extension);
    static bool renameFile(const std::string& oldPath, const std::string& newPath);
    static bool fileExists(const std::string& path);
    static bool isDirectory(const std::string& path);
};

class StringUtils {
public:
    static std::string toUpper(const std::string& str);
    static std::string toLower(const std::string& str);
    static std::string capitalize(const std::string& str);
    static std::string toCamelCase(const std::string& str);
    static std::string toSnakeCase(const std::string& str);
    static std::string toKebabCase(const std::string& str);
    static std::string replaceAll(const std::string& str, const std::string& search, const std::string& replace);
    static std::string regexReplace(const std::string& str, const std::string& pattern, const std::string& replacement);
    static std::string formatNumber(int number, int padding);
    static std::vector<std::string> split(const std::string& str, char delimiter);
    static std::string join(const std::vector<std::string>& parts, const std::string& delimiter);
    static std::string trim(const std::string& str);
};

class RenameProcessor {
public:
    static std::string processFileName(const std::string& name, const RenameOptions& options, int index = 0);
    static void processFiles(std::vector<FileInfo>& files, const RenameOptions& options);
    static bool executeRename(std::vector<FileInfo>& files, const RenameOptions& options);
    static void resolveConflicts(std::vector<FileInfo>& files, ConflictResolution resolution);
    
private:
    static std::string addPrefix(const std::string& name, const std::string& prefix);
    static std::string addSuffix(const std::string& name, const std::string& suffix);
    static std::string addPrefixAndSuffix(const std::string& name, const std::string& prefix, const std::string& suffix);
    static std::string applyNumbering(const std::string& name, const std::string& format, int startNumber, int index, int padding);
    static std::string applyCaseTransformation(const std::string& name, RenameMode mode);
    static std::string getNextAutoRename(const std::string& name, int counter);
    static bool isNameUnique(size_t currentIndex, const std::string& testName, 
                               const std::string& testExtension,
                               const std::vector<FileInfo>& files);
    static bool checkAndResolveAllConflicts(std::vector<FileInfo>& files, ConflictResolution resolution);
};

class ArgParser {
public:
    static void printHelp();
    static RenameOptions parseArguments(int argc, char* argv[]);
    static void validateOptions(const RenameOptions& options);
};

#endif
