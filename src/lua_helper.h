#pragma once
#include <chrono>
#include <ctime>
#include <string>

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
private:
	static LuaHelper* instance;
	bool flight_loop_defined;
	std::chrono::system_clock::time_point last_flight_loop_call;
	LuaHelper();
};

