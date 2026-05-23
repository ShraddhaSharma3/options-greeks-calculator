cmake_minimum_required(VERSION 3.11)
project(options-greeks)

set(CMAKE_CXX_STANDARD 17)

find_package(ftxui REQUIRED)

add_executable(greeks main.cpp)
target_link_libraries(greeks
    ftxui::screen
    ftxui::dom
    ftxui::component
)