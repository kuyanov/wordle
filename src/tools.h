#pragma once

#include <array>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

std::vector<std::string> &GetDictAll() {
    static std::vector<std::string> dict_all;
    return dict_all;
}

std::vector<std::string> &GetDictAns() {
    static std::vector<std::string> dict_ans;
    return dict_ans;
}

inline void ReadDicts() {
    std::ifstream fin_all("all.txt"), fin_ans("ans.txt");
    std::string word;
    while (fin_all >> word) {
        GetDictAll().push_back(word);
    }
    while (fin_ans >> word) {
        GetDictAns().push_back(word);
    }
    if (GetDictAll().empty() || GetDictAns().empty()) {
        throw std::runtime_error("dictionary not found or empty");
    }
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
