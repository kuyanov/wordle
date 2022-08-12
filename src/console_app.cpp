#include <iostream>
#include <memory>

#include "game.h"

int main() {
    auto dict = ReadDict("dictionary.txt");
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
    ConsoleGame game(dict, host.get(), guesser.get());
    game.Play(host_type != 3 && guesser_type != 1);
    Player winner = game.GetWinner();
    if (winner == Player::GUESSER) {
        std::cout << "guesser won in " << game.GetMove() << " moves" << std::endl;
    } else if (winner == Player::HOST) {
        std::cout << "host won, the answer was '" << game.GetAnswer() << "'" << std::endl;
    }
}
