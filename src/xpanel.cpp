#include "XPLMDisplay.h"
#include <vector>
#include <thread>
#include <chrono>
#include <utility>
#include <filesystem>
#include <string.h>
#include "lua.hpp"
#include "XPLMGraphics.h"
#include "XPLMPlanes.h"
#include "SaitekMultiPanel.h"
#include "configuration.h"
#include "configparser.h"

#if IBM
#include <windows.h>
#endif
#if LIN
#include <GL/gl.h>
#elif __GNUC__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#ifndef XPLM302
#error This is made to be compiled against the XPLM302 SDK
#endif

lua_State* lua;
std::vector<Configuration> config;
Configparser* p = NULL;

PLUGIN_API int XPluginStart(
	char* outName,
	char* outSig,
	char* outDesc)
{
	lua = luaL_newstate();
	luaL_openlibs(lua);
	
	strcpy_s(outName, 16, "XPanel ver 1.0");
	strcpy_s(outSig, 16, "xpanel");
	strcpy_s(outDesc, 64, "A plugin to handle control devices using hidapi interface");

	char aircraft_file_name[256];
	char aircraft_path[512];
	XPLMGetNthAircraftModel(0, aircraft_file_name, aircraft_path);	
	lua_pushstring(lua, aircraft_file_name);
	lua_setglobal(lua, "AIRCRAFT_FILENAME");
	lua_pushstring(lua, aircraft_path);
	lua_setglobal(lua, "AIRCRAFT_PATH");
	
	std::filesystem::path init_path = std::filesystem::path(aircraft_path);
	init_path /= "xpanel.ini";
	int result = p->parse_file(init_path.string(), config);
	// TODO: Handle parser error

	std::vector<Device*> devices;
	Device* device;

	for (auto it : config)
	{
		switch (it.device_type) {
		case DeviceType::SAITEK_MULTI:
			// set default vid & pid if it's not set in config file
			if (it.vid == 0)
				it.vid = 0x6a3;
			if (it.pid == 0)
				it.pid = 0x0d06;

			device = new SaitekMultiPanel(it);
			devices.push_back(device);
			std::thread t(&SaitekMultiPanel::thread_func, *(SaitekMultiPanel*)device);
			break;
		}				
	}

	return 1;
}

PLUGIN_API void	XPluginStop(void)
{
	lua_close(lua);
}

PLUGIN_API void XPluginDisable(void) 
{

}

PLUGIN_API int  XPluginEnable(void) 
{
	return 1;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void* inParam) 
{

}