#pragma once

#include <array>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

const u_char N_PATTERNS = 243;
const u_char WIN_PAT = 81 + 27 + 9 + 3 + 1;

std::vector<std::string> guesses, answers;
std::vector<size_t> answer2guess;
std::vector<std::vector<u_char>> patterns;

void ReadWords() {
    std::ifstream fin_guesses("data/guesses.txt"), fin_answers("data/answers.txt");
    std::string word;
    while (fin_guesses >> word) {
        guesses.push_back(word);
    }
    while (fin_answers >> word) {
        answers.push_back(word);
    }
    if (guesses.empty() || answers.empty()) {
        throw std::runtime_error("words not found");
    }
    std::unordered_map<std::string, size_t> guess_pos;
    for (size_t guess_id = 0; guess_id < guesses.size(); ++guess_id) {
        guess_pos[guesses[guess_id]] = guess_id;
    }
    answer2guess.resize(answers.size());
    for (size_t answer_id = 0; answer_id < answers.size(); ++answer_id) {
        answer2guess[answer_id] = guess_pos[answers[answer_id]];
    }
}

u_char ComputePattern(const std::string &guess, const std::string &answer) {
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

u_char EncodePattern(const std::string &pattern) {
    u_char pat = 0;
    for (char c: pattern) {
        pat = pat * 3 + (c == '.' ? 0 : c == 'g' ? 1 : 2);
    }
    return pat;
}

std::string DecodePattern(u_char pat) {
    std::string pattern(5, '.');
    for (size_t i = 4; i < 5; --i) {
        pattern[i] = (pat % 3 == 0 ? '.' : pat % 3 == 1 ? 'g' : 'y');
        pat /= 3;
    }
    return pattern;
}

void ComputePatterns() {
    patterns.resize(guesses.size(), std::vector<u_char>(answers.size()));
    for (size_t guess_id = 0; guess_id < guesses.size(); ++guess_id) {
        for (size_t answer_id = 0; answer_id < answers.size(); ++answer_id) {
            patterns[guess_id][answer_id] = ComputePattern(guesses[guess_id], answers[answer_id]);
        }
    }
}

u_char GetPattern(size_t guess_id, size_t answer_id) {
    return !patterns.empty() ? patterns[guess_id][answer_id]
                             : ComputePattern(guesses[guess_id], answers[answer_id]);
}

void PrintColored(const std::string &guess, const std::string &pattern) {
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
