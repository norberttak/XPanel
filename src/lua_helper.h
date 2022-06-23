#pragma once
#include <chrono>
#include <ctime>
#include <string>
#include <map>
#include "XPLMPlugin.h"
#include "XPLMUtilities.h"
#include "XPLMDataAccess.h"
#include "XPLMMenus.h"

class LuaHelper
{
public:
	static LuaHelper* get_instace();
	int init();
	void close();
	void push_global_string(std::string name, std::string value);
	int load_script_file(std::string file_name);
	int do_flight_loop();
	int do_string(std::string lua_str);
	XPLMCommandRef get_commandref(std::string commandref_str);
	XPLMDataRef get_dataref(std::string dataref_str);
	XPLMDataTypeID get_dataref_type(XPLMDataRef dataref);
private:
	static LuaHelper* instance;
	std::map<std::string,XPLMCommandRef> command_refs;
	std::map<std::string, XPLMDataRef> data_refs;
	std::map<XPLMDataRef, XPLMDataTypeID> data_ref_types;
	bool flight_loop_defined;
	std::chrono::system_clock::time_point last_flight_loop_call;
	LuaHelper();
};

