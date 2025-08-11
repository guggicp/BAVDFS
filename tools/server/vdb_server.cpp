#include <index/index_factory.h>
#include <logger/logger.h>
#include <spdlog/logger.h>
#include "httpserver/http_server.h"

int main()
{

    int dim = 1;

    init_global_logger();
    set_log_level(spdlog::level::debug);
    GlobalLogger->info("Global logger initialized!");
    IndexFactory *globalIndexFactory = getGlobalIndexFactory();
    globalIndexFactory->init(IndexFactory::IndexType::FLAT, dim);
    GlobalLogger->info("Global IndexFactory initualized!");
    HttpServer server("localhost", 7781);
    server.start();
    return 0;
}