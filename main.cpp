#include <algorithm>
#include <array>
#include <iostream>
#include <ctime>
#include <fstream>
#include <random>
#include <unordered_map>
#include <utility>
#include <vector>

std::vector<std::string> ReadDict(std::ifstream &fin) {
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

class Host {
protected:
    std::vector<std::string> dict;

public:
    explicit Host(std::vector<std::string> dict) : dict(std::move(dict)) {}

    virtual std::string OnGuess(const std::string &) = 0;

    virtual std::string GetAnswer() = 0;
};

class Guesser {
protected:
    std::vector<std::string> dict;

public:
    explicit Guesser(std::vector<std::string> dict) : dict(std::move(dict)) {}

    virtual std::string MakeGuess() = 0;

    virtual void OnResult(const std::string &) = 0;
};

class HostStdio : public Host {
public:
    using Host::Host;

    std::string OnGuess(const std::string &guess) override {
        std::cout << guess << std::endl;
        std::string result;
        std::cin >> result;
        return result;
    }

    std::string GetAnswer() override {
        std::cout << "enter the answer: ";
        std::string answer;
        std::cin >> answer;
        return answer;
    }
};

class HostRandom : public Host {
    size_t ans_id;

public:
    explicit HostRandom(std::vector<std::string> dict_) : Host(std::move(dict_)) {
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
    explicit HostHater(std::vector<std::string> dict_) : Host(std::move(dict_)) {
        possibilities = dict;
    }

    std::string OnGuess(const std::string &guess) override {
        std::unordered_map<std::string, size_t> cnt;
        for (const auto &word: possibilities) {
            cnt[Compare(guess, word)]++;
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
        for (const auto &word: possibilities) {
            if (Compare(guess, word) == max_res) {
                new_possibilities.push_back(word);
            }
        }
        possibilities = new_possibilities;
        return max_res;
    }

    std::string GetAnswer() override {
        return possibilities[0];
    }
};

class GuesserStdio : public Guesser {
public:
    using Guesser::Guesser;

    std::string MakeGuess() override {
        std::string guess;
        std::cin >> guess;
        return guess;
    }

    void OnResult(const std::string &result) override {
        std::cout << result << std::endl;
    }
};

class Game {
    std::vector<std::string> dict;
    Host *host;
    Guesser *guesser;
    size_t max_moves;

public:
    Game(std::vector<std::string> dict, Host *host, Guesser *guesser, size_t max_moves = 6)
            : dict(std::move(dict)), host(host), guesser(guesser), max_moves(max_moves) {}

    void Play() {
        for (size_t move = 0; move < max_moves; ++move) {
            std::string guess = guesser->MakeGuess();
            if (std::find(dict.begin(), dict.end(), guess) == dict.end()) {
                std::cerr << "guesser error: word '" << guess << "' does not exist" << std::endl;
                return;
            }
            std::string result = host->OnGuess(guess);
            guesser->OnResult(result);
            if (std::count(result.begin(), result.end(), 'g') == result.size()) {
                std::cout << "guesser won" << std::endl;
                return;
            }
        }
        std::string answer = host->GetAnswer();
        if (std::find(dict.begin(), dict.end(), answer) == dict.end()) {
            std::cerr << "host error: word '" << answer << "' does not exist" << std::endl;
            return;
        }
        std::cout << "host won, the answer was '" << answer << "'" << std::endl;
    }
};

int main() {
    std::ifstream fin("dictionary.txt");
    auto dict = ReadDict(fin);
    HostHater host(dict);
    GuesserStdio guesser(dict);
    Game game(dict, &host, &guesser);
    game.Play();
}
