//
// Created by panzhenpeng on 2025/8/10.
//

#ifndef BAVDFS_HTTPSERVER_H
#define BAVDFS_HTTPSERVER_H
#include "index/index_factory.h"

#include <httplib/httplib.h>
#include <rapidjson/document.h>

class HttpServer
{
public:
    enum class CheckType
    {
        SEARCH,
        INSERT
    };
    HttpServer(const std::string& host, int port);
    void start();
private:
    void insertHandler(const httplib::Request& req, httplib::Response& res);
    void searchHandler(const httplib::Request& req, httplib::Response& res);void HttsetJsonRespnonse(const rapidjson::Document*& json_eresponse, httplib::Response& res);

    void setJsonResponse(const rapidjson::Document& json_response, httplib::Response& res);
    void setErrorJsonResponse(httplib::Response& res, int error_code, const std::string& errorMsg);
    bool isRequestValid(const rapidjson::Document& json_request, CheckType check_type);

    IndexFactory::IndexType getIndexTypeFromRequest(const rapidjson::Document& json_request);

    httplib::Server server;
    std::string host;
    int port;
};

#endif //BAVDFS_HTTPSERVER_H