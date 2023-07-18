#pragma once

#include <algorithm>
#include <cmath>
#include <fstream>
#include <memory>
#include <sstream>
#include <vector>

#include "common.h"

struct Node {
    size_t guess_id = -1;
    std::vector<std::shared_ptr<Node>> go;

    Node() {
        go.resize(N_PATTERNS);
    }
};

class DecisionTree {
    std::shared_ptr<Node> root;

public:
    explicit DecisionTree(std::shared_ptr<Node> root = nullptr) : root(std::move(root)) {}

    [[nodiscard]] std::shared_ptr<Node> Get() const {
        return root;
    }

    void Read(const std::string &path) {
        std::ifstream fin(path);
        std::stringstream buf;
        buf << fin.rdbuf();
        root = Load(buf);
    }

    void Write(const std::string &path) {
        std::ofstream fout(path);
        std::stringstream buf;
        Dump(root, buf);
        fout << buf.str();
    }

private:
    static std::shared_ptr<Node> Load(std::stringstream &buf) {
        size_t value;
        buf.read(reinterpret_cast<char *>(&value), sizeof(value));
        if (value == -1) {
            return nullptr;
        }
        auto v = std::make_shared<Node>();
        v->guess_id = value;
        for (u_char pat = 0; pat < N_PATTERNS; ++pat) {
            v->go[pat] = Load(buf);
        }
        return v;
    }

    static void Dump(const std::shared_ptr<Node> &v, std::stringstream &buf) {
        if (!v) {
            size_t value = -1;
            buf.write(reinterpret_cast<const char *>(&value), sizeof(value));
            return;
        }
        buf.write(reinterpret_cast<const char *>(&v->guess_id), sizeof(v->guess_id));
        for (u_char pat = 0; pat < N_PATTERNS; ++pat) {
            Dump(v->go[pat], buf);
        }
    }
};

double ApproxScore(double entropy) {
    return sqrt(1 + entropy);
}

std::pair<std::shared_ptr<Node>, double> BruteForceSearch(
        const std::vector<size_t> &possible_answers, size_t cnt_top) {
    auto v = std::make_shared<Node>();
    if (possible_answers.size() == 1) {
        v->guess_id = answer2guess[possible_answers[0]];
        return {v, 1};
    }
    if (possible_answers.size() == 2) {
        v->guess_id = answer2guess[possible_answers[0]];
        u_char pat = GetPattern(v->guess_id, possible_answers[1]);
        v->go[pat] = std::make_shared<Node>();
        v->go[pat]->guess_id = answer2guess[possible_answers[1]];
        return {v, 1.5};
    }
    std::vector<std::pair<double, size_t>> options;
    options.reserve(guesses.size());
    for (size_t guess_id = 0; guess_id < guesses.size(); ++guess_id) {
        std::vector<size_t> cnt(N_PATTERNS);
        for (size_t answer_id: possible_answers) {
            cnt[GetPattern(guess_id, answer_id)]++;
        }
        double score = 0;
        for (u_char pat = 0; pat < N_PATTERNS; ++pat) {
            if (cnt[pat] != 0 && pat != WIN_PAT) {
                score += (double) cnt[pat] * ApproxScore(log2(cnt[pat]));
            }
        }
        score /= (double) possible_answers.size();
        options.emplace_back(score, guess_id);
    }
    std::sort(options.begin(), options.end());
    double min_score = 10;
    for (size_t i = 0; i < cnt_top; ++i) {
        size_t guess_id = options[i].second;
        std::vector<std::vector<size_t>> partition(N_PATTERNS);
        for (size_t answer_id: possible_answers) {
            if (guess_id != answer2guess[answer_id]) {
                partition[GetPattern(guess_id, answer_id)].push_back(answer_id);
            }
        }
        std::vector<u_char> ord(N_PATTERNS);
        for (u_char pat = 0; pat < N_PATTERNS; ++pat) {
            ord[pat] = pat;
        }
        std::sort(ord.begin(), ord.end(), [&](size_t pat1, size_t pat2) {
            return partition[pat1].size() > partition[pat2].size();
        });
        double lb = 0;
        for (u_char pat: ord) {
            if (!partition[pat].empty()) {
                lb += (2 * (double) partition[pat].size() - 1) / (double) possible_answers.size();
            }
        }
        std::vector<std::shared_ptr<Node>> go(N_PATTERNS);
        double cur_score = 1;
        for (u_char pat: ord) {
            if (partition[pat].empty()) break;
            if (cur_score + lb >= min_score) {
                cur_score += lb;
                break;
            }
            auto [u, score] = BruteForceSearch(partition[pat], cnt_top);
            go[pat] = u;
            cur_score += score * (double) partition[pat].size() / (double) possible_answers.size();
            lb -= (2 * (double) partition[pat].size() - 1) / (double) possible_answers.size();
        }
        if (cur_score < min_score) {
            min_score = cur_score;
            v->guess_id = guess_id;
            v->go = std::move(go);
        }
    }
    return {v, min_score};
}

DecisionTree DecisionTreeBruteForce(size_t cnt_top) {
    std::vector<size_t> possible_answers(answers.size());
    for (size_t answer_id = 0; answer_id < answers.size(); ++answer_id) {
        possible_answers[answer_id] = answer_id;
    }
    return DecisionTree(BruteForceSearch(possible_answers, cnt_top).first);
}
