#include <htpp/response.h>
#include <htpp/lib.h>
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

constexpr static auto ERROR_404 = "404 Not Found";

class StringResponse : public Response{
    std::string_view content;
public:
    StringResponse(uint16_t code, std::string_view content): Response{code}, content{content} {}
    void print_content(std::stringstream& s) const { s << content; }
    ContentType content_type() const { return ContentType::TextPlain; }
    std::size_t content_size() const { return content.size(); }
};
static_assert(SizedContentConcept<StringResponse>);

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

// Fake const here, beware
class FileResponse : public OkResponse{
    ContentType type;
    std::size_t file_size;
    mutable std::ifstream file;
public:
    FileResponse(ContentType type, const std::filesystem::path& path): type{type}, file{path, std::ios::ate | std::ios::binary}{
        file_size = file.tellg();
        file.seekg(0, std::ios::beg);
    }
    ContentType content_type() const { return type; }
    std::size_t content_size() const { return file_size; }
    void print_content(std::stringstream& s) const {
        s << file.rdbuf();
        file.seekg(0, std::ios::beg); // Preserve fake constness
    }
};
static_assert( SizedContentConcept<FileResponse> );

template<typename T>
[[nodiscard]] asio::awaitable<void> fire_handler(const Server& server, const Request& request, HttpProtocol<T>& http) {
    if(request.url.starts_with(server.static_dir)){
        if(request.url.contains("..")){
            return http.send(StringResponse{404, ERROR_404});
        }
        auto path = server.static_path / request.url.substr(server.static_dir.size());
        
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
            return http.send(FileResponse{type, path});
        }
    }

    auto it = server.routes.find({request.type, request.url});
    if(it == server.routes.end()){
        return http.send(StringResponse{404, ERROR_404});
    }
    const WebPoint::Handler handler = it->second;
    return handler(http, request.param);
}

template<Connection ConnectionType>
void handle_connection(const Server& server, ConnectionType connection){
    asio::co_spawn(connection.get_executor(), [&, http = HttpProtocol<ConnectionType>{std::move(connection)}]() mutable -> asio::awaitable<void> {
        char read[1024*4096]; // Max request size set to 4MB
        try{
            co_await http.init();
            do{
                http.set_buffer(read);
                Request request = co_await http.parse_request();
                co_await http.receive_headers();
                for(const auto& mid : server.middlewares)
                    mid->on_received(request);
                co_await fire_handler(server, request, http);
            } while(http.connection_keepalive > std::time(nullptr));
        }
        catch(std::exception& e){
            // std::cout << e.what() << std::endl;
        }
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