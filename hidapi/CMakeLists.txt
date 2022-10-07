if (WIN32)
    add_library(hidapi::hidapi SHARED IMPORTED GLOBAL)
    set_target_properties(hidapi::hidapi PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/hidapi.dll
        IMPORTED_IMPLIB ${CMAKE_CURRENT_SOURCE_DIR}/lib/hidapi.lib
    )
    target_include_directories(hidapi::hidapi INTERFACE headers)
else()
    find_package(hidapi REQUIRED)
endif()
