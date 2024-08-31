set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Qt6 6.4 REQUIRED COMPONENTS Quick QuickControls2 Core Network)

if(CMAKE_BUILD_TYPE MATCHES Release)
    add_compile_definitions(QT_NO_DEBUG_OUTPUT)
endif()

add_compile_options(-Wall -Wextra -Werror)

if (NOT ANDROID)
    add_compile_options(-fsanitize=address)
    add_compile_options(-fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)

    add_compile_options(-fprofile-arcs -ftest-coverage)
    add_link_options(-fprofile-arcs -ftest-coverage)
endif()

include_directories(lexicon)

qt_add_qml_module(libatproto
    URI atproto
    IMPORT_PATH .
    SOURCES
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
        user_preferences.h
        user_preferences.cpp
        at_regex.h
        at_regex.cpp
        presence.h
        lexicon/com_atproto_moderation.h
        lexicon/com_atproto_moderation.cpp
        profile_master.h
        profile_master.cpp
        lexicon/app_bsky_unspecced.h
        lexicon/app_bsky_unspecced.cpp
        rich_text_master.h
        rich_text_master.cpp
        lexicon/app_bsky_graph_include.h
        SOURCES qml_utils.h
        SOURCES lexicon/app_bsky_labeler.h
        SOURCES lexicon/app_bsky_labeler.cpp
        SOURCES lexicon/plc_directory.h
        SOURCES lexicon/plc_directory.cpp
        SOURCES plc_directory_client.h
        SOURCES plc_directory_client.cpp
        SOURCES lexicon/chat_bsky_actor.h
        SOURCES lexicon/chat_bsky_actor.cpp
        SOURCES lexicon/chat_bsky_convo.h
        SOURCES lexicon/chat_bsky_convo.cpp
        SOURCES chat_master.h
        SOURCES chat_master.cpp
        SOURCES lexicon/app_bsky_video.h
        SOURCES lexicon/app_bsky_video.cpp
)

if (NOT ANDROID)
    set(COVERAGE_LIB -lgcov)
endif()

target_link_libraries(libatproto PRIVATE
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Core
    Qt6::Network
    ${COVERAGE_LIB}
)

target_include_directories(libatproto INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
