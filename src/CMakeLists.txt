add_library(htpp server.cpp threadpool.cpp connection.cpp)
target_compile_definitions(htpp PRIVATE HTPP_VERSION="${HTPP_VERSION}")
target_link_libraries(htpp asio)
target_compile_options(htpp PRIVATE -Wall -Wpedantic -Wconversion -Wextra -Wswitch-enum)

target_sources(htpp PUBLIC
    FILE_SET httpPublic
    TYPE HEADERS
    BASE_DIRS include
    FILES
        include/htpp/lib.h
        include/htpp/json.h
        include/htpp/http.h)