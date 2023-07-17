#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "players.h"

const int max_moves = 6;

int main() {
    ReadWords();
    std::cout << "use true wordle list (0/1): ";
    bool use_twl;
    std::cin >> use_twl;
    std::cout << "use frequency priors (0/1): ";
    bool use_priors;
    std::cin >> use_priors;
    std::cout << "test on true wordle list (0/1): ";
    bool test_on_twl;
    std::cin >> test_on_twl;

    std::vector<int> stats(max_moves + 1);
    std::vector<std::pair<double, int>> scores;
    size_t cnt_games = test_on_twl ? twl.size() : all.size();
    for (size_t answer_id = 0; answer_id < cnt_games; ++answer_id) {
        HostFixed host(answer_id, test_on_twl);
        GuesserHeuristic guesser(use_twl, use_priors);
        std::vector<std::shared_ptr<Node>> tree_path;
        int cnt_moves = max_moves + 1;
        for (int move = 1; move <= max_moves; ++move) {
            tree_path.push_back(guesser.CurrentNode());
            size_t guess_id = guesser.MakeGuess();
            u_char pat = host.OnGuess(guess_id);
            guesser.OnResult(guess_id, pat);
            if (IsAllGreen(pat)) {
                cnt_moves = move;
                break;
            }
        }
        stats[cnt_moves - 1]++;
        for (size_t i = 0; i < tree_path.size(); ++i) {
            scores.emplace_back(tree_path[i]->entropy, cnt_moves - i);
        }
    }
    std::ofstream fout("data/entropy_scores.txt");
    for (const auto &[entropy, score]: scores) {
        fout << entropy << ' ' << score << '\n';
    }
    double avg = 0;
    for (int move = 1; move <= max_moves; ++move) {
        std::cout << move << ": " << stats[move - 1] << std::endl;
        avg += move * stats[move - 1];
    }
    avg /= (double) cnt_games;
    std::cout << "lost: " << stats[max_moves] << std::endl;
    std::cout << "average: " << avg << std::endl;
}
