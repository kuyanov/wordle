#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "players.h"

const int max_moves = 6;

enum class Player {
    HOST,
    GUESSER,
    UNDEFINED
};

std::tuple<Player, int, std::string> Play(Host *host, Guesser *guesser, bool print_game) {
    std::vector<size_t> guesses;
    std::vector<u_char> patterns;
    for (int move = 1; move <= max_moves; ++move) {
        size_t guess_id = guesser->MakeGuess();
        u_char pat = host->OnGuess(guess_id);
        guesser->OnResult(guess_id, pat);
        guesses.push_back(guess_id);
        patterns.push_back(pat);
        std::string guess = all[guess_id], pattern = DecodePattern(pat);
        if (print_game) {
            PrintColored(guess, pattern);
        }
        if (IsAllGreen(pat)) {
            return {Player::GUESSER, move, guess};
        }
    }
    size_t answer_id = host->GetAnswer();
    for (int move = 1; move <= max_moves; ++move) {
        if (pattern_mat[guesses[move - 1]][answer_id] != patterns[move - 1]) {
            std::cerr << "host error on move " << move << std::endl;
            return {Player::UNDEFINED, move, all[answer_id]};
        }
    }
    return {Player::HOST, 0, all[answer_id]};
}

int main() {
    Init();
    std::cout << "host type (1 - fixed, 2 - random, 3 - stdio, 4 - hater): ";
    int host_type;
    std::cin >> host_type;
    std::unique_ptr<Host> host;
    if (host_type == 1) {
        std::cout << "use true wordle list (0/1): ";
        bool use_twl;
        std::cin >> use_twl;
        std::cout << "word id (0 ... " << (use_twl ? twl.size() - 1 : all.size() - 1) << "): ";
        size_t answer_id;
        std::cin >> answer_id;
        host = std::make_unique<HostFixed>(answer_id, use_twl);
    } else if (host_type == 2) {
        std::cout << "use true wordle list (0/1): ";
        bool use_twl;
        std::cin >> use_twl;
        host = std::make_unique<HostRandom>(use_twl);
    } else if (host_type == 3) {
        host = std::make_unique<HostStdio>();
    } else if (host_type == 4) {
        std::cout << "use true wordle list (0/1): ";
        bool use_twl;
        std::cin >> use_twl;
        host = std::make_unique<HostHater>(0.2, use_twl);
    } else {
        std::cout << "unknown type, exiting" << std::endl;
        return 1;
    }
    std::cout << "guesser type (1 - stdio, 2 - heuristic): ";
    int guesser_type;
    std::cin >> guesser_type;
    std::unique_ptr<Guesser> guesser;
    if (guesser_type == 1) {
        guesser = std::make_unique<GuesserStdio>();
    } else if (guesser_type == 2) {
        std::cout << "use true wordle list (0/1): ";
        bool use_twl;
        std::cin >> use_twl;
        std::cout << "use frequency priors (0/1): ";
        bool use_priors;
        std::cin >> use_priors;
        guesser = std::make_unique<GuesserHeuristic>(use_twl, use_priors);
    } else {
        std::cout << "unknown type, exiting" << std::endl;
        return 1;
    }
    bool print_game = host_type != 3 && guesser_type != 1;
    auto [winner, move, answer] = Play(host.get(), guesser.get(), print_game);
    if (winner == Player::GUESSER) {
        std::cout << "guesser won in " << move << " moves" << std::endl;
    } else if (winner == Player::HOST) {
        std::cout << "host won, the answer was '" << answer << "'" << std::endl;
    }
}
