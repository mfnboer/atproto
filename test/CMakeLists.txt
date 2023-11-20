cmake_minimum_required(VERSION 3.16)

project(test_atproto LANGUAGES CXX)

enable_testing()

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Test Network)

qt_add_executable(test_atproto
    main.cpp
    test_at_uri.h
    test_post_master.h
)

target_link_libraries(test_atproto
    PRIVATE libatproto
    PRIVATE Qt6::Test
    PRIVATE Qt6::Network
)

