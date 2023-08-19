find_package(Qt6 6.4 REQUIRED COMPONENTS Quick QuickControls2 Core)

qt_add_library(libatproto STATIC
    client.h
    xrpc_client.h
    xrpc_client.cpp
    client.cpp
    xjson.h
    xjson.cpp
    lexicon/app_bsky_actor.h
    lexicon/app_bsky_actor.cpp
)


target_link_libraries(libatproto PRIVATE
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Core
)

target_include_directories(libatproto INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})