#pragma once

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "tools.h"

const u_char N_PATTERNS = 243;

struct Node {
    size_t guess_id = -1;
    double entropy = 0;
    std::vector<std::shared_ptr<Node>> go;

    Node() {
        go.resize(N_PATTERNS);
    }
};

inline void DumpDecisionTree(const std::shared_ptr<Node> &v, std::stringstream &buf) {
    if (!v) {
        size_t value = -1;
        buf.write(reinterpret_cast<const char *>(&value), sizeof(value));
        return;
    }
    buf.write(reinterpret_cast<const char *>(&v->guess_id), sizeof(v->guess_id));
    buf.write(reinterpret_cast<const char *>(&v->entropy), sizeof(v->entropy));
    for (u_char pat = 0; pat < N_PATTERNS; ++pat) {
        DumpDecisionTree(v->go[pat], buf);
    }
}

inline void LoadDecisionTree(std::shared_ptr<Node> &v, std::stringstream &buf) {
    size_t value;
    buf.read(reinterpret_cast<char *>(&value), sizeof(value));
    if (value == -1) {
        return;
    }
    v = std::make_shared<Node>();
    v->guess_id = value;
    buf.read(reinterpret_cast<char *>(&v->entropy), sizeof(v->entropy));
    for (u_char pat = 0; pat < N_PATTERNS; ++pat) {
        LoadDecisionTree(v->go[pat], buf);
    }
}

static std::unordered_map<std::string, std::shared_ptr<Node>> tree_registry;

inline std::shared_ptr<Node> GetDecisionTree(const std::string &key) {
    auto &root = tree_registry[key];
    if (root) {
        return root;
    }
    auto path = std::filesystem::path("trees") / key;
    if (std::filesystem::exists(path)) {
        std::cout << "reading " << path.string() << std::endl;
        std::ifstream fin(path.string());
        std::stringstream buf;
        buf << fin.rdbuf();
        LoadDecisionTree(root, buf);
    }
    return root;
}

inline void SaveDecisionTree(const std::string &key, const std::shared_ptr<Node> &root) {
    tree_registry[key] = root;
    auto path = std::filesystem::path("trees") / key;
    if (!std::filesystem::exists(path)) {
        std::cout << "saving to " << path.string() << std::endl;
        std::filesystem::create_directories(path.parent_path());
        std::ofstream fout(path.string());
        std::stringstream buf;
        DumpDecisionTree(root, buf);
        fout << buf.str();
    }
}

inline double GetApproxScore(double entropy) {
    return sqrt(1 + entropy);
}

inline void BuildDecisionTreeHeuristic(std::shared_ptr<Node> &v, const std::vector<size_t> &possible_words,
                                       bool use_priors) {
    v = std::make_shared<Node>();
    double min_score = 10;
    if (!use_priors) {
        v->entropy = log2(possible_words.size());
        std::vector<size_t> cnt(N_PATTERNS);
        for (size_t guess_id = 0; guess_id < all.size(); ++guess_id) {
            std::fill(cnt.begin(), cnt.end(), 0);
            for (size_t answer_id: possible_words) {
                cnt[GetPattern(guess_id, answer_id)]++;
            }
            double score = 0;
            for (u_char pat = 0; pat < N_PATTERNS; ++pat) {
                if (cnt[pat] != 0 && !IsAllGreen(pat)) {
                    score += (double) cnt[pat] * GetApproxScore(log2(cnt[pat]));
                }
            }
            score /= (double) possible_words.size();
            if (score < min_score) {
                min_score = score;
                v->guess_id = guess_id;
            }
        }
    } else {
        double total_sum = 0;
        for (size_t answer_id: possible_words) {
            total_sum += priors[answer_id];
        }
        for (size_t answer_id: possible_words) {
            double p = priors[answer_id] / total_sum;
            v->entropy += p * -log2(p);
        }
        std::vector<double> sum(N_PATTERNS), entropies(N_PATTERNS);
        for (size_t guess_id = 0; guess_id < all.size(); ++guess_id) {
            std::fill(sum.begin(), sum.end(), 0);
            for (size_t answer_id: possible_words) {
                sum[GetPattern(guess_id, answer_id)] += priors[answer_id];
            }
            std::fill(entropies.begin(), entropies.end(), 0);
            for (size_t answer_id: possible_words) {
                u_char pat = GetPattern(guess_id, answer_id);
                double p = priors[answer_id] / sum[pat];
                entropies[pat] += p * -log2(p);
            }
            total_sum = 0;
            for (double p: sum) {
                total_sum += p;
            }
            double expected_score = 0;
            for (u_char pat = 0; pat < N_PATTERNS; ++pat) {
                if (sum[pat] != 0 && !IsAllGreen(pat)) {
                    expected_score += GetApproxScore(entropies[pat]) * sum[pat];
                }
            }
            expected_score /= total_sum;
            if (expected_score < min_score) {
                min_score = expected_score;
                v->guess_id = guess_id;
            }
        }
    }
    std::vector<std::vector<size_t>> partition(N_PATTERNS);
    for (size_t answer_id: possible_words) {
        if (answer_id != v->guess_id) {
            partition[GetPattern(v->guess_id, answer_id)].push_back(answer_id);
        }
    }
    for (size_t i = 0; i < partition.size(); ++i) {
        if (!partition[i].empty()) {
            BuildDecisionTreeHeuristic(v->go[i], partition[i], use_priors);
        }
    }
}
