#include "index/nsg_index.h"
#include <faiss/IndexNSG.h>
#include <faiss/impl/NSG.h>

// nsg_index.cpp
void NsgIndex::add_with_ids(idx_t n, const float* x, const idx_t* ids) {
    // 1. 记录自定义ID与内部索引的映射（内部索引仍用0,1,2...）
    for (idx_t i = 0; i < n; ++i) {
        id_to_offset_[ids[i]] = this->ntotal + i;  // ntotal是父类Index的成员，记录当前向量总数
    }
    // 2. 调用父类方法添加向量（父类会自动维护ntotal）
    faiss::IndexNSG::add(n, x);
}

void NsgIndex::search(idx_t n, const float* x, idx_t k, float* distances, idx_t* labels, const faiss::SearchParameters* params) const {
    // 1. 调用父类搜索，得到内部索引
    faiss::IndexNSG::search(n, x, k, distances, labels);
    // 2. 将内部索引映射回自定义ID（需先构建反向映射，或遍历id_to_offset_）
    // 注意：需处理未找到的情况（labels可能为-1）
    for (idx_t i = 0; i < n * k; ++i) {
        if (labels[i] == -1) continue;
        // 这里需要一个反向映射表（offset_to_id），可在add时同步维护
        labels[i] = offset_to_id_.at(labels[i]);  // 需提前定义offset_to_id_
    }
}

void NsgIndex::persist() {
    // 1. 持久化 Faiss 原生索引数据
    faiss::write_index(this, (persist_path_ + "/nsg_core.index").c_str());
    // 2. 持久化自定义数据（ID映射、删除标记等）
    // 示例：用二进制文件存储id_to_offset_和deleted_ids_
    std::ofstream f(persist_path_ + "/meta.data", std::ios::binary);
    // ... 序列化逻辑 ...
}

bool NsgIndex::load() {
    // 1. 加载 Faiss 原生索引
    faiss::Index* idx = faiss::read_index((persist_path_ + "/nsg_core.index").c_str());
    // 强制转换为 IndexNSG（需确保类型匹配）
    auto* nsg_idx = dynamic_cast<faiss::IndexNSG*>(idx);
    if (!nsg_idx) return false;
    // 复制父类成员（需注意 Faiss 内部状态的完整性）
    this->d = nsg_idx->d;
    this->metric_type = nsg_idx->metric_type;
    this->ntotal = nsg_idx->ntotal;
    this->nsg = std::move(nsg_idx->nsg);  // 假设nsg是可移动的（需看Faiss源码）
    // 2. 加载自定义元数据
    std::ifstream f(persist_path_ + "/meta.data", std::ios::binary);
    // ... 反序列化逻辑 ...
    return true;
}