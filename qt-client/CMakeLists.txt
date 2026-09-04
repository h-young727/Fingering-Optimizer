cmake_minimum_required(VERSION 3.16)

project(FingeringOptimizerClient LANGUAGES CXX)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Widgets Network REQUIRED)

add_executable(FingeringOptimizerClient
    src/ApiClient.cpp
    src/LibraryView.cpp
    src/LoginView.cpp
    src/MainWindow.cpp
    src/TabResultView.cpp
    src/WeightPanel.cpp
    src/main.cpp
)

target_link_libraries(FingeringOptimizerClient
    PRIVATE
        Qt6::Network
        Qt6::Widgets
)
