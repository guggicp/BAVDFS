//
// Created by panzhenpeng on 2025/8/10.
//
#include "index/index_factory.h"
#include "index/faiss_index.h"
#include "index/hnswlib_index.h"
#include <faiss/Index.h>
#include <faiss/IndexIDMap.h>
#include <faiss/IndexFlat.h>

void IndexFactory::init(IndexType type, int dim, int num_data, MetricType metric)
{
    faiss::MetricType faiss_matric = ((metric == MetricType::L2) ? faiss::METRIC_L2 : faiss::METRIC_INNER_PRODUCT);
    switch (type)
    {
    case IndexType::FLAT:
        index_map[type] = new FaissIndex( new faiss::IndexIDMap(new faiss::IndexFlat(dim, faiss_matric)));
        break;
    case IndexType::HNSW:
        index_map[type] = new HNSWLibIndex(dim, num_data, metric, 16, 200);
    default:
        break;
    }
}

void *IndexFactory::getIndex(IndexType type) const
{
    auto it = index_map.find(type);
    if (it != index_map.end())
    {
        return it->second;
    }
    return nullptr;
}

namespace
{
    IndexFactory globalIndexFactory;
}

IndexFactory *getGlobalIndexFactory()
{
    return &globalIndexFactory;
}