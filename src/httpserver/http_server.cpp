//
// Created by panzhenpeng on 2025/8/10.
//
#include "httpserver/http_server.h"
#include "common/constants.h"
#include <faiss/Index.h>
#include <index/faiss_index.h>
#include <logger/logger.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "index/hnswlib_index.h"
#include "index/index_factory.h"

HttpServer::HttpServer(const std::string& host, int port) : host(host), port(port)
{
    server.Post("/search", [this](const httplib::Request& req, httplib::Response& res)
    {
        searchHandler(req, res);
    });

    server.Post("/insert", [this](const httplib::Request& req, httplib::Response& res)
    {
        insertHandler(req, res);
    });
};

void HttpServer::start()
{
    server.listen(host.c_str(), port);
}

bool HttpServer::isRequestValid(const rapidjson::Document& json_request, CheckType check_type)
{
    switch (check_type)
    {
    case CheckType::SEARCH:
        return json_request.HasMember(REQUEST_VECTORS) &&
            json_request.HasMember(REQUEST_K) &&
                (!json_request.HasMember(REQUEST_INDEX_TYPE) ||
                    json_request[REQUEST_INDEX_TYPE].IsString());
    case CheckType::INSERT:
        return json_request.HasMember(REQUEST_VECTORS) &&
            json_request.HasMember(REQUEST_ID) &&
                (!json_request.HasMember(REQUEST_INDEX_TYPE) ||
                    json_request[REQUEST_INDEX_TYPE].IsString());
    default:
        return false;
    }
}

IndexFactory::IndexType HttpServer::getIndexTypeFromRequest(const rapidjson::Document& json_request)
{
    // TODO: learn the way to find out the right params.
    if (json_request.HasMember(REQUEST_INDEX_TYPE))
    {
        std::string index_type_str = json_request[REQUEST_INDEX_TYPE].GetString();
        if (index_type_str == "FLAT")
        {
            return IndexFactory::IndexType::FLAT;
        }
        else if (index_type_str == "HNSW")
        {
            // add support for HNSW
            return IndexFactory::IndexType::HNSW;
        }
    }
    return IndexFactory::IndexType::UNKNOWN;
}

void HttpServer::insertHandler(const httplib::Request& req, httplib::Response& res)
{
    GlobalLogger->debug("Received insert request");

    // analysis JSON request
    rapidjson::Document json_request;
    json_request.Parse(req.body.c_str());

    // print the input params. of user
    GlobalLogger->info("Insert request parameters: {}", req.body);

    // check the JSON Document is valid OBJ.
    // TODO: learn to check the JSON Document is valid OBJ.
    if (!json_request.IsObject())
    {
        auto tmp_info = "Invalid JSON request";
        GlobalLogger->error(tmp_info);
        res.status = 400;
        setErrorJsonResponse(res, RESPONSE_RETCODE_ERROR, tmp_info);
        return ;
    }
    // TODO: learn to check the request legal or not
    if (!isRequestValid(json_request, CheckType::INSERT))
    {
        auto tmp_info = "Missing vectors or id parameter in the request";
        GlobalLogger->error(tmp_info);
        res.status = 400;
        setErrorJsonResponse(res, RESPONSE_RETCODE_ERROR, tmp_info);
        return ;
    }

    // get the insert params.
    std::vector<float> data;
    for (const auto& d: json_request[REQUEST_VECTORS].GetArray())
    {
        data.push_back(d.GetFloat());
    }
    uint64_t label = json_request[REQUEST_ID].GetUint64();

    GlobalLogger->debug("Insert parameters: label = {}", label);

    //get the index type of request params
    IndexFactory::IndexType indexType = getIndexTypeFromRequest(json_request);
    // if the Index Type is UNKNOWN, return the 400 error
    if (indexType == IndexFactory::IndexType::UNKNOWN)
    {
        auto tmp_info = "Invalid indexType parameter in the request";
        GlobalLogger->error(tmp_info);
        res.status = 400;
        setErrorJsonResponse(res, RESPONSE_RETCODE_ERROR, tmp_info);
        return ;
    }

    // using the global IndexFactory
    void *index = getGlobalIndexFactory()->getIndex(indexType);
    // according to the Index Type, initialize the Object and call the `insert_vectors` fsiwwitch (i
    switch (indexType) {
        case IndexFactory::IndexType::FLAT: {
            FaissIndex *faissIndex = static_cast<FaissIndex *>(index);
            faissIndex->insert_vectors(data, label);
            break;
        }
    case IndexFactory::IndexType::HNSW:
            {
                HNSWLibIndex *hnswIndex = static_cast<HNSWLibIndex*>(index);
                hnswIndex->search_vectors(data, label);
                break;
            }
        default:
            break;
    }

    // set the response
    rapidjson::Document json_response;
    json_response.SetObject();
    rapidjson::Document::AllocatorType& allocator = json_response.GetAllocator();

    // add retCode to response
    json_response.AddMember(RESPONSE_RETCODE, RESPONSE_RETCODE_SUCCESS, allocator);
    setJsonResponse(json_response, res);
}

void HttpServer::searchHandler(const httplib::Request& req, httplib::Response& res)
{
    GlobalLogger->debug("Received search request");

    // analysis JSON Request
    rapidjson::Document json_request;
    json_request.Parse(req.body.c_str());

    // print the user's input params.
    GlobalLogger->info("Search request parameters: {}", req.body);

    // check if the json document is a valid obj.
    if (!json_request.IsObject())
    {
        GlobalLogger->error("Invalid JSON request");
        res.status = 400;
        setErrorJsonResponse(res, RESPONSE_RETCODE_ERROR, "Invalid JSON request");
        return ;
    }
    // check if the request valid
    if (!isRequestValid(json_request, CheckType::SEARCH))
    {
        GlobalLogger->error("Missing vectors or k parameter in the request");
        res.status = 400;
        setErrorJsonResponse(res, RESPONSE_RETCODE_ERROR, "Missing vectors or k parameter in the request");
        return ;
    }

    // get the query params.
    std::vector<float> query;
    for (const auto& q: json_request[REQUEST_VECTORS].GetArray())
    {
        query.push_back(q.GetFloat());
    }
    int k = json_request[REQUEST_K].GetInt();

    GlobalLogger->debug("Query parameters: k = {}", k);

    // get the index type in request params.
    IndexFactory::IndexType indexType = getIndexTypeFromRequest(json_request);

    // if the index type is  `UNKNOWN`, then, return 400 ERR_CODE
    if (indexType == IndexFactory::IndexType::UNKNOWN)
    {
        GlobalLogger->error("Invalid indexType parameter in the request");
        res.status = 400;
        setErrorJsonResponse(res, RESPONSE_RETCODE_ERROR, "Invalid indexType parameter in the request");
        return ;
    }

    // using the Global IndexFactory to get the Index OBJ.
    void *index = getGlobalIndexFactory()->getIndex(indexType);

    // according to the indexType to initialize the Index OBJ., and, call the `search_vector` function
    std::pair<std::vector<long>, std::vector<float>> results; // directly, declear the `results`
    switch (indexType)
    {
    case IndexFactory::IndexType::FLAT:
        {
            FaissIndex *faissIndex = static_cast<FaissIndex *>(index);
            results = faissIndex->search_vectors(query, k);
            break;
        }
        // add the dealing logi. of others
    default:
        break;
    }

    // transfer results to JSON
    rapidjson::Document json_response;
    json_response.SetObject();
    rapidjson::Document::AllocatorType& allocator = json_response.GetAllocator();

    // check if there is any valid search result
    bool valid_results = false;
    rapidjson::Value vectors(rapidjson::kArrayType);
    rapidjson::Value distances(rapidjson::kArrayType);
    for (size_t i = 0; i < results.first.size(); ++i)
    {
        if (results.first[i] != -1)
        {
            valid_results = true;
            vectors.PushBack(results.first[i], allocator);
            distances.PushBack(results.second[i], allocator);
        }
    }

    if (valid_results)
    {
        json_response.AddMember(RESPONSE_VECTORS, vectors, allocator);
        json_response.AddMember(RESPONSE_DISTANCES, distances, allocator);
    }

    // set response
    json_response.AddMember(RESPONSE_RETCODE, RESPONSE_RETCODE_SUCCESS, allocator);
    setJsonResponse(json_response, res);
}

void HttpServer::setJsonResponse(const rapidjson::Document& json_response, httplib::Response& res)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    json_response.Accept(writer);
    res.set_content(buffer.GetString(), RESPONSE_CONTENT_TYPE_JSON);
}

void HttpServer::setErrorJsonResponse(httplib::Response& res, int error_code, const std::string& errorMsg)
{
    rapidjson::Document json_response;
    json_response.SetObject();
    rapidjson::Document::AllocatorType& allocator = json_response.GetAllocator();
    json_response.AddMember(RESPONSE_RETCODE, error_code, allocator);
    json_response.AddMember(RESPONSE_ERROR_MSG, rapidjson::StringRef(errorMsg.c_str()), allocator);
    setJsonResponse(json_response, res);
}
