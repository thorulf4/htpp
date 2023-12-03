#pragma once
#include <asio.hpp>
#include <asio/ssl.hpp>

#include <optional>

class SslConnection {
    asio::ssl::stream<asio::ip::tcp::socket> socket;
public: 
    SslConnection(asio::ip::tcp::socket socket, asio::ssl::context& ctx): socket{std::move(socket), ctx} {}
    SslConnection(const SslConnection&) = delete;
    SslConnection& operator=(const SslConnection&) = delete;
    SslConnection(SslConnection&&) noexcept = default;
    SslConnection& operator=(SslConnection&&) noexcept = default;

    asio::awaitable<void> init(){
        return socket.async_handshake(asio::ssl::stream_base::server, asio::use_awaitable);
    }
    asio::awaitable<std::size_t> receive(asio::mutable_buffer buffer) {
        return socket.async_read_some(buffer, asio::use_awaitable);
    }
    void write(asio::const_buffer data) {
        socket.write_some(data);
    }
    std::size_t available(){
        return socket.lowest_layer().available();
    }
    asio::any_io_executor get_executor() {
        return socket.get_executor();
    }
    bool is_open(){
        return socket.lowest_layer().is_open();
    }
    asio::awaitable<void> close(){
        if(socket.lowest_layer().is_open()){
            co_await socket.async_shutdown(asio::use_awaitable);
        }
        socket.lowest_layer().close();
    }
};