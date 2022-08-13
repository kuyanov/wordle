#include <iostream>
#include <memory>
#include <tuple>

#include "players.h"

const int max_moves = 6;

enum class Player {
    HOST,
    GUESSER,
    UNDEFINED
};

std::tuple<Player, int, std::string> Play(const std::vector<std::string> &dict, Host *host, Guesser *guesser,
                                             bool print_game) {
    std::vector<std::string> guesses, results;
    for (int move = 1; move <= max_moves; ++move) {
        std::string guess = guesser->MakeGuess();
        std::string result = host->OnGuess(guess);
        if (print_game) {
            PrintColored(guess, result);
        }
        guesser->OnResult(guess, result);
        guesses.push_back(guess);
        results.push_back(result);
        if (std::count(result.begin(), result.end(), 'g') == result.size()) {
            return {Player::GUESSER, move, guess};
        }
    }
    std::string answer = host->GetAnswer();
    for (int move = 1; move <= max_moves; ++move) {
        if (Compare(guesses[move - 1], answer) != results[move - 1]) {
            std::cerr << "host error on move " << move << std::endl;
            return {Player::UNDEFINED, move, answer};
        }
    }
    return {Player::HOST, 0, answer};
}

int main() {
    auto dict = ReadDictionary("dictionary.txt");
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
    bool print_game = host_type != 3 && guesser_type != 1;
    auto [winner, move, answer] = Play(dict, host.get(), guesser.get(), print_game);
    if (winner == Player::GUESSER) {
        std::cout << "guesser won in " << move << " moves" << std::endl;
    } else if (winner == Player::HOST) {
        std::cout << "host won, the answer was '" << answer << "'" << std::endl;
    }
}
