add_library(xpsdk::xpsdk INTERFACE IMPORTED GLOBAL)

target_include_directories(xpsdk::xpsdk INTERFACE CHeaders/XPLM CHeaders/Widgets)
target_compile_definitions(xpsdk::xpsdk INTERFACE XPLM200=1 XPLM210=1 XPLM300=1 XPLM301=1 XPLM302=1 XPLM303=1 XPLM400=1)

if (WIN32)
    target_compile_definitions(xpsdk::xpsdk INTERFACE APL=0 IBM=1 LIN=0)
    target_link_libraries(xpsdk::xpsdk
      INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/Libraries/Win/XPLM_64.lib
        ${CMAKE_CURRENT_SOURCE_DIR}/Libraries/Win/XPWidgets_64.lib
    )
elseif (APPLE)
    target_compile_definitions(xpsdk::xpsdk INTERFACE APL=1 IBM=0 LIN=0)
    list(APPEND CMAKE_FRAMEWORK_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Libraries/Mac")
    find_library(XPLM_LIBRARY NAMES XPLM XPLM.framework)
    find_library(XPWIDGETS_LIBRARY NAMES XPWidgets XPWidgets.framework)
    target_link_libraries(xpsdk::xpsdk
        INTERFACE
	    "-framework AppKit"
            "-framework CoreFoundation"
	    "-framework IOKit"
	    ${XPLM_LIBRARY}
	    ${XPWIDGETS_LIBRARY}
    )
elseif (UNIX)
    target_compile_definitions(xpsdk::xpsdk INTERFACE APL=0 IBM=0 LIN=1)
endif()
