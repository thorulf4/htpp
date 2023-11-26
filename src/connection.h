#pragma once
#include <htpp/http.h>
#include "connection.h"
#include "utilites.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <asio.hpp>
#include <string_view>
#include <limits>
#include <concepts>

template<typename T>
concept Connection = requires (T t) {
    {t.init()} -> std::same_as<asio::awaitable<void>>;
    {t.receive(asio::mutable_buffer{})} -> std::same_as<asio::awaitable<std::size_t>>;
    {t.write(asio::const_buffer{})} -> std::same_as<asio::awaitable<void>>;
    {t.available()} -> std::same_as<std::size_t>;
    {t.get_executor()} -> std::same_as<asio::any_io_executor>;
    {t.is_open()} -> std::same_as<bool>;
    {t.close()} -> std::same_as<asio::awaitable<void>>;
};

template<Connection ConnectionType>
class HttpProtocol{
    static constexpr int keepalive_timeout = 30;

    char* it;
    char* end;
    size_t bytes_left;
public:
    ConnectionType connection;
    std::time_t connection_keepalive{std::time(nullptr) + keepalive_timeout};

    explicit HttpProtocol(ConnectionType connection): connection{std::move(connection)} {}
    HttpProtocol(const HttpProtocol&) = delete;
    HttpProtocol& operator=(const HttpProtocol&) = delete;
    HttpProtocol(HttpProtocol&&) noexcept = default;
    HttpProtocol& operator=(HttpProtocol&&) noexcept = default;

    template<size_t N>
    void set_buffer(char (& buffer)[N]){
        it = buffer;
        end = it;
        bytes_left = N;
    }

    asio::awaitable<void> init(){
        co_await connection.init();
    }

    asio::awaitable<void> receive(){
        while(connection.available() == 0){
            if(connection_keepalive < std::time(nullptr) || !connection.is_open()){
                throw std::logic_error{"Timed out"}; // Convert to return value error handling or custom exception
            }
            auto timer = asio::high_resolution_timer{connection.get_executor()};
            timer.expires_after(std::chrono::nanoseconds(10));
            co_await timer.async_wait(asio::use_awaitable);
        }
        auto count = co_await connection.receive(asio::buffer(end, bytes_left));
        end += count;
        bytes_left -= count;
    }

    asio::awaitable<void> wait_for_count(size_t bytes){
        while(static_cast<size_t>(std::distance(it, end)) < bytes)
            co_await receive();
    }

    asio::awaitable<void> find_first_of(std::string_view matches){
        while(it = std::find_first_of(it, end, matches.begin(), matches.end()), it == end)
            co_await receive();
    }

    asio::awaitable<void> find(char match){
        while(it = std::find(it, end, match), it == end)
            co_await receive();
    }

    asio::awaitable<htpp::Request> parse_request(){
        co_await wait_for_count(8);
        auto type = get_type(it);

        auto start = it;
        co_await find_first_of("? ");
        std::string_view url{start, it};
        std::string_view param;
        if(*it == '?'){ // request contains parameters parse those
            start = ++it;
            find(' ');
            param = {start, it};
        }

        constexpr std::string_view handshake{" HTTP/1.1\r\n"};
        co_await wait_for_count(handshake.size());
        if(!std::equal(handshake.begin(), handshake.end(), it))
            throw std::logic_error{"Wrong HTTP version"};
        it += handshake.size();

        co_return htpp::Request{type, url, param};
    }

    asio::awaitable<std::string_view> receive_header_value(){
        auto start = it;
        co_await find_first_of("\r\n");
        auto str = std::string_view{start, it};
        it += *it == '\r' ? 2 : 1;
        co_return str;
    };

    asio::awaitable<void> receive_headers(){
        while(true){
            auto key_start = it;
            co_await find_first_of(":\r\n");
            auto key = std::string_view{key_start, it};
            if(*it == ':'){
                it += 2; // Skip : and whitespace, not entirely robust
                if(key == "Connection"){
                    auto value = co_await receive_header_value();
                    if(value == "close"){
                        connection_keepalive = std::numeric_limits<std::time_t>::max();
                    }else{
                        connection_keepalive = std::time(nullptr) + keepalive_timeout;
                    }
                }
            }else{
                it += *it == '\r'? 2 : 1; // Skip past final endline
                co_return;
            }
        }
    }

    asio::awaitable<void> write_response(const htpp::Response& response){
        auto s = std::stringstream{};
        s << "HTTP/1.1 " << response.response_code << " \r\n";
        s << "Server: HTPP/" << HTPP_VERSION << "\r\n";
        auto timestamp = time(nullptr);
        tm utc; gmtime_r(&timestamp, &utc);
        s << "Date: " << weekday(utc) << ", " << Fmt2Int{utc.tm_mday} << ' ' << month(utc) << ' ' << (utc.tm_year + 1900) << ' ' << Fmt2Int{utc.tm_hour} << ':' << Fmt2Int{utc.tm_min} << ':' << Fmt2Int{utc.tm_sec} << " GMT\r\n";
        if(connection_keepalive < std::numeric_limits<std::time_t>::max()){
            s << "Connection: keep-alive\r\n";
            s << "Keep-Alive: timeout=" << keepalive_timeout << ", max=1000\r\n";
        }else{
            s << "Connection: close\r\n";
        }
        if(response.content){
            s << "Content-Type: " << to_str(response.content->type) << "\r\n";
            s << "Content-Length: " << response.content->data.size() << "\r\n\r\n";
            s << response.content.value().data;
        }else{
            s << "\r\n";
        }

        
        co_await connection.write(asio::buffer(s.view()));
    }

    asio::awaitable<void> close(){
        co_await connection.close();
    }
};
