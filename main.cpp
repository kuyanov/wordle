#include <algorithm>
#include <array>
#include <iostream>
#include <ctime>
#include <fstream>
#include <memory>
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
    std::vector<std::string> dict;

public:
    explicit Host(std::vector<std::string> dict) : dict(std::move(dict)) {}

    virtual std::string OnGuess(const std::string &) = 0;

    virtual std::string GetAnswer() = 0;

    virtual ~Host() = default;
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

class HostFixed : public Host {
    size_t ans_id;

public:
    explicit HostFixed(std::vector<std::string> dict_, size_t ans_id) : Host(std::move(dict_)), ans_id(ans_id) {}

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
    std::vector<std::string> dict;

public:
    explicit Guesser(std::vector<std::string> dict) : dict(std::move(dict)) {}

    virtual std::string MakeGuess() = 0;

    virtual void OnResult(const std::string &, const std::string &) = 0;

    virtual ~Guesser() = default;
};

class GuesserStdio : public Guesser {
public:
    using Guesser::Guesser;

    std::string MakeGuess() override {
        std::string guess;
        std::cin >> guess;
        return guess;
    }

    void OnResult(const std::string &guess, const std::string &result) override {
        std::cout << "\x1b[1A";
        PrintColored(guess, result);
    }
};

class GuesserHeuristic : public Guesser {
    std::vector<std::string> possibilities;

public:
    explicit GuesserHeuristic(std::vector<std::string> dict_) : Guesser(std::move(dict_)) {
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

enum class Player {
    HOST,
    GUESSER,
    UNDEFINED
};

class Game {
    std::vector<std::string> dict;
    Host *host;
    Guesser *guesser;
    size_t move = 0, max_moves;
    std::string answer;
    Player winner = Player::UNDEFINED;

public:
    Game(std::vector<std::string> dict, Host *host, Guesser *guesser, size_t max_moves = 6)
            : dict(std::move(dict)), host(host), guesser(guesser), max_moves(max_moves) {}

    void Play(bool print_game) {
        std::vector<std::string> guesses, results;
        for (move = 1; move <= max_moves; ++move) {
            std::string guess = guesser->MakeGuess();
            if (std::find(dict.begin(), dict.end(), guess) == dict.end()) {
                std::cerr << "guesser error: word '" << guess << "' does not exist" << std::endl;
                return;
            }
            std::string result = host->OnGuess(guess);
            if (print_game) {
                PrintColored(guess, result);
            }
            guesser->OnResult(guess, result);
            guesses.push_back(guess);
            results.push_back(result);
            if (std::count(result.begin(), result.end(), 'g') == result.size()) {
                answer = guess;
                winner = Player::GUESSER;
                return;
            }
        }
        answer = host->GetAnswer();
        if (std::find(dict.begin(), dict.end(), answer) == dict.end()) {
            std::cerr << "host error: word '" << answer << "' does not exist" << std::endl;
            return;
        }
        for (size_t i = 0; i < max_moves; ++i) {
            if (Compare(guesses[i], answer) != results[i]) {
                std::cerr << "host error on move " << i + 1 << std::endl;
                return;
            }
        }
        winner = Player::HOST;
    }

    [[nodiscard]] size_t GetMove() const {
        return move;
    }

    [[nodiscard]] std::string GetAnswer() const {
        return answer;
    }

    [[nodiscard]] Player GetWinner() const {
        return winner;
    }
};

int main() {
    std::ifstream fin("dictionary.txt");
    auto dict = ReadDict(fin);
    std::cout << "host type (1 - fixed, 2 - random, 3 - stdio, 4 - hater): ";
    int host_type;
    std::cin >> host_type;
    std::unique_ptr<Host> host;
    if (host_type == 1) {
        std::cout << "word id (0 ... " << dict.size() - 1 << "): ";
        size_t ans_id;
        std::cin >> ans_id;
        host = std::make_unique<HostFixed>(dict, ans_id);
    } else if (host_type == 2) {
        host = std::make_unique<HostRandom>(dict);
    } else if (host_type == 3) {
        host = std::make_unique<HostStdio>(dict);
    } else if (host_type == 4) {
        host = std::make_unique<HostHater>(dict);
    } else {
        std::cout << "unknown type, exiting" << std::endl;
        return 0;
    }
    std::cout << "guesser type (1 - stdio, 2 - heuristic): ";
    int guesser_type;
    std::cin >> guesser_type;
    std::unique_ptr<Guesser> guesser;
    if (guesser_type == 1) {
        guesser = std::make_unique<GuesserStdio>(dict);
    } else if (guesser_type == 2) {
        guesser = std::make_unique<GuesserHeuristic>(dict);
    } else {
        std::cout << "unknown type, exiting" << std::endl;
        return 0;
    }
    Game game(dict, host.get(), guesser.get());
    game.Play(host_type != 3 && guesser_type != 1);
    Player winner = game.GetWinner();
    if (winner == Player::GUESSER) {
        std::cout << "guesser won" << std::endl;
    } else if (winner == Player::HOST) {
        std::cout << "host won, the answer was '" << game.GetAnswer() << "'" << std::endl;
    }
}
