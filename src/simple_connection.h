#pragma once
#include <asio.hpp>
#include <iostream>

class SimpleConnection {
    asio::ip::tcp::socket socket;
public:
    SimpleConnection(asio::ip::tcp::socket socket): socket{std::move(socket)} {}
    SimpleConnection(const SimpleConnection&) = delete;
    SimpleConnection& operator=(const SimpleConnection&) = delete;
    SimpleConnection(SimpleConnection&&) noexcept = default;
    SimpleConnection& operator=(SimpleConnection&&) noexcept = default;

    [[nodiscard]] asio::awaitable<void> init(){ co_return; }
    [[nodiscard]] asio::awaitable<std::size_t> receive(asio::mutable_buffer buffer) {
        return socket.async_receive(buffer, asio::use_awaitable);
    }
    [[nodiscard]] asio::awaitable<size_t> write(asio::const_buffer data) {
        return asio::async_write(socket, data, asio::use_awaitable);
    }
    std::size_t available(){
        return socket.available();
    }
    [[nodiscard]] asio::any_io_executor get_executor() {
        return socket.get_executor();
    }
    bool is_open(){
        return socket.is_open();
    }
    [[nodiscard]] asio::awaitable<void> close(){
        if(socket.is_open()){
            socket.shutdown(asio::ip::tcp::socket::shutdown_send);
        }
        socket.close();
        co_return;
    }
};