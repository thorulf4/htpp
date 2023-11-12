#pragma once
#include <string>
#include <string_view>
#include <optional>
#include <utility>

namespace htpp{
    enum class RequestType{
        GET,
        HEAD,
        POST,
        PUT,
        DELETE,
        CONNECT,
        OPTIONS,
        TRACE,
        PATCH
    };

    enum class ContentType{
        TextHtml,
        TextJson
    };

    struct Content{
        ContentType type;
        std::string data;

        constexpr Content(ContentType type, std::string data): type{type}, data{std::move(data)} {}
        constexpr Content(std::string data): type{ContentType::TextHtml}, data{std::move(data)} {}
    };

    struct Response {
        int response_code{200};
        std::optional<Content> content{std::nullopt};
    };

    struct Request{
        RequestType type;
        std::string_view url;
        std::string_view param;
    };

    inline std::string_view to_str(RequestType type){
        using enum RequestType;
        switch (type)
        {
        case GET: return "GET";
        case PUT: return "PUT";
        case POST: return "POST";
        case DELETE: return "DELETE";
        case HEAD: return "HEAD";
        case OPTIONS: return "OPTIONS";
        case TRACE: return "TRACE";
        case CONNECT: return "CONNECT";
        case PATCH: return "OTHER";
        default: return "UNKNOWN";
        }
        std::unreachable();
    }
}