add_library(doctest::doctest INTERFACE IMPORTED GLOBAL)

target_include_directories(doctest::doctest INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
