/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "XPLMDisplay.h"
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <utility>
#include <filesystem>
#include <cstring>
#include <cstdio>
#include "lua.hpp"
#include "XPLMGraphics.h"
#include "XPLMPlugin.h"
#include "XPLMPlanes.h"
#include "XPLMProcessing.h"
#include "UsbHidDevice.h"
#include "saitek-multi/SaitekMultiPanel.h"
#include "saitek-radio/SaitekRadioPanel.h"
#include "saitek-switch/SaitekSwitchPanel.h"
#include "arduino-homecockpit/ArduinoHomeCockpit.h"
#include "core/Configuration.h"
#include "core/ConfigParser.h"
#include "core/Action.h"
#include "fip/FIPDevice.h"
#include "trc-1000/TRC1000PFD.h"
#include "trc-1000/TRC1000Audio.h"
#include "core/LuaHelper.h"
#include "log-message-window/MessageWindow.h"
#include "core/Logger.h"

#if IBM
#include <windows.h>
BOOL APIENTRY DllMain(HANDLE hModule,
	DWORD ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return true;
}
#endif

#ifndef XPLM300
#error This is made to be compiled against the XPLM300 SDK
#endif

float flight_loop_callback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void* inRefcon);
float error_display_callback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void* inRefcon);

void stop_and_clear_xpanel_plugin();
int init_and_start_xpanel_plugin();

std::vector<Device*> devices;
bool plugin_already_initialized = false;
const float FLIGHT_LOOP_TIME_PERIOD = 0.2f;
const float ERROR_DISPLAY_TIME_PERIOD = 2.0f;

int g_menu_container_idx;
XPLMMenuID g_menu_id;
void menu_handler(void*, void*);
MessageWindow* msg_window = NULL;

typedef enum {
	MENU_ITEM_RELOAD = 0
} MenuItemType;

MenuItemType menu_item_reload = MenuItemType::MENU_ITEM_RELOAD;

PLUGIN_API int XPluginStart(
	char* outName,
	char* outSig,
	char* outDesc)
{
	Logger::set_log_level(TLogLevel::logINFO);
	Logger(TLogLevel::logINFO) << "plugin start" << std::endl;

	//XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);
	//XPLMEnableFeature("XPLM_USE_NATIVE_WIDGET_WINDOWS", 1);

	snprintf(outName, 256, "XPanel ver %s", PLUGIN_VERSION);
	snprintf(outSig, 256, "%s", PLUGIN_SIGNATURE);
	snprintf(outDesc, 256, "A plugin to handle control devices using hidapi interface");

	Logger(TLogLevel::logINFO) << "XPanel ver " PLUGIN_VERSION << std::endl;
	Logger(TLogLevel::logINFO) << "Built at " __DATE__ " " __TIME__ << std::endl;

	g_menu_container_idx = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "XPanel", 0, 0);
	g_menu_id = XPLMCreateMenu("XPanel Plugin", XPLMFindPluginsMenu(), g_menu_container_idx, menu_handler, NULL);
	XPLMAppendMenuItem(g_menu_id, "Reload Plugin", (void*)&menu_item_reload, 1);

	XPLMRegisterFlightLoopCallback(error_display_callback, ERROR_DISPLAY_TIME_PERIOD, NULL);

	return 1;
}

PLUGIN_API void	XPluginStop(void)
{
	Logger(TLogLevel::logINFO) << "plugin stop called" << std::endl;
	stop_and_clear_xpanel_plugin();
	XPLMUnregisterFlightLoopCallback(error_display_callback, NULL);
	XPLMDestroyMenu(g_menu_id);
}

PLUGIN_API void XPluginDisable(void)
{
	Logger(TLogLevel::logINFO) << "plugin disable called" << std::endl;
}

PLUGIN_API int  XPluginEnable(void)
{
	Logger(TLogLevel::logINFO) << "plugin enable called" << std::endl;
	return 1;
}

float flight_loop_callback(float, float, int, void*)
{
	for (auto dev : devices)
	{
		for (const auto& triggers : dev->get_config().light_triggers)
		{
			for (auto& trigger : triggers.second)
			{
				trigger->evaluate_and_store_action();
			}
		}

		// check and set 7 segment display states
		for (const auto& display : dev->get_config().multi_displays)
		{
			display.second->evaluate_and_store_dataref_value();
		}

		for (const auto& display : dev->get_config().generic_displays)
		{
			display.second->evaluate_and_store_dataref_value();
		}

		// update the FIP devices
		for (const auto& screen : dev->get_config().fip_screens)
		{
			screen.second->evaluate_and_store_screen_action();
		}
	}

	// execute lua scrip defined flight_loop function
	LuaHelper::get_instace()->do_flight_loop();

	// process button push/release events
	ActionQueue::get_instance()->activate_actions_in_queue();
	return FLIGHT_LOOP_TIME_PERIOD;
}

float error_display_callback(float, float, int, void*)
{
	// Nothing to display. Call me back later.
	if (Logger::number_of_stored_messages() == 0)
		return ERROR_DISPLAY_TIME_PERIOD;

	std::list<std::string> error_msgs = Logger::get_and_clear_stored_messages();
	if (!msg_window)
		msg_window = new MessageWindow(std::string("Xpanel: Errors and Warnings"));
	msg_window->append_multi_lines(error_msgs);
	msg_window->show();

	return ERROR_DISPLAY_TIME_PERIOD;
}

void stop_and_clear_xpanel_plugin()
{
	XPLMUnregisterFlightLoopCallback(flight_loop_callback, NULL);
	LuaHelper::get_instace()->close();

	for (auto& dev : devices)
	{
		if (dev != NULL)
		{
			dev->stop(500);
			dev->release();
		}
	}
	devices.clear();
	ActionQueue::get_instance()->clear_all_actions();
	plugin_already_initialized = false;

	if (msg_window)
		delete msg_window;
	msg_window = NULL;
}

std::filesystem::path absolute_path(std::string aircraft_path, std::string file_name)
{
	std::filesystem::path init_path = std::filesystem::path(aircraft_path);
	init_path = init_path.remove_filename();
	init_path /= file_name;

	return init_path;
}

extern std::filesystem::path get_plugin_path()
{
	char plugin_xpl_path[256];
	XPLMPluginID plugin_id = XPLMFindPluginBySignature(PLUGIN_SIGNATURE);
	if (plugin_id == XPLM_NO_PLUGIN_ID)
	{
		Logger(TLogLevel::logERROR) << "get_plugin_path: unable to find plugin by signature: " << PLUGIN_SIGNATURE << std::endl;
	}
	XPLMGetPluginInfo(plugin_id, NULL, plugin_xpl_path, NULL, NULL);

	std::filesystem::path plugin_path = std::filesystem::path(plugin_xpl_path);
	plugin_path = plugin_path.remove_filename();

	return plugin_path;
}

template <class T>
int enumerate_and_add_hid_devices(ClassConfiguration& it)
{
	Device* device;
	struct hid_device_info* dev_info;
	struct hid_device_info* dev_info_first;

	dev_info = hid_enumerate(it.vid, it.pid);
	if (!dev_info) {
		Logger(TLogLevel::logERROR) << "error enumerating hid device with vid=" << it.vid << " pid=" << it.pid << std::endl;
		hid_free_enumeration(dev_info);
		return EXIT_FAILURE;
	}
	dev_info_first = dev_info;
	do
	{		
		Logger(TLogLevel::logDEBUG) << "add new panel device. vid =" << it.vid << " pid = " << it.pid << std::endl;
		device = new T(it);
		devices.push_back(device);

		hid_device* hid_dev = hid_open_path(dev_info->path);
		hid_set_nonblocking(hid_dev, 1);
		((T*)device)->connect(hid_dev);
		device->start();
		device->thread_handle = new std::thread(&T::thread_func, (T*)device);
		LuaHelper::get_instace()->register_hid_device((UsbHidDevice*)device);
		dev_info = dev_info->next;
	} while (dev_info);
	hid_free_enumeration(dev_info_first);

	return EXIT_SUCCESS;
}

int init_and_start_xpanel_plugin(void)
{
	Configuration config;

	char aircraft_file_name[256];
	char aircraft_file_path[512];
	XPLMGetNthAircraftModel(0, aircraft_file_name, aircraft_file_path);
	Logger(TLogLevel::logINFO) << "aircraft file name: " << aircraft_file_name << std::endl;

	std::filesystem::path init_path = absolute_path(aircraft_file_path, "xpanel.ini");
	Logger(logINFO) << "config file: " << init_path.string() << std::endl;

	config.aircraft_path = std::filesystem::absolute(init_path).remove_filename().string();
	Logger(TLogLevel::logINFO) << "aircraft path: " << config.aircraft_path << std::endl;

	std::filesystem::path plugin_path = get_plugin_path();
	config.plugin_path = plugin_path.string();

	Configparser p;
	int result = p.parse_file(init_path.string(), config);
	if (result != EXIT_SUCCESS)
	{
		stop_and_clear_xpanel_plugin();
		Logger(TLogLevel::logERROR) << "error parsing config file" << std::endl;
		return EXIT_FAILURE;
	}

	if (config.aircraft_acf.compare(aircraft_file_name))
	{
		Logger(TLogLevel::logWARNING) << "Config was created for another aircraft (" << config.aircraft_acf << "). Current is " << aircraft_file_name << std::endl;
		//stop_and_clear_xpanel_plugin();
		//return EXIT_FAILURE;
	}

	LuaHelper::get_instace()->init();
	LuaHelper::get_instace()->push_global_string("AIRCRAFT_FILENAME", aircraft_file_name);
	LuaHelper::get_instace()->push_global_string("AIRCRAFT_PATH", aircraft_file_path);

	std::filesystem::path script_path;
	if (config.script_file != "")
		script_path = absolute_path(aircraft_file_path, config.script_file);
	if (std::filesystem::exists(script_path))
	{
		if (LuaHelper::get_instace()->load_script_file(script_path.string()) != EXIT_SUCCESS)
		{
			stop_and_clear_xpanel_plugin();
			Logger(TLogLevel::logERROR) << "Error loading Lua script: " << config.script_file << std::endl;
			return EXIT_FAILURE;
		}
	}

	Device* device;

	for (auto& it : config.class_configs)
	{
		switch (it.device_class_type) {
		case DeviceClassType::SAITEK_MULTI:
			// set default vid & pid if it's not set in config file
			if (it.vid == 0)
				it.vid = 0x06a3;
			if (it.pid == 0)
				it.pid = 0x0d06;
			Logger(TLogLevel::logDEBUG) << "add new saitek multi panel device" << std::endl;
			enumerate_and_add_hid_devices<SaitekMultiPanel>(it);
			break;
		case DeviceClassType::HOME_COCKPIT:
			// set default vid & pid if it's not set in config file
			if (it.vid == 0)
				it.vid = 0x2341;
			if (it.pid == 0)
				it.pid = 0x8036;
			Logger(TLogLevel::logDEBUG) << "add new homecockpit devices" << std::endl;
			enumerate_and_add_hid_devices<ArduinoHomeCockpit>(it);
			break;
		case DeviceClassType::SAITEK_RADIO:
			// set default vid & pid if it's not set in config file
			if (it.vid == 0)
				it.vid = 0x06a3;
			if (it.pid == 0)
				it.pid = 0x0d05;
			Logger(TLogLevel::logDEBUG) << "add new saitek radio devices" << std::endl;
			enumerate_and_add_hid_devices<SaitekRadioPanel>(it);
			break;
		case DeviceClassType::SAITEK_SWITCH:
			// set default vid & pid if it's not set in config file
			if (it.vid == 0)
				it.vid = 0x06a3;
			if (it.pid == 0)
				it.pid = 0x0d67;
			Logger(TLogLevel::logDEBUG) << "add new saitek switch panel devices" << std::endl;
			enumerate_and_add_hid_devices<SaitekSwitchPanel>(it);
			break;
		case DeviceClassType::LOGITECH_FIP:
			Logger(TLogLevel::logDEBUG) << "add new FIP device" << std::endl;
			device = new FIPDevice(it);
			devices.push_back(device);
			device->connect();
			device->start();
			device->thread_handle = new std::thread(&FIPDevice::thread_func, (FIPDevice*)device);
			break;
		case DeviceClassType::TRC1000_PFD:
			Logger(TLogLevel::logDEBUG) << "add new TRC1000 PFD devices" << std::endl;
			enumerate_and_add_hid_devices<TRC1000PFD>(it);
			break;
		case DeviceClassType::TRC1000_AUDIO:
			Logger(TLogLevel::logDEBUG) << "add new TRC1000 Audio devices" << std::endl;
			enumerate_and_add_hid_devices<TRC1000Audio>(it);
			break;
		default:
			Logger(TLogLevel::logERROR) << "unknown device type" << std::endl;
			return EXIT_FAILURE;
			break;
		}
	}

	XPLMRegisterFlightLoopCallback(flight_loop_callback, FLIGHT_LOOP_TIME_PERIOD, NULL);
	plugin_already_initialized = true;
	Logger(TLogLevel::logINFO) << "successful init and start plugin" << std::endl;
	return EXIT_SUCCESS;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void*)
{
	if (inFrom == XPLM_PLUGIN_XPLANE)
	{
		switch (inMsg) {
		case XPLM_MSG_AIRPORT_LOADED:
			Logger(TLogLevel::logTRACE) << "XPLM_MSG_AIRPORT_LOADED: message received" << std::endl;

			if (plugin_already_initialized)
				stop_and_clear_xpanel_plugin();

			if (init_and_start_xpanel_plugin() != EXIT_SUCCESS)
				Logger(TLogLevel::logERROR) << "error during plugin init and start" << std::endl;

			break;
		case XPLM_MSG_PLANE_CRASHED:
		case XPLM_MSG_PLANE_UNLOADED:
			Logger(TLogLevel::logINFO) << "XPLM_MSG_PLANE_CRASHED or XPLM_MSG_PLANE_UNLOADED message received" << std::endl;
			break;
		default:
			break;
		}
	}
}

void menu_handler(void*, void* in_item_ref)
{
	MenuItemType menu_item = *(MenuItemType*)in_item_ref;

	switch (menu_item)
	{
	case MenuItemType::MENU_ITEM_RELOAD:
		Logger(TLogLevel::logINFO) << "Reload plugin initiated" << std::endl;
		stop_and_clear_xpanel_plugin();
		if (init_and_start_xpanel_plugin() != EXIT_SUCCESS)
			Logger(TLogLevel::logERROR) << "Plugin reload error" << std::endl;
		Logger(TLogLevel::logTRACE) << "Reload plugin done" << std::endl;
		break;
	default:
		//
		break;
	}
}
