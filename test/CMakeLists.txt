cmake_minimum_required(VERSION 3.16)

project(test_atproto LANGUAGES CXX)

enable_testing()

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Test Network Quick QuickControls2 Core)

add_compile_options(-Wall -Wextra -Werror)

if (NOT ANDROID)
    add_compile_options(-fsanitize=address)
    add_compile_options(-fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()

qt_add_executable(test_atproto
    test_at_uri.h
    test_rich_text_master.h
    main.cpp
    test_xjson.h)

set(LINK_LIBS
    PRIVATE libatproto
    PRIVATE Qt6::Test
    PRIVATE Qt6::Network
    PRIVATE Qt6::Quick
    PRIVATE Qt6::QuickControls2
    PRIVATE Qt6::Core
)

target_link_libraries(test_atproto ${LINK_LIBS})
