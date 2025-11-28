cmake_minimum_required(VERSION 3.16)
project(first-time-chatter-dock VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(libobs REQUIRED)
find_package(obs-frontend-api REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Widgets Network WebSockets)

add_library(first-time-chatter-dock MODULE
    src/plugin-main.cpp
    src/first-time-chatter-dock.cpp
    src/first-time-chatter-dock.hpp
    src/twitch-irc-client.cpp
    src/twitch-irc-client.hpp
    src/settings-dialog.cpp
    src/settings-dialog.hpp
)

target_link_libraries(first-time-chatter-dock
    OBS::libobs
    OBS::obs-frontend-api
    Qt6::Widgets
    Qt6::Network
    Qt6::WebSockets
)

target_include_directories(first-time-chatter-dock PRIVATE src)

set_target_properties(first-time-chatter-dock PROPERTIES
    PREFIX ""
    OUTPUT_NAME "first-time-chatter-dock"
)
