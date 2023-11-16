#pragma once
#include <htpp/http.h>
#include <asio.hpp>
#include <string_view>
#include <limits>

class HttpConnection{
    static constexpr int keepalive_timeout = 1;

    char* it;
    char* end;
    size_t bytes_left;
public:
    asio::ip::tcp::socket socket;
    std::time_t connection_keepalive{std::time(nullptr) + keepalive_timeout};

    explicit HttpConnection(asio::ip::tcp::socket socket): socket{std::move(socket)} {}
    ~HttpConnection();
    HttpConnection(const HttpConnection&) = delete;
    HttpConnection& operator=(const HttpConnection&) = delete;
    HttpConnection(HttpConnection&&) noexcept = default;
    HttpConnection& operator=(HttpConnection&&) noexcept = default;

    template<size_t N>
    void set_buffer(char (& buffer)[N]){
        it = buffer;
        end = it;
        bytes_left = N;
    }

    void receive();
    void wait_for_count(size_t bytes);
    void find_first_of(std::string_view matches);
    void find(char match);
    htpp::Request parse_request();
    std::string_view receive_header_value();
    void receive_headers();
    void write_response(const htpp::Response& response);
};