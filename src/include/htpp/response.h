#pragma once
#include <htpp/http.h>
#include <sstream>
#include <optional>
#include <functional>
#include <iostream>

namespace htpp
{
    class Context{
    protected:
        std::stringstream response_buffer;
        virtual void default_headers() = 0;
        virtual void send_response() = 0;
    public:
        // Don't override destructor, we shouldn't need it

        template<ResponseConcept ResponseType>
        void send(const ResponseType& response){
            std::stringstream& s = response_buffer;
            s << "HTTP/1.1 " << response.response_code();
            response.header_line(s);
            s  << " \r\n";
            default_headers();
            if constexpr( ContentConcept<ResponseType> ){
                s << "Content-Type: " << to_str(response.content_type()) << "\r\n";

                if constexpr( SizedContentConcept<ResponseType> ){
                    s << "Content-Length: " << response.content_size() << "\r\n\r\n";
                    response.print_content(s);
                }else{
                    std::stringstream data{};// Consider improving this allocation
                    response.print_content(data);
                    s << "Content-Length: " << data.view().size() << "\r\n\r\n";
                    s << data.rdbuf();
                }
            }else{
                s << "\r\n";
            }
            send_response();
        }
    };
} // namespace htpp
