#pragma once
#include <htpp/http.h>
#include <string_view>
#include <vector>
#include <filesystem>
#include <string_view>
#include <memory>
#include <optional>
#include <unordered_map>

namespace htpp{
    struct WebPoint : Endpoint{
        using Handler = Response(*)(std::string_view);
        Handler function;
    };


    class Middleware{
    public:
        virtual ~Middleware() = default;
        virtual void on_received(const Request& request) = 0;
    };


    class Server{
        uint16_t port;
        std::unordered_map<Endpoint, WebPoint::Handler> routes;
        std::string static_dir;
        std::filesystem::path static_path;
        std::vector<std::unique_ptr<Middleware>> middlewares;
        
        Response fire_handler(const Request&) const;
    public:
        Server(uint16_t port = 80): port{port} {}

        Server& set_routes(std::vector<WebPoint> routes);
        Server& static_files(std::string directory, std::filesystem::path static_path);
        void run() const;

        template<typename T, typename ... Params>
        Server& add_middleware(Params&& ... params){
            middlewares.push_back(std::make_unique<T>(std::forward<Params>(params)...));
            return *this;
        }
    };

}