#include "htpp/lib.h"
#include "threadpool.h"
#include "connection.h"

#include <string>
#include <numeric>
#include <vector>
#include <span>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <functional>
#include <utility>
#include <map>
#include <asio.hpp>

#ifndef HTPP_VERSION
#define HTPP_VERSION "unversioned"
#endif

using asio::ip::tcp;
using namespace std::literals::string_view_literals;
using namespace htpp;

static auto ERROR_404 = Content{ContentType::TextHtml, "404 Not Found"};

Server& Server::set_routes(std::vector<WebPoint> routes) {
    this->routes = std::move(routes);
    return *this;
}


Server& Server::static_files(std::string directory, std::filesystem::path static_path) {
    this->static_path = std::move(static_path);
    static_dir = std::move(directory);
    return *this;
}

static Response read_file(const std::filesystem::path& path){
    std::string text;
    std::ifstream file{path, std::ios::ate | std::ios::binary};
    text.reserve(file.tellg());
    file.seekg(0, std::ios::beg);
    text.append(std::istreambuf_iterator{file}, {});
    return {200, text};
}

Response Server::fire_handler(const Request& request) const {
    if(request.url.starts_with(static_dir)){
        auto path = static_path / request.url.substr(static_dir.size());
        if(std::filesystem::is_directory(path))
            path = path / "index.html";
        if(std::filesystem::exists(path))
            return read_file(path);
    }

    auto it = std::ranges::find_if(routes, [&](const WebPoint& p){ return p.type == request.type && p.address == request.url; });
    if(it == routes.end())
        return {404, ERROR_404};
    return it->function(request.param);
}


void Server::run() const{
    asio::io_context context;
    tcp::acceptor accepter{context, tcp::endpoint{tcp::v4(), port}};
    ThreadPool pool{50};

    while(true){
        pool.queue_task([this, http = HttpConnection{accepter.accept()}] mutable {
            char read[1024*4096]; // Max request size set to 4MB
            try{
                do{
                    http.set_buffer(read);
                    http.receive();
                    Request request = http.parse_request();
                    http.receive_headers();
                    for(const auto& mid : this->middlewares)
                        mid->on_received(request);
                    auto response = this->fire_handler(request);
                    http.write_response(response);
                } while(http.connection_keepalive > std::time(nullptr));
            }
            catch(...){
                http.write_response(Response{500, std::nullopt});
            }
        });
    }
}