// ef_searcher.h
#pragma once

#include <vector>
#include <string>
#include <functional>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <cassert>

class EFSearcher {
public:
    using QueryVector = std::vector<float>;
    using LabelType = int; // 可改为 size_t 或 DATA_TYPE

    // 用户需提供的搜索函数签名
    using SearchFunc = std::function<void(
        const float* query,
        int ef,
        std::vector<float>& out_distances,
        std::vector<LabelType>& out_labels
    )>;

    // 用户需提供的真值获取函数
    using GroundTruthFunc = std::function<std::vector<LabelType>(int query_id)>;

    struct Config {
        int knn = 10;
        float target_recall = 0.95f;
        int max_ef = 2000;
        int min_ef = 10;
        int ef_step_initial = 10;
        float ef_growth_factor = 1.2f;
        size_t max_queries = 100; // 用于测试的查询数
        bool verbose = true;
    };

    EFSearcher(
        const std::vector<QueryVector>& queries,
        const Config& config,
        const std::string& log_path
    ) : queries_(queries), config_(config), log_path_(log_path) {}

    void run(const SearchFunc& search_fn, const GroundTruthFunc& gt_fn);
    void set_Config(const Config& config);

private:
    float compute_recall(const std::vector<LabelType>& results, const std::vector<LabelType>& ground_truth);

    void init_log();

    void log_to_files(int ef, float recall, float time_us);
    // === 成员变量 ===
    const std::vector<QueryVector>& queries_;
    Config config_;
    std::string log_path_;
    std::ofstream log_file_;
    std::ofstream csv_file_;
};


