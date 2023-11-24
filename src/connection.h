#pragma once
#include <htpp/http.h>
#include <asio.hpp>
#include <string_view>
#include <limits>

class HttpConnection{
    static constexpr int keepalive_timeout = 30;

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

    asio::awaitable<void> receive();
    asio::awaitable<void> wait_for_count(size_t bytes);
    asio::awaitable<void> find_first_of(std::string_view matches);
    asio::awaitable<void> find(char match);
    asio::awaitable<htpp::Request> parse_request();
    asio::awaitable<std::string_view> receive_header_value();
    asio::awaitable<void> receive_headers();
    asio::awaitable<void> write_response(const htpp::Response& response);
};