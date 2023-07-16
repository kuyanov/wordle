#pragma once

#include <algorithm>
#include <memory>
#include <vector>

#include "tools.h"

const u_char N_PATTERNS = 243;

struct Node {
    double entropy = 0;
    size_t guess_id = -1;
    std::vector<std::unique_ptr<Node>> go;
};

std::unique_ptr<Node> root;

inline double GetApproxScore(double entropy) {
    return sqrt(1 + entropy);
}

inline void BuildDecisionTree(Node *v, const std::vector<size_t> &possible_words, bool use_priors) {
    double min_score = 10;
    if (!use_priors) {
        v->entropy = log2(possible_words.size());
        std::vector<size_t> cnt(N_PATTERNS);
        for (size_t guess_id = 0; guess_id < all.size(); ++guess_id) {
            std::fill(cnt.begin(), cnt.end(), 0);
            for (size_t answer_id: possible_words) {
                cnt[pattern_mat[guess_id][answer_id]]++;
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
                sum[pattern_mat[guess_id][answer_id]] += priors[answer_id];
            }
            std::fill(entropies.begin(), entropies.end(), 0);
            for (size_t answer_id: possible_words) {
                u_char pat = pattern_mat[guess_id][answer_id];
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
            partition[pattern_mat[v->guess_id][answer_id]].push_back(answer_id);
        }
    }
    v->go.resize(partition.size());
    for (size_t i = 0; i < partition.size(); ++i) {
        if (!partition[i].empty()) {
            v->go[i] = std::make_unique<Node>();
            BuildDecisionTree(v->go[i].get(), partition[i], use_priors);
        }
    }
}
