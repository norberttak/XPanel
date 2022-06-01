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

lua_State* lua;
std::vector<Configuration> config;
std::vector<Device*> devices;
std::thread* t_multi = NULL;
std::thread* t_home = NULL;
const float FLIGHT_LOOP_TIME_PERIOD = 0.2;

PLUGIN_API int XPluginStart(
	char* outName,
	char* outSig,
	char* outDesc)
{
	XPLMDebugString("XPanel: Plugin Start\n");
	lua = luaL_newstate();
	luaL_openlibs(lua);
	
	strcpy_s(outName, 16, "XPanel ver 1.0");
	strcpy_s(outSig, 16, "xpanel");
	strcpy_s(outDesc, 64, "A plugin to handle control devices using hidapi interface");

	return 1;
}

PLUGIN_API void	XPluginStop(void)
{
	XPLMUnregisterFlightLoopCallback(flight_loop_callback, NULL);

	for (auto dev : devices)
	{
		if (dev != NULL)
		{
			dev->stop(0);
			dev->release();
			delete dev;
			dev = NULL;
		}
	}
	devices.clear();

	lua_close(lua);
}

PLUGIN_API void XPluginDisable(void) 
{

}

PLUGIN_API int  XPluginEnable(void) 
{
	return 1;
}

float flight_loop_callback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void* inRefcon)
{
	ActionQueue::get_instance()->activate_actions_in_queue();
	return FLIGHT_LOOP_TIME_PERIOD;
}

int init_and_start_xpanel_plugin(void)
{
	char aircraft_file_name[256];
	char aircraft_path[512];
	XPLMGetNthAircraftModel(0, aircraft_file_name, aircraft_path);
	XPLMDebugString("\nXPanel: aircraft file name:");
	XPLMDebugString(aircraft_file_name);

	lua_pushstring(lua, aircraft_file_name);
	lua_setglobal(lua, "AIRCRAFT_FILENAME");
	lua_pushstring(lua, aircraft_path);
	lua_setglobal(lua, "AIRCRAFT_PATH");
	
	std::filesystem::path init_path = std::filesystem::path(aircraft_path);
	init_path = init_path.remove_filename();
	init_path /= "xpanel.ini";

	Configparser p;
	int result = p.parse_file(init_path.string(), config);
	if (result != EXIT_SUCCESS)
	{
		XPLMDebugString(p.get_last_error_message().c_str());
		return 0;
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
			XPLMDebugString("XPanel: add new homecockpit device");
			device = new ArduinoHomeCockpit(it);
			devices.push_back(device);
			device->connect();
			device->start();
			t_home = new std::thread(&ArduinoHomeCockpit::thread_func, (ArduinoHomeCockpit*)device);
			break;
		default:
			XPLMDebugString("XPanel: Unknown device type\n");
			return 0;
			break;
		}
	}

	XPLMRegisterFlightLoopCallback(flight_loop_callback, FLIGHT_LOOP_TIME_PERIOD, NULL);

	return 1;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void* inParam) 
{
	if (inFrom == XPLM_PLUGIN_XPLANE) 
	{
		switch (inMsg) {
		case XPLM_MSG_AIRPORT_LOADED:
			if (init_and_start_xpanel_plugin() != 1)
			{
				XPLMDebugString("XPanel: Error during plugin init and start\n");
			}
			XPLMDebugString("XPanel: Succesful plugin init and start\n");
			break;
		case XPLM_MSG_PLANE_CRASHED:
		case XPLM_MSG_PLANE_UNLOADED:
			//
			break;
		default:
			break;
		}
	}
}