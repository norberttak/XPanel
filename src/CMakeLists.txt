
if (WIN32)
    list(APPEND XPANEL_SRCS ../3rdparty/hidapi/windows/hid.c)
elseif (APPLE)
    list(APPEND XPANEL_SRCS ../3rdparty/hidapi/mac/hid.c)
elseif (UNIX)
    list(APPEND XPANEL_SRCS ../3rdparty/hidapi/linux/hid.c)   
endif()
set_source_files_properties(${XPANEL_SRCS} PROPERTIES LANGUAGE CXX)

add_library(xpanel SHARED
    ${XPANEL_SRCS}
    core/Action.cpp
    core/AgeingCounter.cpp
    core/IniFileParser.cpp
    core/ConfigParser.cpp
    core/Configuration.cpp
    core/Device.cpp
    core/GenericDisplay.cpp
    core/Logger.cpp
    core/LuaHelper.cpp
    core/MultiPurposeDisplay.cpp
    core/Trigger.cpp
    core/UsbHidDevice.cpp
    core/XPanel.cpp
    core/GenericScreen.cpp
    log-message-window/MessageWindow.cpp
    devices/arduino-homecockpit/ArduinoHomeCockpit.cpp
    devices/saitek-multi/SaitekMultiPanel.cpp
    devices/saitek-radio/SaitekRadioPanel.cpp
    devices/saitek-switch/SaitekSwitchPanel.cpp
    devices/fip/FIPDriver.cpp
    devices/fip/FIPDevice.cpp
    devices/fip/FIPPage.cpp
    devices/fip/FIPScreen.cpp
    devices/fip/FIPLayer.cpp
    devices/fip/FIPTextLayer.cpp
    devices/fip/FIPImageLayer.cpp
    devices/trc-1000/TRC1000.cpp
    devices/trc-1000/TRC1000PFD.cpp
    devices/trc-1000/TRC1000Audio.cpp
)

target_include_directories(xpanel PRIVATE ./ core devices config-ui ../3rdparty/hidapi)

if (UNIX AND NOT APPLE)
    include(FindPkgConfig)
    pkg_check_modules(libudev REQUIRED IMPORTED_TARGET libudev)
    find_library(HIDAPI_LIBRARY NAMES udev)
    target_link_libraries(xpanel PkgConfig::libudev)
endif()

target_link_libraries(xpanel xpsdk::xpsdk Lua::Lua FIPSDK::FIPSDK)

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
