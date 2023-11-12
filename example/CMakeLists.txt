add_executable(example main.cpp)
target_link_libraries(example htpp)
target_compile_options(example PRIVATE -Wall -Wpedantic -Wconversion -Wextra -Wswitch-enum)

target_compile_definitions(example PRIVATE STATIC_FILE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/static")