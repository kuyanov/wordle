#pragma once

#include <array>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

inline std::vector<std::string> ReadDictionary(const std::string &filename) {
    std::ifstream fin(filename);
    std::vector<std::string> dict;
    std::string word;
    while (fin >> word) {
        dict.push_back(word);
    }
    if (dict.empty()) {
        throw std::runtime_error("dictionary not found or empty");
    }
    return dict;
}

inline std::string Compare(const std::string &guess, const std::string &answer) {
    std::string result(guess.size(), '.');
    std::array<int, 26> cnt{};
    for (size_t i = 0; i < result.size(); ++i) {
        if (guess[i] == answer[i]) {
            result[i] = 'g';
        } else {
            ++cnt[answer[i] - 'a'];
        }
    }
    for (size_t i = 0; i < result.size(); ++i) {
        if (result[i] != 'g' && cnt[guess[i] - 'a'] > 0) {
            result[i] = 'y';
            --cnt[guess[i] - 'a'];
        }
    }
    return result;
}

inline void PrintColored(const std::string &guess, const std::string &result) {
    for (size_t i = 0; i < guess.size(); ++i) {
        if (result[i] == 'g') {
            std::cout << "\033[42m" << guess[i] << "\033[0m";
        } else if (result[i] == 'y') {
            std::cout << "\033[43m" << guess[i] << "\033[0m";
        } else {
            std::cout << guess[i];
        }
    }
    std::cout << std::endl;
}
