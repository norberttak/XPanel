set(XPANEL_TEST_SRCS
    ../src/core/Action.cpp
    ../src/core/AgeingCounter.cpp
    ../src/core/IniFileParser.cpp
    ../src/core/ConfigParser.cpp
    ../src/core/Configuration.cpp
    ../src/core/Device.cpp
    ../src/core/GenericDisplay.cpp
    ../src/core/GenericScreen.cpp
    ../src/core/Logger.cpp
    ../src/core/LuaHelper.cpp
    ../src/core/MultiPurposeDisplay.cpp
    ../src/core/Trigger.cpp
    ../src/core/UsbHidDevice.cpp
    ../src/core/XPanel.cpp
    ../src/log-message-window/MessageWindow.cpp
    ../src/devices/arduino-homecockpit/ArduinoHomeCockpit.cpp
    ../src/devices/saitek-multi/SaitekMultiPanel.cpp
    ../src/devices/saitek-radio/SaitekRadioPanel.cpp
    ../src/devices/saitek-switch/SaitekSwitchPanel.cpp
    ../src/devices/trc-1000/TRC1000.cpp
    ../src/devices/trc-1000/TRC1000PFD.cpp
    ../src/devices/trc-1000/TRC1000Audio.cpp
    ../src/devices/fip/FIPDriver.cpp
    ../src/devices/fip/FIPDevice.cpp
    ../src/devices/fip/FIPPage.cpp
    ../src/devices/fip/FIPScreen.cpp
    ../src/devices/fip/FIPLayer.cpp
    ../src/devices/fip/FIPTextLayer.cpp
    ../src/devices/fip/FIPImageLayer.cpp
    mock_xplane.cpp
    mock_hid_api.cpp
    mock_xplane_display.cpp
    test_main.cpp
    test_ageing_counter.cpp
    test_arduino_home.cpp
    test_config_parser.cpp
    test_dynamic_speed.cpp
    test_generic_display.cpp
    test_log_window.cpp
    test_lua.cpp
    test_multi_panel.cpp
    test_radio_panel.cpp
    test_trc1000_audio.cpp
    test_trc1000_pfd.cpp
    test_xpanel_plugin.cpp
    test_fip_macro.cpp
)

add_executable(xpanel_tests ${XPANEL_TEST_SRCS})

target_include_directories(xpanel_tests PRIVATE
    cmake-shim
    ../src
    ../src/core
    ../src/devices
    ../3rdparty/hidapi
    ../3rdparty/SDK/CHeaders/XPLM
    ../3rdparty/SDK/CHeaders/Widgets
    ../3rdparty/FIP-SDK/include
)

target_compile_definitions(xpanel_tests PRIVATE
    PLUGIN_SIGNATURE="${PROJECT_NAME_LOWERCASE}"
    PLUGIN_VERSION="${PROJECT_VERSION}"
    XPLM200=1 XPLM210=1 XPLM300=1 XPLM301=1 XPLM302=1 XPLM303=1 XPLM400=1
)

if (WIN32)
    target_compile_definitions(xpanel_tests PRIVATE APL=0 IBM=1 LIN=0)
elseif (APPLE)
    target_compile_definitions(xpanel_tests PRIVATE APL=1 IBM=0 LIN=0)
elseif (UNIX)
    target_compile_definitions(xpanel_tests PRIVATE APL=0 IBM=0 LIN=1)
endif()

# Deliberately not linking xpsdk::xpsdk: the XPLM/XPWidgets stub frameworks
# carry an @executable_path-relative install name meant for plugins loaded by
# X-Plane.app, which doesn't resolve for a standalone test binary. All XPLM*/
# XPWidgets* symbols used by src/ are provided by the mocks instead.
target_link_libraries(xpanel_tests PRIVATE Lua::Lua doctest::doctest)

# The test sources locate their fixture files via "../../test/...", mirroring
# the Windows test runner's working directory ($(VCInstallDir).../x64/Release,
# two levels below the repo root). CMAKE_CURRENT_BINARY_DIR (build/test) is
# also two levels below the repo root, so the same relative paths resolve to
# this directory's fixture files.
file(COPY ${CMAKE_SOURCE_DIR}/sample-config/board-config.ini DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/test/fip-images/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/fip-images)
file(COPY ${CMAKE_SOURCE_DIR}/test/macros/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/macros)
file(COPY ${CMAKE_SOURCE_DIR}/3rdparty/FIP-SDK/fonts/fip-fonts.bmp DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set(_test_wd ${CMAKE_CURRENT_BINARY_DIR})
add_test(NAME test_ageing_counter    COMMAND xpanel_tests --test-suite=test_ageing_counter       WORKING_DIRECTORY ${_test_wd})
add_test(NAME test_arduino_home      COMMAND xpanel_tests --test-suite=test_arduino_home_panel   WORKING_DIRECTORY ${_test_wd})
add_test(NAME test_config_parser     COMMAND xpanel_tests --test-suite=test_config               WORKING_DIRECTORY ${_test_wd})
add_test(NAME test_dynamic_speed     COMMAND xpanel_tests --test-suite=test_dynamic_speed        WORKING_DIRECTORY ${_test_wd})
add_test(NAME test_fip               COMMAND xpanel_tests --test-suite=TestFIP                   WORKING_DIRECTORY ${_test_wd})
add_test(NAME test_generic_display   COMMAND xpanel_tests --test-suite=test_generic_display      WORKING_DIRECTORY ${_test_wd})
add_test(NAME test_log_window        COMMAND xpanel_tests --test-suite=TestLogWindow              WORKING_DIRECTORY ${_test_wd})
add_test(NAME test_lua               COMMAND xpanel_tests --test-suite=test_lua                  WORKING_DIRECTORY ${_test_wd})
add_test(NAME test_multi_panel       COMMAND xpanel_tests --test-suite=test_multi_panel           WORKING_DIRECTORY ${_test_wd})
add_test(NAME test_radio_panel       COMMAND xpanel_tests --test-suite=test_radio_panel           WORKING_DIRECTORY ${_test_wd})
add_test(NAME test_trc1000_audio     COMMAND xpanel_tests --test-suite=test_trc1000_audio_panel   WORKING_DIRECTORY ${_test_wd})
add_test(NAME test_trc1000_pfd       COMMAND xpanel_tests --test-suite=test_trc1000_pfd_panel     WORKING_DIRECTORY ${_test_wd})
add_test(NAME test_xpanel_plugin     COMMAND xpanel_tests --test-suite=test_plugin                WORKING_DIRECTORY ${_test_wd})
add_test(NAME test_fip_macro         COMMAND xpanel_tests --test-suite=test_fip_macro             WORKING_DIRECTORY ${_test_wd})
