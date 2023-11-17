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

Server& Server::set_routes(std::vector<WebPoint> new_routes) {
    for(const WebPoint& route : new_routes)
        routes[route] = route.function;
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
        if(request.url.contains(".."))
            return {404, ERROR_404};
        auto path = static_path / request.url.substr(static_dir.size());
        if(std::filesystem::is_directory(path))
            path = path / "index.html";
        if(std::filesystem::exists(path))
            return read_file(path);
    }

    auto it = routes.find({request.type, request.url});
    if(it == routes.end())
        return {404, ERROR_404};
    const WebPoint::Handler handler = it->second;
    return handler(request.param);
}


void Server::run() const{
    asio::io_context context(std::thread::hardware_concurrency());
    // ThreadPool pool{50};
    
    asio::co_spawn(context, [&]() mutable -> asio::awaitable<void> {
        auto executor = co_await asio::this_coro::executor;
        tcp::acceptor accepter{executor, tcp::endpoint{tcp::v4(), port}};
        while(true){
            auto socket = co_await accepter.async_accept(asio::use_awaitable);
            asio::co_spawn(executor, [&, http = HttpConnection{std::move(socket)}]() mutable -> asio::awaitable<void> {
                char read[1024*4096]; // Max request size set to 4MB
                try{
                    do{
                        http.set_buffer(read);
                        co_await http.receive();
                        Request request = co_await http.parse_request();
                        co_await http.receive_headers();
                        for(const auto& mid : this->middlewares)
                            mid->on_received(request);
                        auto response = this->fire_handler(request);
                        co_await http.write_response(response);
                    } while(http.connection_keepalive > std::time(nullptr));
                    co_return;
                }
                catch(...){ }
                co_await http.write_response(Response{500, std::nullopt});
            }, asio::detached);
        }
    }, asio::detached);

    std::vector<std::jthread> threads;
    for(auto i = 1u; i < std::thread::hardware_concurrency(); i++)
        threads.emplace_back([&](){context.run();});

    context.run();
}