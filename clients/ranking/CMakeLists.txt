cmake_minimum_required(VERSION 3.16)
project(RankingClient)

set(CMAKE_CXX_STANDARD 17)

# 🟢 Habilita AUTOMOC y AUTOUIC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# ✅ Sin QtCharts
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Network)

add_executable(RankingClient
    main.cpp
    ui/VentanaRanking.cpp
    ui/VentanaRanking.h
    network/ClienteRanking.cpp
    network/ClienteRanking.h
)

target_include_directories(RankingClient PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/network
)

target_link_libraries(RankingClient
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Network
)
