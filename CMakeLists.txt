cmake_minimum_required(VERSION 3.14)

project(XPanel VERSION 1.7 LANGUAGES CXX)

string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWERCASE)
set(PLUGIN_INSTALL_DIR ${PROJECT_NAME}/64)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/.cmake)

option(INSTALL_DEPS "Install dependencies into the plugin's directory" OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 20 CACHE STRING "C++ standard")
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "C++ standard is a requirement")
set(CMAKE_CXX_EXTENSIONS OFF CACHE BOOL "C++ compiler specific extensions")

if (NOT MSVC)
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include(CTest)

add_subdirectory(3rdparty/hidapi)
add_subdirectory(3rdparty/LUA)
add_subdirectory(3rdparty/SDK)
add_subdirectory(3rdparty/FIP-SDK)

add_subdirectory(src)
add_subdirectory(test)

install(DIRECTORY config/ DESTINATION sample_configs)
install(DIRECTORY doc/ DESTINATION doc)
install(FILES config/board-config.ini DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES 3rdparty/FIP-SDK/fonts/fip-fonts.bmp DESTINATION ${PLUGIN_INSTALL_DIR})
