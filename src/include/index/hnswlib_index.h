//
// Created by panzhenpeng on 2025/9/16.
//

#ifndef VECTORDB_HNSWLIB_INDEX_H
#define VECTORDB_HNSWLIB_INDEX_H

#include <hnswlib/hnswlib.h>

#include "index_factory.h"

// HNSWLibIndex: packaging the Interface of HNSW Algo. providing `create`, `control` and `usage` for HNSW Indexing.
class HNSWLibIndex
{
public:
    HNSWLibIndex(
        int dim, int num_data, IndexFactory::MetricType metric, int M=16, int ef_construction=200); // constructor
    void insert_vectors(const std::vector<float>& data, uint64_t label);  // write the vectors into database
    std::pair<std::vector<long>, std::vector<float>> search_vectors(
        const std::vector<float>& query, int k, int ef_search=50);  // find the similar/same vectors
private:
    int dim;
    hnswlib::SpaceInterface<float> *space;
    hnswlib::HierarchicalNSW<float> *index;
};

#endif //VECTORDB_HNSWLIB_INDEX_H