//
// Created by panzhenpeng on 2025/8/10.
//

#ifndef BAVDFS_HTTPSERVER_H
#define BAVDFS_HTTPSERVER_H
#include "httplib/httplib.h"
#include "rapidjson/document.h"

class HttpServer
{
public:
    httplib::Server server;
    std::string host;
    int port;
    HttpServer(const std::string& host, int port);
    void insertHandler(const httplib::Request& req, httplib::Response& res);
    void searchHandler(const httplib::Request& req, httplib::Response& res);void HttsetJsonRespnonse(const rapidjson::Document*& json_eresponse, httplib::Response& res);
    void start();
};

#endif //BAVDFS_HTTPSERVER_H