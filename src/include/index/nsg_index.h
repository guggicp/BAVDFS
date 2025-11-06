//
// Created by panzhenpeng on 2025/8/9.
//

#ifndef BAVDFS_FAISSINDEX_H
#define BAVDFS_FAISSINDEX_H
#include <cstdint>
#include <faiss/Index.h>
#include <faiss/impl/NSG.h>
#include <faiss/IndexNSG.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <sys/types.h>

using idx_t = int64_t;
class NsgIndex : public faiss::IndexNSG {
public:
    // 构造函数：指定维度、距离度量（如L2）、持久化路径
    NsgIndex(int d, faiss::MetricType metric = faiss::METRIC_L2, 
             const std::string& persist_path = "");
    ~NsgIndex() override;

    // 重写或新增核心方法
    void add_with_ids(idx_t n, const float* x, const idx_t* ids) override;  // 支持自定义ID
    void search(idx_t n, const float* x, idx_t k, float* distances, idx_t* labels, const faiss::SearchParameters* params = nullptr) const override;// 搜索
    bool delete_by_id(idx_t id);  // 新增：删除指定ID的向量（NSG原生不支持，需自定义逻辑）
    void persist();  // 持久化索引到磁盘
    bool load();     // 从磁盘加载索引

private:
    std::string persist_path_;  // 持久化路径
    std::unordered_map<idx_t, size_t> id_to_offset_;  // 自定义ID到内部索引的映射（NSG默认用连续整数）
    // 其他辅助变量：如已删除ID标记、元数据等
};

#endif //BAVDFS_FAISSINDEX_H