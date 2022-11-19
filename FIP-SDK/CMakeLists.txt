add_library(FIPSDK::FIPSDK INTERFACE IMPORTED GLOBAL)

if (WIN32)
    target_include_directories(FIPSDK::FIPSDK INTERFACE include)
    set_target_properties(FIPSDK::FIPSDK PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/DirectOutput.dll
    )

    install(FILES lib/DirectOutput.dll DESTINATION ${PLUGIN_INSTALL_DIR})
else()
    # Linux/Mac versions are not yet avaliable
endif()
