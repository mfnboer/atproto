cmake_minimum_required(VERSION 3.16)

project(appatproto VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT ANDROID)
    add_compile_options(-fsanitize=address)
    add_compile_options(-fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2 Core HttpServer)

qt_standard_project_setup()

add_subdirectory(lib)

if (NOT ANDROID)
    add_subdirectory(test)
endif()

qt_add_executable(appatproto
    main.cpp
)

qt_add_qml_module(appatproto
    URI atproto
    VERSION ${PROJECT_VERSION}
    QML_FILES Main.qml
    SOURCES test.h
    SOURCES test.cpp
    QML_FILES Login.qml
    RESOURCES qtquickcontrols2.conf
    SOURCES
    SOURCES
)

set_target_properties(appatproto PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appatproto
    PRIVATE libatproto
    PRIVATE Qt6::Quick
    PRIVATE Qt6::QuickControls2
    PRIVATE Qt6::Core
    PRIVATE Qt6::HttpServer
)

install(TARGETS appatproto
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
