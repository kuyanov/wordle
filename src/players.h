#pragma once

#include <algorithm>
#include <ctime>
#include <iostream>
#include <random>
#include <regex>
#include <vector>

#include "decision_tree.h"
#include "tools.h"

class Host {
public:
    virtual u_char OnGuess(size_t) = 0;

    virtual size_t GetAnswer() = 0;

    virtual ~Host() = default;
};

class HostStdio : public Host {
public:
    u_char OnGuess(size_t guess_id) override {
        std::cout << all[guess_id] << std::endl;
        while (true) {
            std::string pattern;
            std::cin >> pattern;
            if (pattern.size() == 5 && std::regex_match(pattern, std::regex("[\\.|y|g]+"))) {
                return EncodePattern(pattern);
            }
            std::cout << "\x1b[1A";
            for (size_t i = 0; i < pattern.size(); ++i) {
                std::cout << ' ';
            }
            std::cout << '\r';
        }
    }

    size_t GetAnswer() override {
        while (true) {
            std::cout << "enter the answer: ";
            std::string answer;
            std::cin >> answer;
            size_t answer_id = std::find(all.begin(), all.end(), answer) - all.begin();
            if (answer_id != all.size()) {
                return answer_id;
            }
            std::cout << "\x1b[1A";
            for (size_t i = 0; i < answer.size() + 18; ++i) {
                std::cout << ' ';
            }
            std::cout << '\r';
        }
    }
};

class HostFixed : public Host {
    size_t answer_id;

public:
    HostFixed(size_t answer_id, bool use_twl) :
            answer_id(use_twl ? twl2all_map[answer_id] : answer_id) {}

    u_char OnGuess(size_t guess_id) override {
        return pattern_mat[guess_id][answer_id];
    }

    size_t GetAnswer() override {
        return answer_id;
    }
};

class HostRandom : public Host {
    size_t answer_id;

public:
    explicit HostRandom(bool use_twl) {
        std::mt19937 rnd(clock());
        answer_id = use_twl ? twl2all_map[rnd() % twl.size()] : rnd() % all.size();
    }

    u_char OnGuess(size_t guess_id) override {
        return pattern_mat[guess_id][answer_id];
    }

    size_t GetAnswer() override {
        return answer_id;
    }
};

class HostHater : public Host {
    std::vector<size_t> possible_words;
    double randomness;
    std::mt19937 rnd;

public:
    explicit HostHater(double randomness, bool use_twl) : randomness(randomness), rnd(clock()) {
        if (use_twl) {
            possible_words = twl2all_map;
        } else {
            possible_words.resize(all.size());
            for (size_t i = 0; i < possible_words.size(); ++i) possible_words[i] = i;
        }
    }

    u_char OnGuess(size_t guess_id) override {
        std::vector<size_t> cnt(243);
        if (possible_words.size() == 1 && possible_words.front() == guess_id) {
            return pattern_mat[guess_id][guess_id];
        }
        for (size_t answer_id: possible_words) {
            if (answer_id != guess_id) {
                cnt[pattern_mat[guess_id][answer_id]]++;
            }
        }
        std::vector<std::pair<size_t, u_char>> options;
        for (size_t i = 0; i < cnt.size(); ++i) {
            if (cnt[i] != 0) {
                options.emplace_back(cnt[i], i);
            }
        }
        std::sort(options.begin(), options.end());
        size_t max_value = options.back().first;
        size_t threshold = max_value * (1 - randomness);
        size_t l = std::lower_bound(options.begin(), options.end(),
                                    std::pair(threshold, u_char(0))) - options.begin();
        u_char pat = options[l + rnd() % (options.size() - l)].second;
        std::vector<size_t> new_possible_words;
        for (size_t answer_id: possible_words) {
            if (pattern_mat[guess_id][answer_id] == pat) {
                new_possible_words.push_back(answer_id);
            }
        }
        possible_words = new_possible_words;
        return pat;
    }

    size_t GetAnswer() override {
        return possible_words.front();
    }
};

class Guesser {
public:
    virtual size_t MakeGuess() = 0;

    virtual void OnResult(size_t, u_char) = 0;

    virtual ~Guesser() = default;
};

class GuesserStdio : public Guesser {
public:
    size_t MakeGuess() override {
        while (true) {
            std::string guess;
            std::cin >> guess;
            size_t guess_id = std::find(all.begin(), all.end(), guess) - all.begin();
            if (guess_id != all.size()) {
                return guess_id;
            }
            std::cout << "\x1b[1A";
            for (size_t i = 0; i < guess.size(); ++i) {
                std::cout << ' ';
            }
            std::cout << '\r';
        }
    }

    void OnResult(size_t guess_id, u_char pat) override {
        std::cout << "\x1b[1A";
        PrintColored(all[guess_id], DecodePattern(pat));
    }
};

class GuesserHeuristic : public Guesser {
    Node *cur;

public:
    GuesserHeuristic(bool use_twl, bool use_priors) {
        if (!root) {
            root = std::make_unique<Node>();
            std::cerr << "building decision tree" << std::endl;
            if (use_twl) {
                BuildDecisionTree(root.get(), twl2all_map, use_priors);
            } else {
                std::vector<size_t> possible_words(all.size());
                for (size_t i = 0; i < possible_words.size(); ++i) possible_words[i] = i;
                BuildDecisionTree(root.get(), possible_words, use_priors);
            }
        }
        cur = root.get();
    }

    size_t MakeGuess() override {
        return cur->guess_id;
    }

    void OnResult(size_t guess_id, u_char pat) override {
        cur = cur->go[pat].get();
    }
};
