/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <cstring>
#include <string>
#include "LuaHelper.h"
#include "lua.hpp"
#include "Logger.h"

LuaHelper* LuaHelper::instance = NULL;
lua_State* lua = NULL;

extern "C" {
#define LUA_FUNC_NAME_COMMAND_ONCE	"command_once"
#define LUA_FUNC_NAME_COMMAND_BEGIN "command_begin"
#define LUA_FUNC_NAME_COMMAND_END	"command_end"
#define LUA_FUNC_NAME_GET_DATAREF	"get_dataref"
#define LUA_FUNC_NAME_SET_DATAREF	"set_dataref"
#define LUA_FUNC_NAME_LOG			"log_msg"
#define LUA_FUNC_NAME_GET_BUTTON_STATE	"hid_get_button_state"
#define LUA_FUNC_NAME_GET_LIGHT_STATE "hid_get_light"

	static int LuaCommand(lua_State* L, const char* command_name)
	{
		Logger(TLogLevel::logTRACE) << "Lua XP command call:" << command_name << "(" << lua_tostring(L, 1) << ")" << std::endl;

		if (!lua_isstring(L, 1))
		{
			Logger(TLogLevel::logERROR) << "Lua command: invalid parameter. not a string?" << std::endl;
			return 0;
		}

		std::string dataref_str = lua_tostring(L, 1);
		XPLMCommandRef CommandId = XPLMFindCommand(dataref_str.c_str());
		if (CommandId == NULL)
		{
			Logger(TLogLevel::logERROR) << "Lua command: invalid dataref :" << dataref_str << std::endl;
			return 0;
		}
		if (strcmp(command_name, LUA_FUNC_NAME_COMMAND_ONCE) == 0)
			XPLMCommandOnce(CommandId);
		else if (strcmp(command_name, LUA_FUNC_NAME_COMMAND_BEGIN) == 0)
			XPLMCommandBegin(CommandId);
		else if (strcmp(command_name, LUA_FUNC_NAME_COMMAND_END) == 0)
			XPLMCommandEnd(CommandId);
		else
		{
			Logger(TLogLevel::logERROR) << "Unknonw command type: " << command_name << std::endl;
			return 0;
		}

		return 1;
	}

	static int LuaCommandOnce(lua_State* L)
	{
		return LuaCommand(L, LUA_FUNC_NAME_COMMAND_ONCE);
	}

	static int LuaCommandBegin(lua_State* L)
	{
		return LuaCommand(L, LUA_FUNC_NAME_COMMAND_BEGIN);
	}

	static int LuaCommandEnd(lua_State* L)
	{
		return LuaCommand(L, LUA_FUNC_NAME_COMMAND_END);
	}

	static int LuaGet(lua_State* L)
	{
		int  dataref_array_index = 0;

		if (!lua_isstring(L, 1))
		{
			Logger(TLogLevel::logERROR) << "Lua get: invalid parameter. not a string?" << std::endl;
			return 0;
		}

		std::string dataref_str = lua_tostring(L, 1);
		XPLMDataRef dataref = LuaHelper::get_instace()->get_dataref(dataref_str);
		if (dataref == NULL)
		{
			Logger(TLogLevel::logERROR) << "Lua get: invalid dataref :" << dataref_str << std::endl;
			return 0;
		}

		if (lua_isnumber(L, 2))
		{
			dataref_array_index = lua_tointeger(L, 2);
		}

		XPLMDataTypeID dataref_type = LuaHelper::get_instace()->get_dataref_type(dataref);
		int value_ia = 0;
		float value_fa = 0;

		switch (dataref_type) {
		case xplmType_Int:
			lua_pushnumber(L, XPLMGetDatai(dataref));
			break;
		case xplmType_Double:
			lua_pushnumber(L, XPLMGetDatad(dataref));
			break;
		case xplmType_Float:
			lua_pushnumber(L, XPLMGetDataf(dataref));
			break;
		case xplmType_IntArray:
			XPLMGetDatavi(dataref, &value_ia, dataref_array_index, 1);
			lua_pushnumber(L, value_ia);
			break;
		case xplmType_FloatArray:
			XPLMGetDatavf(dataref, &value_fa, dataref_array_index, 1);
			lua_pushnumber(L, value_fa);
			break;
		case xplmType_Data:
			char value_str[4096];
			XPLMGetDatab(dataref, value_str, 0, sizeof(value_str));
			lua_pushstring(L, value_str);
			break;
		default:
			Logger(TLogLevel::logERROR) << "Lua get: invalid datareftype: " << dataref_type << std::endl;
			return 0;
			break;
		}
		return 1;
	}

	static int LuaSet(lua_State* L)
	{

		if (!(lua_isstring(L, 1) && lua_isnumber(L, 2)))
		{
			Logger(TLogLevel::logERROR) << "lua set: wrong arguments to function set()" << std::endl;
			return 0;
		}

		std::string dataref_str = lua_tostring(L, 1);
		XPLMDataRef dataref = LuaHelper::get_instace()->get_dataref(dataref_str);
		if (dataref == NULL)
		{
			Logger(TLogLevel::logERROR) << "lua set: invalid dataref: " << dataref_str << std::endl;
			return 0;
		}

		XPLMDataTypeID dataref_type = LuaHelper::get_instace()->get_dataref_type(dataref);
		switch (dataref_type)
		{
		case xplmType_Int:
			XPLMSetDatai(dataref, (int)lua_tonumber(L, 2));
			break;
		case xplmType_Double:
			XPLMSetDatad(dataref, (double)lua_tonumber(L, 2));
			break;
		case xplmType_Float:
			XPLMSetDataf(dataref, (float)lua_tonumber(L, 2));
			break;
		case xplmType_IntArray:
			//
			break;
		case xplmType_FloatArray:
			//
			break;
		case xplmType_Data:
			//
		default:
			Logger(TLogLevel::logERROR) << "Lua set: invalid datareftype: " << dataref_type << std::endl;
			return 0;
			break;
		}

		return 1;
	}

	/*lua function: log_msg("ERROR","your log message")*/
	int LuaLogMsg(lua_State* L)
	{
		if (!(lua_isstring(L, 1) && lua_isstring(L, 2)))
		{
			Logger(TLogLevel::logERROR) << "lua set: wrong arguments to function log_msg()" << std::endl;
			return 0;
		}

		std::string log_level_str = lua_tostring(L, 1);
		std::string log_msg = lua_tostring(L, 2);

		TLogLevel log_level;
		if (log_level_str == "ERROR")
			log_level = TLogLevel::logERROR;
		else if (log_level_str == "WARNING")
			log_level = TLogLevel::logWARNING;
		else if (log_level_str == "INFO")
			log_level = TLogLevel::logINFO;
		else if (log_level_str == "DEBUG")
			log_level = TLogLevel::logDEBUG;
		else if (log_level_str == "TRACE")
			log_level = TLogLevel::logTRACE;
		else
			log_level = TLogLevel::logTRACE;

		Logger(log_level) << "LUA script: " << log_msg << std::endl;
		return 1;
	}

	// hid_get_button_state(vid,pid,button_name)
	int LuaGetButtonState(lua_State* L)
	{
		if (!(lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isstring(L,3)))
		{
			Logger(TLogLevel::logERROR) << "lua: wrong arguments to function hid_get_button_state()" << std::endl;
			return 0;
		}

		int vid = (int)lua_tonumber(L, 1);
		int pid = (int)lua_tonumber(L, 2);
		std::string button_name = lua_tostring(L, 3);

		int state = LuaHelper::get_instace()->get_button_state(vid, pid, button_name);

		std::string button_state_str = "";

		if (state == -1)
		{
			Logger(TLogLevel::logWARNING) << "lua: error in get_button_state(" << vid << "," << pid << "," << button_name << ")" << std::endl;
			button_state_str = "UNKNOWN";
		}

		if (state == 1)
			button_state_str = "ON";
		else if (state == 0)
			button_state_str = "OFF";
		else
			button_state_str = "UNKNOWN";

		Logger(TLogLevel::logTRACE) << "lua: LuaGetButtonState "<< lua_tostring(L, 3) <<" return: " << button_state_str << std::endl;

		lua_pushstring(L, button_state_str.c_str());
		return 1;
	}

	// hid_get_light_state(vid,pid,light_name)
	int LuaGetLightState(lua_State* L)
	{
		if (!(lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isstring(L, 3)))
		{
			Logger(TLogLevel::logERROR) << "lua: wrong arguments to function hid_get_light_state()" << std::endl;
			return 0;
		}

		int vid = (int)lua_tonumber(L, 1);
		int pid = (int)lua_tonumber(L, 2);
		std::string light_name = lua_tostring(L, 3);

		TriggerType state = LuaHelper::get_instace()->get_light_state(vid, pid, light_name);

		if (state == TriggerType::UNKNOWN)
		{
			Logger(TLogLevel::logWARNING) << "lua: error in get_light_state(" << vid << "," << pid << "," << light_name << ")" << std::endl;
		}

		std::string state_str = "";

		switch (state) {
		case TriggerType::BLINK: state_str = "BLINK";
			break;
		case TriggerType::LIT: state_str = "LIT";
			break;
		case TriggerType::UNLIT: state_str = "UNLIT";
			break;
		default:
			state_str = "UNKOWN";
			break;
		}

		Logger(TLogLevel::logTRACE) << "lua: LuaGetLightState " << lua_tostring(L, 3) << " return: " << state_str << std::endl;

		lua_pushstring(L, state_str.c_str());
		return 1;
	}
}

LuaHelper::LuaHelper()
{
	flight_loop_defined = false;
	lua_enabled = false;
}

LuaHelper* LuaHelper::get_instace()
{
	if (instance == NULL)
		instance = new LuaHelper();
	return instance;
}

void LuaHelper::register_hid_device(UsbHidDevice* _device)
{
	hid_devices.push_back(_device);
}

int LuaHelper::get_button_state(int vid, int pid, std::string button_name)
{
	for (auto &dev : hid_devices)
	{
		if (dev->get_vid() == vid && dev->get_pid() == pid)
		{
			int buttonstate = dev->get_stored_button_state(button_name);
			if (buttonstate != -1)
				return buttonstate;
		}
	}

	return -1; // error case
}

TriggerType LuaHelper::get_light_state(int vid, int pid, std::string light_name)
{
	for (auto &dev : hid_devices)
	{
		if (dev->get_vid() == vid && dev->get_pid() == pid)
		{
			TriggerType lightstate = dev->get_stored_light_state(light_name);
			if (lightstate != TriggerType::UNKNOWN)
				return lightstate;
		}
	}

	return TriggerType::UNKNOWN; // error case
}

int LuaHelper::load_script_file(std::string file_name)
{
	if (luaL_dofile(lua, file_name.c_str()) != LUA_OK)
	{
		Logger(TLogLevel::logERROR) << "Lua: Error during load script " << file_name << " " << lua_tostring(lua, lua_gettop(lua)) << std::endl;
		Logger(TLogLevel::logERROR) << "Disable LUA module. fix error in lua script and reload the plugin" << std::endl;
		lua_enabled = false;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int LuaHelper::init()
{
	Logger(TLogLevel::logTRACE) << "LuaHelper init" << std::endl;

	flight_loop_defined = true;
	lua = luaL_newstate();
	if (!lua)
		return EXIT_FAILURE;
	
	luaL_openlibs(lua);
	lua_register(lua, LUA_FUNC_NAME_COMMAND_ONCE, LuaCommandOnce);
	lua_register(lua, LUA_FUNC_NAME_COMMAND_BEGIN, LuaCommandBegin);
	lua_register(lua, LUA_FUNC_NAME_COMMAND_END, LuaCommandEnd);
	lua_register(lua, LUA_FUNC_NAME_GET_DATAREF, LuaGet);
	lua_register(lua, LUA_FUNC_NAME_SET_DATAREF, LuaSet);
	lua_register(lua, LUA_FUNC_NAME_LOG, LuaLogMsg);
	lua_register(lua, LUA_FUNC_NAME_GET_BUTTON_STATE, LuaGetButtonState);
	lua_register(lua, LUA_FUNC_NAME_GET_LIGHT_STATE, LuaGetLightState);

	last_flight_loop_call = std::chrono::system_clock::now();

	lua_enabled = true;
	return EXIT_SUCCESS;
}

void LuaHelper::close()
{
	Logger(TLogLevel::logTRACE) << "LuaHelper close" << std::endl;
	command_refs.clear();
	data_refs.clear();
	data_ref_types.clear();

	if (lua)
		lua_close(lua);
	lua = NULL;
	lua_enabled = false;
}

void LuaHelper::push_global_string(std::string name, std::string value)
{
	Logger(TLogLevel::logTRACE) << "LuaHelper push_global_string " << name << "=" << value << std::endl;
	lua_pushstring(lua, value.c_str());
	lua_setglobal(lua, name.c_str());
}

/* The XPLMFindCommand is pretty expensive so we should call it only the first time
   and store the given reference for later usage. This function can return NULL if
   the CommandRef not found by XPlane */
XPLMCommandRef LuaHelper::get_commandref(std::string commandref_str)
{
	if (command_refs.count(commandref_str) == 0)
	{
		XPLMCommandRef commandId = XPLMFindCommand(commandref_str.c_str());
		command_refs[commandref_str] = commandId;
	}
	return command_refs[commandref_str];
}

XPLMDataRef LuaHelper::get_dataref(std::string dataref_str)
{
	if (data_refs.count(dataref_str) == 0)
	{
		XPLMDataRef datarefId = XPLMFindDataRef(dataref_str.c_str());
		data_refs[dataref_str] = datarefId;
	}
	return data_refs[dataref_str];
}

XPLMDataTypeID LuaHelper::get_dataref_type(XPLMDataRef dataref)
{
	if (data_ref_types.count(dataref) == 0)
	{
		XPLMDataTypeID dataref_type = XPLMGetDataRefTypes(dataref);

		/* from xplane documenation:
		Data types each take a bit field; it is legal to have a single dataref
		be more than one type of data. When this happens, you can pick
		any matching get/set API.
		*/
		if (dataref_type & xplmType_Double)
			data_ref_types[dataref] = xplmType_Double;
		else if (dataref_type & xplmType_Float)
			data_ref_types[dataref] = xplmType_Float;
		else if (dataref_type & xplmType_FloatArray)
			data_ref_types[dataref] = xplmType_FloatArray;
		else if (dataref_type & xplmType_IntArray)
			data_ref_types[dataref] = xplmType_IntArray;
		else
			data_ref_types[dataref] = dataref_type;
	}
	return data_ref_types[dataref];
}

int LuaHelper::do_flight_loop()
{
	if (!flight_loop_defined || !lua_enabled)
		return EXIT_FAILURE;

	if (!lua_getglobal(lua, "flight_loop"))
	{
		Logger(TLogLevel::logINFO) << "Lua : no flight_loop function defined" << std::endl;
		flight_loop_defined = false;
		return EXIT_FAILURE;
	}
	std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() - last_flight_loop_call;
	lua_pushnumber(lua, elapsed_seconds.count());

	if (lua_pcall(lua, 1, 0, 0) != LUA_OK)
	{
		lua_enabled = false;
		Logger(TLogLevel::logERROR) << "Lua error in: flight_loop" << std::endl;
		Logger(TLogLevel::logERROR) << "Disable LUA module. fix error in lua script and reload the plugin" << std::endl;
		return EXIT_FAILURE;
	}
	lua_pop(lua, 1);

	return EXIT_SUCCESS;
}

int LuaHelper::do_string(std::string lua_str)
{
	if (!lua_enabled)
		return EXIT_FAILURE;

	if (luaL_dostring(lua, lua_str.c_str()) != LUA_OK)
	{
		Logger(TLogLevel::logERROR) << "Lua error in: " << lua_str << std::endl;
		Logger(TLogLevel::logERROR) << lua_tostring(lua, -1) << std::endl;
		lua_pop(lua, 1);
		Logger(TLogLevel::logERROR) << "LUA module is disabled. Fix error in lua script and reload the plugin" << std::endl;
		lua_enabled = false;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int LuaHelper::do_string(std::string lua_str, double& ret_value)
{
	if (do_string(lua_str) == EXIT_FAILURE)
		return EXIT_FAILURE;
	
	ret_value = lua_tonumber(lua, -1);
	lua_pop(lua, 1);
	return EXIT_SUCCESS;
}

int LuaHelper::do_string(std::string lua_str, std::string& ret_value)
{
	if (do_string(lua_str) == EXIT_FAILURE)
		return EXIT_FAILURE;

	const char* buffer = lua_tostring(lua, -1);
	ret_value = buffer;
	lua_pop(lua, 1);
	return EXIT_SUCCESS;
}
