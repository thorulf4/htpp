#include "connection.h"
#include <filesystem>
#include <fstream>

using asio::ip::tcp;
using namespace std::literals::string_view_literals;
using namespace htpp;

static RequestType get_type(char*& data){
    using enum RequestType;
    switch (data[0])
    {
        case 'G':
            data += 4;
            return GET;
        case 'H':
            data += 5;
            return HEAD;
        case 'P':
            switch (data[1])
            {
                case 'O':
                    data += 5;
                    return POST;
                case 'U':
                    data += 4;
                    return PUT;
                case 'A':
                    data += 6;
                    return PATCH;
                default:
                    throw std::logic_error{"4xx bad request type"};
            }
        case 'D':
            data += 7;
            return DELETE;
        case 'C':
            data += 8;
            return CONNECT;
        case 'O':
            data += 8;
            return OPTIONS;
        case 'T':
            data += 6;
            return TRACE;
        default:
            throw std::logic_error{"4xx bad request type"};
    }
    std::unreachable();
}

static std::string_view to_str(ContentType type){
    using enum ContentType;
    switch(type){
        case TextHtml: return "text/html";
        case TextJson: return "text/json";
        default: std::unreachable();
    }
}

void HttpConnection::receive(){
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
    find('\r');
    auto str = std::string_view{start, it};
    it += 2;
    return str;
};

void HttpConnection::receive_headers(){
    connection_keepalive = false; // Default
    while(true){
        auto key_start = it;
        find_first_of(":\r"sv);
        if(*it == ':'){
            auto key = std::string_view{key_start, it};
            it += 2; // Skip : and whitespace, not entirely robust
            if(key == "Connection" && *it == 'k'){
                auto value = receive_header_value();
                connection_keepalive = value == "keep-alive";
            }else{
                find('\n');
                it++;
            }
        }else{
            it += 2; // Skip past final endline
            return;
        }
    }
}

void HttpConnection::write_response(const Response& response){
    std::stringstream s{};
    s << "HTTP/1.1 " << response.response_code << " \r\n";
    s << "Server: HTPP/" << HTPP_VERSION << "\r\n";  
    if(connection_keepalive){
        s << "Connection: keep-alive\r\n";
        s << "Keep-Alive: timeout=5, max=1000\r\n";
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