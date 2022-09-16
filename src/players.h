#pragma once

#include <algorithm>
#include <ctime>
#include <iostream>
#include <random>
#include <regex>
#include <unordered_map>
#include <vector>

#include "tools.h"

class Host {
protected:
    const std::vector<std::string> &dict;

public:
    explicit Host(const std::vector<std::string> &dict) : dict(dict) {}

    virtual std::string OnGuess(const std::string &) = 0;

    virtual std::string GetAnswer() = 0;

    virtual ~Host() = default;
};

class HostStdio : public Host {
public:
    using Host::Host;

    std::string OnGuess(const std::string &guess) override {
        std::cout << guess << std::endl;
        while (true) {
            std::string result;
            std::cin >> result;
            if (result.size() == guess.size() && std::regex_match(result, std::regex("[\\.|y|g]+"))) {
                return result;
            }
            std::cout << "\x1b[1A";
            for (size_t i = 0; i < result.size(); ++i) {
                std::cout << ' ';
            }
            std::cout << '\r';
        }
    }

    std::string GetAnswer() override {
        while (true) {
            std::cout << "enter the answer: ";
            std::string answer;
            std::cin >> answer;
            if (std::find(dict.begin(), dict.end(), answer) != dict.end()) {
                return answer;
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
    explicit HostFixed(const std::vector<std::string> &dict, size_t answer_id) : Host(dict), answer_id(answer_id) {}

    std::string OnGuess(const std::string &guess) override {
        return Compare(guess, dict[answer_id]);
    }

    std::string GetAnswer() override {
        return dict[answer_id];
    }
};

class HostRandom : public Host {
    size_t answer_id;

public:
    explicit HostRandom(const std::vector<std::string> &dict) : Host(dict) {
        std::mt19937 rnd(clock());
        answer_id = rnd() % dict.size();
    }

    std::string OnGuess(const std::string &guess) override {
        return Compare(guess, dict[answer_id]);
    }

    std::string GetAnswer() override {
        return dict[answer_id];
    }
};

class HostHater : public Host {
    std::vector<std::string> possibilities;
    double randomness;
    std::mt19937 rnd;

public:
    explicit HostHater(const std::vector<std::string> &dict, double randomness = 0) : Host(dict),
                                                                                      randomness(randomness),
                                                                                      rnd(clock()) {
        possibilities = dict;
    }

    std::string OnGuess(const std::string &guess) override {
        std::unordered_map<std::string, size_t> cnt;
        for (const auto &answer: possibilities) {
            if (answer == guess) {
                continue;
            }
            auto result = Compare(guess, answer);
            cnt[result]++;
        }
        if (cnt.empty()) {
            std::string result(guess.size(), 'g');
            return result;
        }
        std::vector<std::pair<std::string, size_t>> options(cnt.begin(), cnt.end());
        std::sort(options.begin(), options.end(), [&](const auto &p1, const auto &p2) {
            return p1.second < p2.second;
        });
        size_t max_value = options.back().second;
        size_t threshold = max_value * (1 - randomness);
        size_t l = std::lower_bound(options.begin(), options.end(),
                                    std::pair("", threshold),
                                    [&](const auto &p1, const auto &p2) {
                                        return p1.second < p2.second;
                                    }) - options.begin();
        auto result = options[l + rnd() % (options.size() - l)].first;
        std::vector<std::string> new_possibilities;
        for (const auto &answer: possibilities) {
            if (Compare(guess, answer) == result) {
                new_possibilities.push_back(answer);
            }
        }
        possibilities = new_possibilities;
        return result;
    }

    std::string GetAnswer() override {
        return possibilities[0];
    }
};

class Guesser {
protected:
    const std::vector<std::string> &dict;

public:
    explicit Guesser(const std::vector<std::string> &dict) : dict(dict) {}

    virtual std::string MakeGuess() = 0;

    virtual void OnResult(const std::string &, const std::string &) = 0;

    virtual ~Guesser() = default;
};

class GuesserStdio : public Guesser {
public:
    using Guesser::Guesser;

    std::string MakeGuess() override {
        while (true) {
            std::string guess;
            std::cin >> guess;
            if (std::find(dict.begin(), dict.end(), guess) != dict.end()) {
                return guess;
            }
            std::cout << "\x1b[1A";
            for (size_t i = 0; i < guess.size(); ++i) {
                std::cout << ' ';
            }
            std::cout << '\r';
        }
    }

    void OnResult(const std::string &guess, const std::string &result) override {
        std::cout << "\x1b[1A";
        PrintColored(guess, result);
    }
};

class GuesserHeuristic : public Guesser {
    std::vector<std::string> possibilities;

public:
    explicit GuesserHeuristic(const std::vector<std::string> &dict) : Guesser(dict) {
        possibilities = dict;
    }

    std::string MakeGuess() override {
        size_t min_sum = dict.size() * dict.size();
        std::string best_guess;
        for (const auto &guess: dict) {
            std::unordered_map<std::string, size_t> cnt;
            for (const auto &answer: possibilities) {
                if (guess != answer) {
                    cnt[Compare(guess, answer)]++;
                }
            }
            size_t sum = 0;
            for (const auto &[result, value]: cnt) {
                sum += value * value;
            }
            if (sum < min_sum) {
                min_sum = sum;
                best_guess = guess;
            }
        }
        return best_guess;
    }

    void OnResult(const std::string &guess, const std::string &result) override {
        std::vector<std::string> new_possibilities;
        for (const auto &answer: possibilities) {
            if (Compare(guess, answer) == result) {
                new_possibilities.push_back(answer);
            }
        }
        possibilities = new_possibilities;
    }
};
