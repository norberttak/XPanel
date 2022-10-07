if (WIN32)
    add_library(Lua::Lua SHARED IMPORTED GLOBAL)
    set_target_properties(Lua::Lua PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lua54.dll
        IMPORTED_IMPLIB ${CMAKE_CURRENT_SOURCE_DIR}/liblua54.a
    )
    target_include_directories(Lua::Lua INTERFACE include)
else()
    find_package(Lua REQUIRED)

    add_library(Lua::Lua INTERFACE IMPORTED GLOBAL)
    target_link_libraries(Lua::Lua INTERFACE ${LUA_LIBRARIES})
    target_include_directories(Lua::Lua INTERFACE ${LUA_INCLUDE_DIR})
endif()
