#pragma once
#include <htpp/response.h>
#include <string_view>
#include <vector>
#include <filesystem>
#include <string_view>
#include <memory>
#include <optional>
#include <thread>
#include <unordered_map>
#include <sstream>

namespace htpp{
    struct WebPoint : Endpoint{
        using Handler = asio::awaitable<void>(*)(Context&, std::string_view);
        Handler function;
    };


    class Middleware{
    public:
        virtual ~Middleware() = default;
        virtual void on_received(const Request& request) = 0;
    };

    struct SslConfig{
        std::string cert_path;
        std::string private_key;
    };


    class Server{
    public:
        uint16_t port;
        std::unordered_map<Endpoint, WebPoint::Handler> routes;
        std::string static_dir;
        std::filesystem::path static_path;
        uint32_t thread_count{std::thread::hardware_concurrency()};
        std::optional<SslConfig> ssl_config;
        
        Server(uint16_t port = 80): port{port} {}

        std::vector<std::unique_ptr<Middleware>> middlewares;

        Server& set_routes(std::vector<WebPoint> routes);
        Server& set_static_files(std::string directory, std::filesystem::path static_path);
        Server& set_threads(uint32_t count);
        Server& use_https(std::string key_path, std::string private_path);
        void run() const;

        template<typename T, typename ... Params>
        Server& add_middleware(Params&& ... params){
            middlewares.push_back(std::make_unique<T>(std::forward<Params>(params)...));
            return *this;
        }
    };

}