#include "htpp/lib.h"
#include "connection.h"
#include "simple_connection.h"
#include "ssl_connection.h"

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

Server& Server::set_threads(uint32_t count) {
    thread_count = count;
    return *this;
}

Server& Server::set_static_files(std::string directory, std::filesystem::path static_path) {
    this->static_path = std::move(static_path);
    static_dir = std::move(directory);
    return *this;
}

Server& Server::use_https(std::string key_path, std::string private_path) {
    ssl_config = SslConfig{std::move(key_path), std::move(private_path)};
    return *this;
}

static std::string read_file(const std::filesystem::path& path){
    std::string text;
    std::ifstream file{path, std::ios::ate | std::ios::binary};
    text.reserve(file.tellg());
    file.seekg(0, std::ios::beg);
    text.append(std::istreambuf_iterator{file}, {});
    return text;
}

Response Server::fire_handler(const Request& request) const {
    if(request.url.starts_with(static_dir)){
        if(request.url.contains(".."))
            return {404, ERROR_404};
        auto path = static_path / request.url.substr(static_dir.size());
        
        ContentType type; // Default for safety
        if(std::filesystem::is_directory(path)){
            path = path / "index.html";
            type = ContentType::TextHtml;
        } else {
            std::size_t pos = request.url.find_last_of('.');
            if(pos == std::string::npos)
                type = ContentType::ApplicationOctetStream;
            else
                type = from_file_extension(request.url.substr(pos));
        }
            
        if(std::filesystem::exists(path)){
            return Content{type, read_file(path)};
        }
    }

    auto it = routes.find({request.type, request.url});
    if(it == routes.end())
        return {404, ERROR_404};
    const WebPoint::Handler handler = it->second;
    return handler(request.param);
}

template<Connection ConnectionType>
void handle_connection(const Server& server, ConnectionType connection){
    asio::co_spawn(connection.get_executor(), [&, http = HttpProtocol<ConnectionType>{std::move(connection)}]() mutable -> asio::awaitable<void> {
        char read[1024*4096]; // Max request size set to 4MB
        try{
            co_await http.init();
            do{
                http.set_buffer(read);
                co_await http.receive();
                Request request = co_await http.parse_request();
                co_await http.receive_headers();
                for(const auto& mid : server.middlewares)
                    mid->on_received(request);
                auto response = server.fire_handler(request);
                co_await http.write_response(response);
            } while(http.connection_keepalive > std::time(nullptr));
            co_await http.close();
            co_return;
        }
        catch(...){}
        co_await http.write_response(Response{500});
        co_await http.close();
    }, asio::detached);
}

void Server::run() const{
    asio::io_context context(thread_count);

    asio::co_spawn(context, [&]() mutable -> asio::awaitable<void> {
        tcp::acceptor accepter{context, tcp::endpoint{tcp::v4(), port}};
        while(true){
            auto socket = co_await accepter.async_accept(asio::use_awaitable);
            handle_connection(*this, SimpleConnection{std::move(socket)});
        }
    }, asio::detached);

    
    asio::ssl::context ssl_ctx{asio::ssl::context::tls_server};
    if(ssl_config.has_value()){
        ssl_ctx.use_certificate_file(ssl_config->cert_path, asio::ssl::context_base::pem);
        ssl_ctx.use_private_key_file(ssl_config->private_key, asio::ssl::context_base::pem);
        ssl_ctx.set_verify_mode(asio::ssl::verify_none);

        asio::co_spawn(context, [&]() mutable -> asio::awaitable<void> {
            tcp::acceptor accepter{context, tcp::endpoint{tcp::v4(), 443}};
            while(true){
                auto socket = co_await accepter.async_accept(asio::use_awaitable);
                handle_connection(*this, SslConnection{std::move(socket), ssl_ctx});
            }
        }, asio::detached);
    }

    std::vector<std::jthread> threads;
    for(auto i = 1u; i < thread_count; i++)
        threads.emplace_back([&](){context.run();});

    context.run();
}