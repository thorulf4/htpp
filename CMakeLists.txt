cmake_minimum_required(VERSION 3.26)
option(HTPP_SAMPLE_PROJETS OFF)
option(ASAN OFF)

set(HTPP_VERSION 0.0.0)
set(CMAKE_CXX_STANDARD 23)

project(htpp VERSION ${HTPP_VERSION} LANGUAGES CXX)

add_library(asio INTERFACE)
target_compile_definitions(asio INTERFACE ASIO_STANDALONE)
target_link_libraries(asio INTERFACE pthread)

find_package(OpenSSL REQUIRED)

if(ASAN)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif(ASAN)

add_subdirectory(src)
if(HTPP_SAMPLE_PROJETS)
    add_subdirectory(example)
endif(HTPP_SAMPLE_PROJETS)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/HTPPConfigVersion.cmake VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion)

install(TARGETS asio EXPORT HTPPConfig)
install(EXPORT HTPPConfig DESTINATION lib/htpp/ NAMESPACE htpp::)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/HTPPConfigVersion.cmake DESTINATION lib/htpp/ )