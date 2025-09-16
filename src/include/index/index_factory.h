//
// Created by panzhenpeng on 2025/8/10.
//

#ifndef BAVDFS_INDEXFACTORY_H
#define BAVDFS_INDEXFACTORY_H
#include <map>
#include <faiss/MetricType.h>
class IndexFactory
{
public:
    enum class IndexType
    {
        FLAT,
        HNSW,
        UNKNOWN=-1,
    };
    enum class MetricType
    {
        L2,
        IP
    };
    void init(IndexType type, int dim, int num_data = 0, MetricType metric = MetricType::L2);
    void *getIndex(IndexType type) const;
private:
    std::map<IndexType, void*> index_map;
};

IndexFactory *getGlobalIndexFactory();

#endif //BAVDFS_INDEXFACTORY_H