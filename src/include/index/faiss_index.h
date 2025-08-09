//
// Created by panzhenpeng on 2025/8/9.
//

#ifndef BAVDFS_FAISSINDEX_H
#define BAVDFS_FAISSINDEX_H
#include <cstdint>
#include <faiss/Index.h>
#include <vector>

class FaissIndex
{
public:
    FaissIndex(faiss::Index* index);
    void insert_vectors(const std::vector<float>& data, uint64_t label);
    std::pair<std::vector<long>, std::vector<float>> search_vectors(const std::vector<float>& query, int k);
private:
    faiss::Index* index;
};

#endif //BAVDFS_FAISSINDEX_H