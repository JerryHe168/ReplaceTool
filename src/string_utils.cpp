#include "../include/renamer.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <iomanip>

std::string StringUtils::toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

std::string StringUtils::toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::string StringUtils::capitalize(const std::string& str) {
    if (str.empty()) return str;
    
    std::string result = str;
    result[0] = std::toupper(static_cast<unsigned char>(result[0]));
    
    for (size_t i = 1; i < result.length(); ++i) {
        if (std::isspace(static_cast<unsigned char>(result[i - 1]))) {
            result[i] = std::toupper(static_cast<unsigned char>(result[i]));
        } else {
            result[i] = std::tolower(static_cast<unsigned char>(result[i]));
        }
    }
    
    return result;
}

std::string StringUtils::toCamelCase(const std::string& str) {
    if (str.empty()) return str;
    
    std::string result;
    bool capitalizeNext = false;
    bool isFirstChar = true;
    
    for (char c : str) {
        if (c == ' ' || c == '_' || c == '-' || c == '.') {
            capitalizeNext = true;
        } else {
            if (isFirstChar) {
                result += std::tolower(static_cast<unsigned char>(c));
                isFirstChar = false;
            } else if (capitalizeNext) {
                result += std::toupper(static_cast<unsigned char>(c));
                capitalizeNext = false;
            } else {
                result += std::tolower(static_cast<unsigned char>(c));
            }
        }
    }
    
    return result;
}

std::string StringUtils::toSnakeCase(const std::string& str) {
    if (str.empty()) return str;
    
    std::string result;
    bool prevWasUpper = false;
    
    for (size_t i = 0; i < str.length(); ++i) {
        char c = str[i];
        
        if (std::isupper(static_cast<unsigned char>(c))) {
            if (i > 0 && !prevWasUpper) {
                result += '_';
            }
            result += std::tolower(static_cast<unsigned char>(c));
            prevWasUpper = true;
        } else if (c == ' ' || c == '-' || c == '.') {
            result += '_';
            prevWasUpper = false;
        } else {
            result += c;
            prevWasUpper = false;
        }
    }
    
    std::string cleaned;
    bool prevUnderscore = false;
    for (char c : result) {
        if (c == '_') {
            if (!prevUnderscore) {
                cleaned += c;
            }
            prevUnderscore = true;
        } else {
            cleaned += c;
            prevUnderscore = false;
        }
    }
    
    if (!cleaned.empty() && cleaned.front() == '_') {
        cleaned = cleaned.substr(1);
    }
    if (!cleaned.empty() && cleaned.back() == '_') {
        cleaned.pop_back();
    }
    
    return cleaned;
}

std::string StringUtils::toKebabCase(const std::string& str) {
    std::string snake = toSnakeCase(str);
    std::replace(snake.begin(), snake.end(), '_', '-');
    return snake;
}

std::string StringUtils::replaceAll(const std::string& str, const std::string& search, const std::string& replace) {
    if (search.empty()) return str;
    
    std::string result;
    size_t pos = 0;
    size_t prevPos = 0;
    
    while ((pos = str.find(search, prevPos)) != std::string::npos) {
        result += str.substr(prevPos, pos - prevPos);
        result += replace;
        prevPos = pos + search.length();
    }
    
    result += str.substr(prevPos);
    return result;
}

std::string StringUtils::regexReplace(const std::string& str, const std::string& pattern, const std::string& replacement) {
    try {
        std::regex regexPattern(pattern);
        return std::regex_replace(str, regexPattern, replacement);
    } catch (const std::regex_error&) {
        return str;
    }
}

std::string StringUtils::formatNumber(int number, int padding) {
    std::stringstream ss;
    ss << std::setw(padding) << std::setfill('0') << number;
    return ss.str();
}

std::vector<std::string> StringUtils::split(const std::string& str, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;
    
    while (std::getline(ss, item, delimiter)) {
        if (!item.empty()) {
            result.push_back(item);
        }
    }
    
    return result;
}

std::string StringUtils::join(const std::vector<std::string>& parts, const std::string& delimiter) {
    if (parts.empty()) return "";
    
    std::string result = parts[0];
    for (size_t i = 1; i < parts.size(); ++i) {
        result += delimiter + parts[i];
    }
    return result;
}

std::string StringUtils::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    size_t end = str.find_last_not_of(" \t\n\r");
    
    if (start == std::string::npos) {
        return "";
    }
    
    return str.substr(start, end - start + 1);
}
