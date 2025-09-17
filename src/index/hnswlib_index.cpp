//
// Created by panzhenpeng on 2025/9/16.
//
#include "index/hnswlib_index.h"

HNSWLibIndex::HNSWLibIndex(int dim, int num_data, IndexFactory::MetricType metric, int M, int ef_construction): dim(dim)
{
    // bool normalize = false;
    if (metric == IndexFactory::MetricType::L2)
    {
        space = new hnswlib::L2Space(dim);
    } else
    {
        throw std::runtime_error("Invalid meric type.");
    }
    index = new hnswlib::HierarchicalNSW<float>(space, num_data, M, ef_construction);
}

void HNSWLibIndex::insert_vectors(const std::vector<float>& data, uint64_t label)
{
    index->addPoint(data.data(), label);
}

std::pair<std::vector<long>, std::vector<float>> HNSWLibIndex::search_vectors(const std::vector<float>& query, int k, int ef_search)
{
    std::vector<long> indices(k, -1);
    std::vector<float> distances(k, -1);
    int j = 0;

    index->setEf(ef_search);
    auto result = index->searchKnn(query.data(), k);

    while (!result.empty())
    {
        auto item = result.top();  // attention! avoid cross-border issues
        indices[j] = item.second;
        distances[j] = item.first;
        result.pop();
        j++;
        if (j == k)
        {
            break;
        }
    }

    return {indices, distances};
}
