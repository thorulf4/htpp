#include <htpp/lib.h>
#include <htpp/json.h>

#include <iostream>
#include <string>
#include <sstream>
#include <utility>
#include <mutex>
#include <ctime>

struct TimeResponse{
    using json_names = json::key_name<"hour", "minute", "second">;

    int hours;
    int minutes;
    int seconds;
};

asio::awaitable<void> handle_time(htpp::Context& ctx, std::string_view){
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);
    auto time = TimeResponse{now->tm_hour, now->tm_min, now->tm_sec};

    return ctx.send(json::From(time));
}

struct Msg{
    using json_names = json::key_name<"message">;
    std::string_view msg;
};

asio::awaitable<void> handle_json(htpp::Context& ctx, std::string_view){
    return ctx.send(json::From(Msg{"Hello, World!"}));
}

class Logger : public htpp::Middleware{
    std::ostream& os;
    std::mutex stream_lock;
public:
    explicit Logger(std::ostream& os) : os{os} {}
    void on_received(const htpp::Request& request) override {
        auto lock = std::unique_lock{stream_lock};
        os << to_str(request.type) << ' ' << request.url << '\n';
    }
};

int main(){
    using enum htpp::RequestType;

    htpp::Server{}
        // .use_https("localhost.pem", "localhost-key.pem")
        .set_threads(4)
        .set_static_files("/", STATIC_FILE_DIR)
        .add_middleware<Logger>(std::cout)
        .set_routes({
            {GET, "/json", handle_json},
            {GET, "/api/time", handle_time}
        })
        .run();
}