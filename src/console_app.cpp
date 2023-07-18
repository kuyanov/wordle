#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "players.h"

const int MAX_MOVES = 6;

enum class Player {
    HOST,
    GUESSER,
    UNDEFINED
};

std::tuple<Player, int, std::string> Play(Host *host, Guesser *guesser, bool print_game) {
    std::vector<size_t> game_guesses;
    std::vector<u_char> game_patterns;
    for (int i = 0; i < MAX_MOVES; ++i) {
        size_t guess_id = guesser->MakeGuess();
        u_char pat = host->OnGuess(guess_id);
        guesser->OnResult(pat);
        game_guesses.push_back(guess_id);
        game_patterns.push_back(pat);
        std::string guess = guesses[guess_id], pattern = DecodePattern(pat);
        if (print_game) {
            PrintColored(guess, pattern);
        }
        if (pat == WIN_PAT) {
            return {Player::GUESSER, i + 1, guess};
        }
    }
    size_t answer_id = host->GetAnswer();
    for (int i = 0; i < MAX_MOVES; ++i) {
        if (GetPattern(game_guesses[i], answer_id) != game_patterns[i]) {
            std::cerr << "host error on move " << i + 1 << std::endl;
            return {Player::UNDEFINED, i + 1, answers[answer_id]};
        }
    }
    return {Player::HOST, 0, answers[answer_id]};
}

int main() {
    ReadWords();
    std::cout << "host type (1 - fixed, 2 - random, 3 - stdio, 4 - hater): ";
    int host_type;
    std::cin >> host_type;
    std::unique_ptr<Host> host;
    if (host_type == 1) {
        std::cout << "word id (0 ... " << answers.size() - 1 << "): ";
        size_t answer_id;
        std::cin >> answer_id;
        host = std::make_unique<HostFixed>(answer_id);
    } else if (host_type == 2) {
        host = std::make_unique<HostRandom>();
    } else if (host_type == 3) {
        host = std::make_unique<HostStdio>();
    } else if (host_type == 4) {
        host = std::make_unique<HostHater>(0.2);
    } else {
        std::cout << "unknown type, exiting" << std::endl;
        return 1;
    }
    std::cout << "guesser type (1 - stdio, 2 - decision tree): ";
    int guesser_type;
    std::cin >> guesser_type;
    std::unique_ptr<Guesser> guesser;
    if (guesser_type == 1) {
        guesser = std::make_unique<GuesserStdio>();
    } else if (guesser_type == 2) {
        std::cout << "filename (default greedy_top10): ";
        std::string filename;
        getchar();
        getline(std::cin, filename);
        if (filename.empty()) filename = "greedy_top10";
        DecisionTree tree;
        tree.Read("trees/" + filename);
        guesser = std::make_unique<GuesserDecisionTree>(tree);
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
