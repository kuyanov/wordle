#include <filesystem>
#include <iostream>
#include <vector>

#include "players.h"

const int MAX_MOVES = 6;

int main() {
    ReadWords();
    ComputePatterns();
    std::cout << "number of top guesses to consider: ";
    size_t k;
    std::cin >> k;
    DecisionTree tree = DecisionTreeGreedy(k);
    std::vector<int> stats(MAX_MOVES);
    int cnt_lost = 0;
    for (size_t answer_id = 0; answer_id < answers.size(); ++answer_id) {
        HostFixed host(answer_id);
        GuesserDecisionTree guesser(tree);
        bool lost = true;
        for (int i = 0; i < MAX_MOVES; ++i) {
            size_t guess_id = guesser.MakeGuess();
            u_char pat = host.OnGuess(guess_id);
            guesser.OnResult(pat);
            if (pat == WIN_PAT) {
                stats[i]++;
                lost = false;
                break;
            }
        }
        if (lost) {
            cnt_lost++;
        }
    }
    double mean = 0;
    for (int i = 0; i < MAX_MOVES; ++i) {
        std::cout << i + 1 << ": " << stats[i] << std::endl;
        mean += (i + 1) * stats[i];
    }
    mean += (MAX_MOVES + 1) * cnt_lost;
    mean /= (double) answers.size();
    std::cout << "lost: " << cnt_lost << std::endl;
    std::cout << "mean: " << mean << std::endl;

    std::filesystem::create_directory("trees");
    tree.Write("trees/greedy_top" + std::to_string(k));
}
