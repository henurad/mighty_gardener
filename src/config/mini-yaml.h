#pragma once

#include <algorithm>
#include <cctype>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace mini_yaml {

namespace detail {
inline std::string Trim(const std::string& value) {
    const std::string whitespace = " \t\r\n";
    const auto begin = value.find_first_not_of(whitespace);
    if (begin == std::string::npos) {
        return "";
    }

    const auto end = value.find_last_not_of(whitespace);
    return value.substr(begin, end - begin + 1);
}

inline std::string StripQuotes(const std::string& value) {
    std::string trimmed = Trim(value);
    if (trimmed.size() >= 2) {
        const char first = trimmed.front();
        const char last = trimmed.back();
        if ((first == '"' && last == '"') || (first == '\'' && last == '\'')) {
            return trimmed.substr(1, trimmed.size() - 2);
        }
    }
    return trimmed;
}

inline std::string NormalizeKey(const std::string& key) {
    std::string normalized = key;
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });
    return normalized;
}
}  // namespace detail

inline std::map<std::string, std::string> LoadFile(const std::string& file_path) {
    std::map<std::string, std::string> values;
    std::ifstream input(file_path);
    if (!input.is_open()) {
        return values;
    }

    std::string line;
    while (std::getline(input, line)) {
        std::string trimmed = detail::Trim(line);
        if (trimmed.empty() || trimmed[0] == '#') {
            continue;
        }

        const std::size_t colon_pos = trimmed.find(':');
        if (colon_pos == std::string::npos) {
            continue;
        }

        std::string key = detail::Trim(trimmed.substr(0, colon_pos));
        std::string value = detail::Trim(trimmed.substr(colon_pos + 1));
        if (key.empty()) {
            continue;
        }

        values[detail::NormalizeKey(key)] = detail::StripQuotes(value);
    }

    return values;
}

inline std::string NormalizeKey(const std::string& key) {
    return detail::NormalizeKey(key);
}

}  // namespace mini_yaml
