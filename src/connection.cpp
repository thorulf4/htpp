#include "connection.h"
#include "utilites.h"

#include <filesystem>
#include <fstream>
#include <iostream>

using asio::ip::tcp;
using namespace std::literals::string_view_literals;
using namespace htpp;

void HttpConnection::receive(){
    while(socket.available() == 0){
        if(connection_keepalive < std::time(nullptr)){
            std::cout << "Timed out\n";
            throw std::logic_error{"Timed out"};
        }
    }

    auto count = socket.receive(asio::buffer(end, bytes_left));
    end += count;
    bytes_left -= count;
}

void HttpConnection::wait_for_count(size_t bytes){
    while(static_cast<size_t>(std::distance(it, end)) < bytes)
        receive();
}

void HttpConnection::find_first_of(std::string_view matches){
    while(it = std::find_first_of(it, end, matches.begin(), matches.end()), it == end)
        receive();
}

void HttpConnection::find(char match){
    while(it = std::find(it, end, match), it == end)
        receive();
}

htpp::Request HttpConnection::parse_request(){
    wait_for_count(8);
    auto type = get_type(it);

    auto start = it;
    find_first_of("? "sv);
    std::string_view url{start, it};
    std::string_view param;
    if(*it == '?'){ // request contains parameters parse those
        start = ++it;
        find(' ');
        param = {start, it};
    }

    constexpr auto handshake = " HTTP/1.1\r\n"sv;
    wait_for_count(handshake.size());
    if(!std::equal(handshake.begin(), handshake.end(), it))
        throw std::logic_error{"Wrong HTTP version"};
    it += handshake.size();

    return {type, url, param};
}

std::string_view HttpConnection::receive_header_value(){
    auto start = it;
    find_first_of("\r\n"sv);
    auto str = std::string_view{start, it};
    it += *it == '\r' ? 2 : 1;
    return str;
};

void HttpConnection::receive_headers(){
    while(true){
        auto key_start = it;
        find_first_of(":\r\n"sv);
        auto key = std::string_view{key_start, it};
        if(*it == ':'){
            it += 2; // Skip : and whitespace, not entirely robust
            if(key == "Connection"){
                if(*it == 'K'){
                    auto value = receive_header_value();
                    if(value != "keep-alive"){
                        connection_keepalive = std::numeric_limits<std::time_t>::max();
                    }
                }else{
                    connection_keepalive = std::time(nullptr) + keepalive_timeout;
                }
            }
        }else{
            it += *it == '\r'? 2 : 1; // Skip past final endline
            return;
        }
    }
}



void HttpConnection::write_response(const Response& response){
    auto s = std::stringstream{};
    s << "HTTP/1.1 " << response.response_code << " \r\n";
    s << "Server: HTPP/" << HTPP_VERSION << "\r\n";
    auto timestamp = time(nullptr);
    tm utc; gmtime_r(&timestamp, &utc);
    s << "Date: " << weekday(utc) << ", " << Fmt2Int{utc.tm_mday} << ' ' << month(utc) << ' ' << (utc.tm_year + 1900) << ' ' << Fmt2Int{utc.tm_hour} << ':' << Fmt2Int{utc.tm_min} << ':' << Fmt2Int{utc.tm_sec} << " GMT\r\n";
    if(connection_keepalive < std::numeric_limits<std::time_t>::max()){
        s << "Connection: keep-alive\r\n";
        s << "Keep-Alive: timeout=1, max=1000\r\n";
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

    socket.write_some(asio::buffer(s.view()));
}

HttpConnection::~HttpConnection(){
    socket.close();
}