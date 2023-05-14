add_library(xpanel SHARED
    Action.cpp
    AgeingCounter.cpp
    ArduinoHomeCockpit.cpp
    IniFileParser.cpp
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
    SaitekSwitchPanel.cpp
    Trigger.cpp
    UsbHidDevice.cpp
    XPanel.cpp
    FIPDriver.cpp
    FIPDevice.cpp
    FIPPage.cpp
    FIPScreen.cpp
    FIPLayer.cpp
    FIPTextLayer.cpp
    GenericScreen.cpp
    FIPImageLayer.cpp
    TRC1000.cpp
    TRC1000PFD.cpp
    TRC1000Audio.cpp
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

set_target_properties(xpanel PROPERTIES PREFIX "" OUTPUT_NAME ${XPANEL_PLUGIN_FILENAME} SUFFIX ".xpl")

if (INSTALL_DEPS)
    set_target_properties(xpanel PROPERTIES INSTALL_RPATH "\$ORIGIN")
    install(TARGETS xpanel
        RUNTIME_DEPENDENCIES POST_EXCLUDE_REGEXES "lib(c|gcc_s|m|stdc|stdc\\+\\+|atomic|rt|dl|pthread)\\." "ld-linux"
        DESTINATION ${PLUGIN_INSTALL_DIR}
    )
else()
    install(TARGETS xpanel DESTINATION ${PLUGIN_INSTALL_DIR})
endif()
