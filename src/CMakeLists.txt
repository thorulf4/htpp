add_library(htpp server.cpp contenttype.cpp)
target_compile_definitions(htpp PRIVATE HTPP_VERSION="${HTPP_VERSION}")
target_link_libraries(htpp PUBLIC asio)
target_link_libraries(htpp PRIVATE OpenSSL::SSL)
target_compile_options(htpp PRIVATE -Wall -Wpedantic -Wconversion -Wextra -Wswitch-enum)

target_sources(htpp PUBLIC
    FILE_SET httpPublic
    TYPE HEADERS
    BASE_DIRS include
    FILES
        include/htpp/lib.h
        include/htpp/json.h
        include/htpp/http.h)

install(TARGETS htpp EXPORT HTPPConfig FILE_SET httpPublic)