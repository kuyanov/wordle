#pragma once

#include <algorithm>
#include <array>
#include <ctime>
#include <fstream>
#include <iostream>
#include <random>
#include <regex>
#include <unordered_map>
#include <vector>

std::vector<std::string> ReadDictionary(const std::string &filename) {
    std::ifstream fin(filename);
    std::vector<std::string> dict;
    std::string word;
    while (fin >> word) {
        dict.push_back(word);
    }
    return dict;
}

std::string Compare(const std::string &guess, const std::string &ans) {
    std::string result(guess.size(), '.');
    std::array<int, 26> cnt{};
    for (size_t i = 0; i < result.size(); ++i) {
        if (guess[i] == ans[i]) {
            result[i] = 'g';
        } else {
            ++cnt[ans[i] - 'a'];
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

void PrintColored(const std::string &guess, const std::string &result) {
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
    size_t ans_id;

public:
    explicit HostFixed(const std::vector<std::string> &dict, size_t ans_id) : Host(dict), ans_id(ans_id) {}

    std::string OnGuess(const std::string &guess) override {
        return Compare(guess, dict[ans_id]);
    }

    std::string GetAnswer() override {
        return dict[ans_id];
    }
};

class HostRandom : public Host {
    size_t ans_id;

public:
    explicit HostRandom(const std::vector<std::string> &dict) : Host(dict) {
        std::mt19937 rnd(clock());
        ans_id = rnd() % dict.size();
    }

    std::string OnGuess(const std::string &guess) override {
        return Compare(guess, dict[ans_id]);
    }

    std::string GetAnswer() override {
        return dict[ans_id];
    }
};

class HostHater : public Host {
    std::vector<std::string> possibilities;

public:
    explicit HostHater(const std::vector<std::string> &dict) : Host(dict) {
        possibilities = dict;
    }

    std::string OnGuess(const std::string &guess) override {
        std::unordered_map<std::string, size_t> cnt;
        for (const auto &ans: possibilities) {
            cnt[Compare(guess, ans)]++;
        }
        size_t max_val = 0;
        std::string max_res(guess.size(), 'g');
        for (const auto &[res, val]: cnt) {
            if (std::count(res.begin(), res.end(), 'g') == res.size()) {
                continue;
            }
            if (val > max_val) {
                max_val = val;
                max_res = res;
            }
        }
        if (max_val == 0) {
            return max_res;
        }
        std::vector<std::string> new_possibilities;
        for (const auto &ans: possibilities) {
            if (Compare(guess, ans) == max_res) {
                new_possibilities.push_back(ans);
            }
        }
        possibilities = new_possibilities;
        return max_res;
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
            for (const auto &ans: possibilities) {
                if (guess != ans) {
                    cnt[Compare(guess, ans)]++;
                }
            }
            size_t sum = 0;
            for (const auto &[res, val]: cnt) {
                sum += val * val;
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
        for (const auto &ans: possibilities) {
            if (Compare(guess, ans) == result) {
                new_possibilities.push_back(ans);
            }
        }
        possibilities = new_possibilities;
    }
};
