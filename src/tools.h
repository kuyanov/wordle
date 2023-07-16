#pragma once

#include <array>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

static std::vector<std::string> all, twl;
static std::vector<size_t> twl2all_map;
static std::vector<double> priors;
static std::vector<std::vector<u_char>> pattern_mat;

inline void ReadWords() {
    std::ifstream fin_all("data/all.txt"), fin_twl("data/twl.txt");
    std::string word;
    while (fin_all >> word) {
        all.push_back(word);
    }
    while (fin_twl >> word) {
        twl.push_back(word);
    }
    if (all.empty() || twl.empty()) {
        throw std::runtime_error("words not found");
    }
    std::unordered_map<std::string, size_t> all_index;
    for (size_t i = 0; i < all.size(); ++i) {
        all_index[all[i]] = i;
    }
    twl2all_map.resize(twl.size());
    for (size_t i = 0; i < twl.size(); ++i) {
        twl2all_map[i] = all_index[twl[i]];
    }
}

inline void ComputeSigmoidPriors(double width, int n_common) {
    size_t n = all.size();
    priors.resize(n);
    for (int id = 0; id < n; ++id) {
        double x = -width * double(id - n_common) / double(n - 1);
        priors[id] = 1 / (1 + exp(-x));
    }
}

inline u_char GetPattern(const std::string &guess, const std::string &answer) {
    u_char pat = 0;
    std::array<int, 26> cnt{};
    std::array<u_char, 5> pw3 = {81, 27, 9, 3, 1};
    for (size_t i = 0; i < 5; ++i) {
        if (guess[i] == answer[i]) {
            pat += pw3[i];
        } else {
            ++cnt[answer[i] - 'a'];
        }
    }
    for (size_t i = 0; i < 5; ++i) {
        if (guess[i] != answer[i] && cnt[guess[i] - 'a'] > 0) {
            pat += 2 * pw3[i];
            --cnt[guess[i] - 'a'];
        }
    }
    return pat;
}

inline u_char EncodePattern(const std::string &pattern) {
    u_char pat = 0;
    for (char c: pattern) {
        pat = pat * 3 + (c == '.' ? 0 : c == 'g' ? 1 : 2);
    }
    return pat;
}

inline std::string DecodePattern(u_char pat) {
    std::string pattern(5, '.');
    for (size_t i = 4; i < 5; --i) {
        pattern[i] = (pat % 3 == 0 ? '.' : pat % 3 == 1 ? 'g' : 'y');
        pat /= 3;
    }
    return pattern;
}

inline bool IsAllGreen(u_char pat) {
    return pat == 81 + 27 + 9 + 3 + 1;
}

inline void ComputePatternMatrix() {
    pattern_mat.resize(all.size(), std::vector<u_char>(all.size()));
    for (size_t i = 0; i < all.size(); ++i) {
        for (size_t j = 0; j < all.size(); ++j) {
            pattern_mat[i][j] = GetPattern(all[i], all[j]);
        }
    }
}

inline void Init() {
    ReadWords();
    ComputeSigmoidPriors(20, 3000);
    ComputePatternMatrix();
}

inline void PrintColored(const std::string &guess, const std::string &pattern) {
    for (size_t i = 0; i < guess.size(); ++i) {
        if (pattern[i] == 'g') {
            std::cout << "\033[42m" << guess[i] << "\033[0m";
        } else if (pattern[i] == 'y') {
            std::cout << "\033[43m" << guess[i] << "\033[0m";
        } else {
            std::cout << guess[i];
        }
    }
    std::cout << std::endl;
}
