#include "XPLMDisplay.h"
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <utility>
#include <filesystem>
#include <string.h>
#include "lua.hpp"
#include "XPLMGraphics.h"
#include "XPLMPlugin.h"
#include "XPLMPlanes.h"
#include "XPLMProcessing.h"
#include "UsbHidDevice.h"
#include "SaitekMultiPanel.h"
#include "ArduinoHomeCockpit.h"
#include "configuration.h"
#include "configparser.h"
#include "Action.h"
#include "logger.h"

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
	return TRUE;
}
#endif
#if LIN
#include <GL/gl.h>
#elif __GNUC__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#ifndef XPLM300
#error This is made to be compiled against the XPLM300 SDK
#endif

float flight_loop_callback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void* inRefcon);
void stop_and_clear_xpanel_plugin();
int init_and_start_xpanel_plugin();

lua_State* lua;
std::vector<Configuration> config;
std::vector<Device*> devices;
std::thread* t_multi = NULL;
std::thread* t_home = NULL;
const float FLIGHT_LOOP_TIME_PERIOD = 0.2;

int g_menu_container_idx;
XPLMMenuID g_menu_id;
void menu_handler(void*, void*);

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
	lua = luaL_newstate();
	luaL_openlibs(lua);

	strcpy_s(outName, 16, "XPanel ver 1.0");
	strcpy_s(outSig, 16, "xpanel");
	strcpy_s(outDesc, 64, "A plugin to handle control devices using hidapi interface");

	g_menu_container_idx = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "XPanel", 0, 0);
	g_menu_id = XPLMCreateMenu("XPanel Plugin", XPLMFindPluginsMenu(), g_menu_container_idx, menu_handler, NULL);
	XPLMAppendMenuItem(g_menu_id, "Reload Plugin", (void*)&menu_item_reload, 1);

	return 1;
}

PLUGIN_API void	XPluginStop(void)
{
	Logger(TLogLevel::logINFO) << "plugin stop called" << std::endl;
	stop_and_clear_xpanel_plugin();
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

float flight_loop_callback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void* inRefcon)
{
	for (auto it : config)
	{
		// check and set LED states
		for (auto triggers : it.light_triggers)
		{
			for (auto trigger : triggers.second)
			{
				trigger->evaluate_and_store_action();
			}
		}
		// check and set 7 segment display states
		for (auto display : it.multi_displays)
		{
			display.second->evaluate_and_store_dataref_value();
		}
	}

	// process button push/release events
	ActionQueue::get_instance()->activate_actions_in_queue();
	return FLIGHT_LOOP_TIME_PERIOD;
}

void stop_and_clear_xpanel_plugin()
{
	XPLMUnregisterFlightLoopCallback(flight_loop_callback, NULL);

	for (auto dev : devices)
	{
		if (dev != NULL)
		{
			dev->stop(500);
			dev->release();		
		}
	}	
	devices.clear();
	config.clear();
	ActionQueue::get_instance()->clear_all_actions();
}

int init_and_start_xpanel_plugin(void)
{
	char aircraft_file_name[256];
	char aircraft_path[512];
	XPLMGetNthAircraftModel(0, aircraft_file_name, aircraft_path);
	Logger(TLogLevel::logINFO) << "aircraft file name: " << aircraft_file_name << std::endl;

	lua_pushstring(lua, aircraft_file_name);
	lua_setglobal(lua, "AIRCRAFT_FILENAME");
	lua_pushstring(lua, aircraft_path);
	lua_setglobal(lua, "AIRCRAFT_PATH");

	std::filesystem::path init_path = std::filesystem::path(aircraft_path);
	init_path = init_path.remove_filename();
	init_path /= "xpanel.ini";

	Configparser p;
	Logger(TLogLevel::logINFO) << "parse config file: " << init_path.string() << std::endl;
	int result = p.parse_file(init_path.string(), config);
	if (result != EXIT_SUCCESS)
	{
		Logger(TLogLevel::logERROR) << "error parsing config file" << std::endl;
		return EXIT_FAILURE;
	}
	Device* device;

	for (auto it : config)
	{
		switch (it.device_type) {
		case DeviceType::SAITEK_MULTI:
			// set default vid & pid if it's not set in config file
			if (it.vid == 0)
				it.vid = 0x06a3;
			if (it.pid == 0)
				it.pid = 0x0d06;
			Logger(TLogLevel::logDEBUG) << "add new saitek multi panel device" << std::endl;
			device = new SaitekMultiPanel(it);
			devices.push_back(device);
			device->connect();
			device->start();
			t_multi = new std::thread(&SaitekMultiPanel::thread_func, (SaitekMultiPanel*)device);
			break;
		case DeviceType::HOME_COCKPIT:
			// set default vid & pid if it's not set in config file
			if (it.vid == 0)
				it.vid = 0x2341;
			if (it.pid == 0)
				it.pid = 0x8036;
			Logger(TLogLevel::logDEBUG) << "add new homecockpit device" << std::endl;
			device = new ArduinoHomeCockpit(it);
			devices.push_back(device);
			device->connect();
			device->start();
			t_home = new std::thread(&ArduinoHomeCockpit::thread_func, (ArduinoHomeCockpit*)device);
			break;
		default:
			Logger(TLogLevel::logERROR) << "unknown device type" << std::endl;
			return EXIT_FAILURE;
			break;
		}
	}

	XPLMRegisterFlightLoopCallback(flight_loop_callback, FLIGHT_LOOP_TIME_PERIOD, NULL);
	Logger(TLogLevel::logINFO) << "successful init and start plugin" << std::endl;
	return EXIT_SUCCESS;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void* inParam)
{
	if (inFrom == XPLM_PLUGIN_XPLANE)
	{
		switch (inMsg) {
		case XPLM_MSG_AIRPORT_LOADED:
			if (init_and_start_xpanel_plugin() != 1)
			{
				Logger(TLogLevel::logERROR) << "error during plugin init and start" << std::endl;
			}
			break;
		case XPLM_MSG_PLANE_CRASHED:
		case XPLM_MSG_PLANE_UNLOADED:
			Logger(TLogLevel::logDEBUG) << "XPLM_MSG_PLANE_CRASHED or XPLM_MSG_PLANE_UNLOADED message recived" << std::endl;
			break;
		default:
			break;
		}
	}
}

void menu_handler(void* in_menu_ref, void* in_item_ref)
{
	MenuItemType menu_item = *(MenuItemType*)in_item_ref;

	switch (menu_item)
	{
	case MenuItemType::MENU_ITEM_RELOAD:
		Logger(TLogLevel::logINFO) << "Reload plugin initiated" << std::endl;
		stop_and_clear_xpanel_plugin();
		if (init_and_start_xpanel_plugin() != EXIT_SUCCESS)
			Logger(TLogLevel::logERROR) << "Plugin reload error" << std::endl;
		break;
	default:
		//
		break;
	}
}