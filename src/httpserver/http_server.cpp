//
// Created by panzhenpeng on 2025/8/10.
//
#include "httpserver/http_server.h"
#include "common/constants.h"
#include <faiss/Index.h>
#include <index/faiss_index.h>
#include <logger/logger.h>
#include <rapidjson/document.h>
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

static IndexFactory::IndexType getIndexTypeFromRequest(const rapidjson::Document& json_request)
{
    // TODO: find out the right params.
    return IndexFactory::IndexType::UNKNOWN;
}
void HttpServer::insertHandler(const httplib::Request& req, httplib::Response& res)
{
    GlobalLogger->debug("Received insert request");

    // analysis JSON request
    rapidjson::Document json_request;
    json_request.Parse(req.path.c_str());

    // print the input params. of user
    GlobalLogger->info("Insert request parameters: {}", req.body);

    // check the JSON Document is valid OBJ.
    // TODO: check the JSON Document is valid OBJ.
    // TODO: check the request legal or not

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
        GlobalLogger->error("Invalid indexType parameter in the request");
        res.status = 400;
        // setErrorJsonResponse(res, RESPONSE_RETCODE_ERROR, "Invalid indexType parameter in the request");
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
        // others
        default:
            break;
    }

    // set the response
    rapidjson::Document json_response;
    json_response.SetObject();
    rapidjson::Document::AllocatorType& allocator = json_response.GetAllocator();

    // add retCode to response
    json_response.AddMember(RESPONSE_RETCODE, RESPONSE_RETCODE_SUCCESS, allocator);
}

void HttpServer::searchHandler(const httplib::Request& req, httplib::Response& res)
{

}

void HttpServer::start() {
    
}