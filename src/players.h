#pragma once

#include <algorithm>
#include <ctime>
#include <iostream>
#include <memory>
#include <random>
#include <regex>
#include <string>
#include <vector>

#include "common.h"
#include "decision_tree.h"

class Host {
public:
    virtual u_char OnGuess(size_t) = 0;

    virtual size_t GetAnswer() = 0;

    virtual ~Host() = default;
};

class HostStdio : public Host {
public:
    u_char OnGuess(size_t guess_id) override {
        std::cout << guesses[guess_id] << std::endl;
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
            size_t answer_id = std::find(answers.begin(), answers.end(), answer) - answers.begin();
            if (answer_id != answers.size()) {
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
    explicit HostFixed(size_t answer_id) : answer_id(answer_id) {}

    u_char OnGuess(size_t guess_id) override {
        return GetPattern(guess_id, answer_id);
    }

    size_t GetAnswer() override {
        return answer_id;
    }
};

class HostRandom : public Host {
    size_t answer_id;

public:
    HostRandom() {
        std::mt19937 rnd(clock());
        answer_id = rnd() % answers.size();
    }

    u_char OnGuess(size_t guess_id) override {
        return GetPattern(guess_id, answer_id);
    }

    size_t GetAnswer() override {
        return answer_id;
    }
};

class HostHater : public Host {
    std::vector<size_t> possible_answers;
    double randomness;
    std::mt19937 rnd;

public:
    explicit HostHater(double randomness) : randomness(randomness), rnd(clock()) {
        possible_answers.resize(answers.size());
        for (size_t answer_id = 0; answer_id < answers.size(); ++answer_id) {
            possible_answers[answer_id] = answer_id;
        }
    }

    u_char OnGuess(size_t guess_id) override {
        if (possible_answers.size() == 1 && guess_id == answer2guess[possible_answers[0]]) {
            return WIN_PAT;
        }
        std::vector<size_t> cnt(N_PATTERNS);
        for (size_t answer_id: possible_answers) {
            if (guess_id != answer2guess[answer_id]) {
                cnt[GetPattern(guess_id, answer_id)]++;
            }
        }
        std::vector<std::pair<size_t, u_char>> options;
        for (u_char pat = 0; pat < N_PATTERNS; ++pat) {
            if (cnt[pat] != 0) {
                options.emplace_back(cnt[pat], pat);
            }
        }
        std::sort(options.begin(), options.end());
        size_t max_value = options.back().first;
        size_t threshold = max_value * (1 - randomness);
        size_t l = std::lower_bound(options.begin(), options.end(),
                                    std::pair(threshold, u_char(0))) - options.begin();
        u_char pat = options[l + rnd() % (options.size() - l)].second;
        std::vector<size_t> new_possible_answers;
        for (size_t answer_id: possible_answers) {
            if (GetPattern(guess_id, answer_id) == pat) {
                new_possible_answers.push_back(answer_id);
            }
        }
        possible_answers = new_possible_answers;
        return pat;
    }

    size_t GetAnswer() override {
        return possible_answers[0];
    }
};

class Guesser {
public:
    virtual size_t MakeGuess() = 0;

    virtual void OnResult(u_char) = 0;

    virtual ~Guesser() = default;
};

class GuesserStdio : public Guesser {
    size_t guess_id;

public:
    size_t MakeGuess() override {
        while (true) {
            std::string guess;
            std::cin >> guess;
            guess_id = std::find(guesses.begin(), guesses.end(), guess) - guesses.begin();
            if (guess_id != guesses.size()) {
                return guess_id;
            }
            std::cout << "\x1b[1A";
            for (size_t i = 0; i < guess.size(); ++i) {
                std::cout << ' ';
            }
            std::cout << '\r';
        }
    }

    void OnResult(u_char pat) override {
        std::cout << "\x1b[1A";
        PrintColored(guesses[guess_id], DecodePattern(pat));
    }
};

class GuesserDecisionTree : public Guesser {
    std::shared_ptr<Node> cur;

public:
    explicit GuesserDecisionTree(const DecisionTree &tree) : cur(tree.Get()) {}

    size_t MakeGuess() override {
        return cur->guess_id;
    }

    void OnResult(u_char pat) override {
        cur = cur->go[pat];
    }
};
