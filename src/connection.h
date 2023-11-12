#pragma once
#include <htpp/http.h>
#include <asio.hpp>
#include <string_view>

class HttpConnection{
    asio::ip::tcp::socket socket;
    char* it;
    char* end;
    size_t bytes_left;
public:
    bool connection_keepalive{false};

    explicit HttpConnection(asio::ip::tcp::socket socket): socket{std::move(socket)} {}
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