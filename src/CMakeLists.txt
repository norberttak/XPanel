add_library(xpanel SHARED
    Action.cpp
    ArduinoHomeCockpit.cpp
    ConfigParser.cpp
    Configuration.cpp
    Device.cpp
    GenericDisplay.cpp
    Logger.cpp
    LuaHelper.cpp
    MessageWindow.cpp
    MultiPurposeDisplay.cpp
    SaitekMultiPanel.cpp
    SaitekRadioPanel.cpp
    Trigger.cpp
    UsbHidDevice.cpp
    XPanel.cpp
    FIPDriver.cpp
    FIPDevice.cpp
    FIPPage.cpp
    FIPScreen.cpp
    GenericScreen.cpp
    RawBMP.cpp
)

target_link_libraries(xpanel PRIVATE xpsdk::xpsdk hidapi::hidapi Lua::Lua FIPSDK::FIPSDK)
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

install(TARGETS xpanel DESTINATION ${PLUGIN_INSTALL_DIR})
