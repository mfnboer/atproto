set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Qt6 6.4 REQUIRED COMPONENTS Quick QuickControls2 Core)

qt_add_library(libatproto STATIC
    client.h
    client.cpp
    tlds.h
    xjson.h
    xjson.cpp
    xrpc_client.h
    xrpc_client.cpp
    lexicon/app_bsky_actor.h
    lexicon/app_bsky_actor.cpp
    lexicon/com_atproto_server.h
    lexicon/com_atproto_server.cpp
    lexicon/app_bsky_feed.h
    lexicon/app_bsky_feed.cpp
    lexicon/lexicon.h
    lexicon/lexicon.cpp
    lexicon/app_bsky_richtext.h
    lexicon/app_bsky_richtext.cpp
    lexicon/app_bsky_embed.h
    lexicon/app_bsky_embed.cpp
    lexicon/com_atproto_label.h
    lexicon/com_atproto_label.cpp
    lexicon/com_atproto_repo.h
    lexicon/com_atproto_repo.cpp
    lexicon/app_bsky_graph.h
    lexicon/app_bsky_graph.cpp
    lexicon/com_atproto_identity.h
    lexicon/com_atproto_identity.cpp
    tlds.cpp
    at_uri.h
    at_uri.cpp
    post_master.h
    post_master.cpp
    lexicon/app_bsky_notification.h
    lexicon/app_bsky_notification.cpp
    graph_master.h
    graph_master.cpp
)


target_link_libraries(libatproto PRIVATE
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Core
)

target_include_directories(libatproto INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
