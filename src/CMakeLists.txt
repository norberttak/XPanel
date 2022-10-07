add_library(xpanel SHARED
    Action.cpp
    ArduinoHomeCockpit.cpp
    configparser.cpp
    configuration.cpp
    Device.cpp
    generic_display.cpp
    logger.cpp
    lua_helper.cpp
    message_window.cpp
    multi_purpose_display.cpp
    SaitekMultiPanel.cpp
    SaitekRadioPanel.cpp
    trigger.cpp
    UsbHidDevice.cpp
    xpanel.cpp
)

target_link_libraries(xpanel PRIVATE xpsdk::xpsdk hidapi::hidapi Lua::Lua)
set_target_properties(xpanel PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN TRUE)
target_compile_definitions(xpanel PRIVATE PLUGIN_SIGNATURE="${PROJECT_NAME_LOWERCASE}" PLUGIN_VERSION="${PROJECT_VERSION}")

if (WIN32)
    set(XPANEL_PLUGIN_FILENAME "win")
elseif (APPLE)
    set(XPANEL_PLUGIN_FILENAME "mac")
elseif (UNIX)
    set(XPANEL_PLUGIN_FILENAME "lin")
endif()

set_target_properties(xpanel PROPERTIES PREFIX "")
set_target_properties(xpanel PROPERTIES OUTPUT_NAME ${XPANEL_PLUGIN_FILENAME})
set_target_properties(xpanel PROPERTIES SUFFIX ".xpl")

install(TARGETS xpanel DESTINATION ${PROJECT_NAME}/64)
