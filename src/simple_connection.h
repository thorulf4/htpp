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

    asio::awaitable<void> init(){ co_return; }
    asio::awaitable<std::size_t> receive(asio::mutable_buffer buffer) {
        return socket.async_receive(buffer, asio::use_awaitable);
    }
    void write(asio::const_buffer data) {
        socket.write_some(data);
    }
    std::size_t available(){
        return socket.available();
    }
    asio::any_io_executor get_executor() {
        return socket.get_executor();
    }
    bool is_open(){
        return socket.is_open();
    }
    asio::awaitable<void> close(){
        if(socket.is_open()){
            socket.shutdown(asio::ip::tcp::socket::shutdown_send);
        }
        socket.close();
        co_return;
    }
};