#include <algorithm>
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

#include "players.h"

const int max_moves = 6;

int main() {
    ReadDicts();
    std::atomic<int> total = 0;
    std::vector<std::atomic<int>> stats(max_moves + 1);
    std::vector<std::thread> threads(std::thread::hardware_concurrency());
    auto &dict = GetDictAns();
    size_t block_length = (dict.size() + threads.size() - 1) / threads.size();
    for (size_t thread_id = 0; thread_id < threads.size(); ++thread_id) {
        size_t l = block_length * thread_id;
        size_t r = std::min(dict.size(), l + block_length);
        threads[thread_id] = std::thread([&, l, r] {
            for (size_t answer_id = l; answer_id < r; ++answer_id) {
                HostFixed host(answer_id);
                GuesserHeuristic guesser;
                int move;
                for (move = 1; move <= max_moves; ++move) {
                    std::string guess = guesser.MakeGuess();
                    std::string result = host.OnGuess(guess);
                    guesser.OnResult(guess, result);
                    if (std::count(result.begin(), result.end(), 'g') == result.size()) {
                        break;
                    }
                }
                stats[move - 1]++;
                double progress = double(total++) / dict.size();
                std::cout << "progress: " << (int) round(progress * 100) << "%" << std::endl;
                std::cout << "\x1b[1A";
            }
        });
    }
    for (auto &thread: threads) {
        thread.join();
    }
    std::cout << "heuristic guesser stats:" << std::endl;
    double avg = 0;
    for (int move = 1; move <= max_moves; ++move) {
        std::cout << move << ": " << stats[move - 1] << std::endl;
        avg += move * stats[move - 1];
    }
    avg /= (double) dict.size();
    std::cout << "lost: " << stats[max_moves] << std::endl;
    std::cout << "average: " << avg << std::endl;
}
