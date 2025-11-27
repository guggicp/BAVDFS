#include "ef_searcher.h"

void EFSearcher::log_to_files(int ef, float recall, float time_us) {
        log_file_ << "EF: " << ef 
                  << ", Recall: " << recall 
                  << ", Time: " << time_us << " us" << std::endl;
        csv_file_ << ef << "," << recall << "," << time_us << std::endl;
        log_file_.flush();
        csv_file_.flush();
    }

void EFSearcher::init_log() {
    log_file_.open(log_path_, std::ios::out);
    csv_file_.open(log_path_ + ".csv", std::ios::out);
    if (!log_file_ || !csv_file_) {
        throw std::runtime_error("Cannot open log files!");
    }
    csv_file_ << "ef,recall,time_us_per_query" << std::endl;
}

float EFSearcher::compute_recall(const std::vector<LabelType>& results, const std::vector<LabelType>& ground_truth) {
    assert(results.size() >= static_cast<size_t>(config_.knn));
    assert(ground_truth.size() == static_cast<size_t>(config_.knn));

    std::vector<LabelType> inter;
    std::set_intersection(
        results.begin(), results.begin() + config_.knn,
        ground_truth.begin(), ground_truth.end(),
        std::back_inserter(inter)
    );
    return static_cast<float>(inter.size()) / config_.knn;
}

void EFSearcher::run(const SearchFunc& search_fn, const GroundTruthFunc& gt_fn) {
    init_log();

    size_t nq = std::min(config_.max_queries, queries_.size());
    if (nq == 0) {
        throw std::runtime_error("No queries provided!");
    }

    int ef = config_.min_ef;
    while (ef <= config_.max_ef) {
        double total_recall = 0.0;
        double total_time_us = 0.0;

        for (size_t i = 0; i < nq; ++i) {
            const auto& query = queries_[i];
            auto gt = gt_fn(i);

            std::vector<float> distances;
            std::vector<LabelType> labels;
            
            auto start = std::chrono::high_resolution_clock::now();
            search_fn(query.data(), ef, distances, labels);
            auto end = std::chrono::high_resolution_clock::now();
            auto time_us = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1000.0;

            // 计算 recall@k
            float recall = compute_recall(labels, gt);
            total_recall += recall;
            total_time_us += time_us;
        }

        float avg_recall = static_cast<float>(total_recall / nq);
        float avg_time_us = static_cast<float>(total_time_us / nq);
        float qps = 1e6f / avg_time_us;

        log_to_files(ef, avg_recall, avg_time_us);

        if (config_.verbose) {
            std::cout << "EF=" << ef 
                        << " Recall=" << avg_recall 
                        << " Time=" << avg_time_us << "us"
                        << " QPS=" << qps << std::endl;
        }

        if (avg_recall >= config_.target_recall) {
            break;
        }

        // 动态调整 ef 步长（类似你的 BeamSizeGenerator）
        ef = static_cast<int>(ef * config_.ef_growth_factor);
        if (ef < config_.min_ef + 1) ef = config_.min_ef + 1;
    }
}
void EFSearcher::set_Config(const Config& config) {
        config_ = config;
    }

/* How to use */
// // 1. 定义你的搜索函数（适配 hnswlib）
// auto search_fn = [&](const float* query, int ef, std::vector<float>& distances, std::vector<DATA_TYPE>& labels) {
//     // 调用 hnswlib 的 searchKnn 或类似接口
//     auto result = hnsw_index->searchKnn(query, knn, ef);
//     // 填充 distances 和 labels
// };

// // 2. 定义真值获取（从 gt 数据中读取）
// auto get_ground_truth = [&](int query_id) -> std::vector<int> {
//     return ground_truth[query_id]; // 长度为 knn
// };

// // 3. 创建 searcher
// EFSearcher searcher(
//     queries,                // 所有查询向量 (nq x dim)
//     knn,                    // top-k
//     target_recall,          // 目标 recall，如 0.95
//     "my_experiment.log"     // 日志路径
// );

// // 4. 执行搜索
// searcher.run(search_fn, get_ground_truth);