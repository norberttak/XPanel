#include "lua_helper.h"
#include "lua.hpp"
#include "logger.h"

LuaHelper* LuaHelper::instance = NULL;
lua_State* lua = NULL;

extern "C" {
#define LUA_FUNC_NAME_COMMAND_ONCE "command_once"
#define LUA_FUNC_NAME_COMMAND_BEGIN "command_begin"
#define LUA_FUNC_NAME_COMMAND_END "command_end"
#define LUA_FUNC_NAME_GET_DATAREF "get_dataref"
#define LUA_FUNC_NAME_SET_DATAREF "set_dataref"
#define LUA_FUNC_NAME_LOG   "log_msg"

    static int LuaCommand(lua_State* L, const char* command_name)
    {
        Logger(TLogLevel::logTRACE) << "Lua XP command call:" << command_name << "(" << lua_tostring(L, 1) << ")" << std::endl;

        if (!lua_isstring(L, 1))
        {
            Logger(TLogLevel::logERROR) << "Lua command: invalid parameter. not a string?" << std::endl;
            return 0;
        }

        char dataref_str[1024];
        strncpy_s(dataref_str, lua_tostring(L, 1), sizeof(dataref_str));
        XPLMCommandRef CommandId = XPLMFindCommand(dataref_str);
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
        char dataref_str[1024];
        int  dataref_array_index = 0;

        if (!lua_isstring(L, 1))
        {
            Logger(TLogLevel::logERROR) << "Lua get: invalid parameter. not a string?" << std::endl;
            return 0;
        }
        strncpy_s(dataref_str, lua_tostring(L, 1), sizeof(dataref_str));

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
        char dataref_str[1024];

        if (!(lua_isstring(L, 1) && lua_isnumber(L, 2)))
        {
            Logger(TLogLevel::logERROR) << "lua set: wrong arguments to function set()" << std::endl;
            return 0;
        }
        strncpy_s(dataref_str, lua_tostring(L, 1), sizeof(dataref_str));

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
        char log_level_str[64];
        memset(log_level_str, 0, sizeof(log_level_str));

        char log_msg[1024];
        memset(log_msg, 0, sizeof(log_msg));

        if (!(lua_isstring(L, 1) && lua_isstring(L, 2)))
        {
            Logger(TLogLevel::logERROR) << "lua set: wrong arguments to function log_msg()" << std::endl;
            return 0;
        }
        strncpy_s(log_level_str, lua_tostring(L, 1), sizeof(log_level_str));
        strncpy_s(log_msg, lua_tostring(L, 2), sizeof(log_msg));

        TLogLevel log_level;
        if (strcmp("ERROR", log_level_str) == 0)
            log_level = TLogLevel::logERROR;
        else if (strcmp("WARNING", log_level_str) == 0)
            log_level = TLogLevel::logWARNING;
        else if (strcmp("INFO", log_level_str) == 0)
            log_level = TLogLevel::logINFO;
        else if (strcmp("DEBUG", log_level_str) == 0)
            log_level = TLogLevel::logDEBUG;
        else if (strcmp("TRACE", log_level_str) == 0)
            log_level = TLogLevel::logTRACE;
        else
            log_level = TLogLevel::logTRACE;

        Logger(log_level) << "LUA script: " << log_msg << std::endl;
        return 1;
    }
}

LuaHelper::LuaHelper()
{
	//
}

LuaHelper* LuaHelper::get_instace()
{
	if (instance == NULL)
		instance = new LuaHelper();
	return instance;
}

int LuaHelper::load_script_file(std::string file_name)
{
    if (luaL_dofile(lua, file_name.c_str()))
    {
        Logger(TLogLevel::logERROR) << "Lua: Error during load script " << file_name << " " << lua_tostring(lua, lua_gettop(lua)) << std::endl;
        lua_pop(lua, 1);
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
    
    last_flight_loop_call = std::chrono::system_clock::now();
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
    if (command_refs.count(dataref_str) == 0)
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
        data_ref_types[dataref] = dataref_type;
    }
    return data_ref_types[dataref];
}

int LuaHelper::do_flight_loop()
{
    if (!flight_loop_defined)
        return EXIT_FAILURE;

    if (!lua_getglobal(lua, "flight_loop"))
    {
        Logger(TLogLevel::logINFO) << "Lua : no flight_loop function defined" << std::endl;
        flight_loop_defined = false;
        return EXIT_FAILURE;
    }
    std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() - last_flight_loop_call;
	lua_pushnumber(lua, elapsed_seconds.count());
	lua_pcall(lua, 1, 0, 0);
	lua_pop(lua, 1);

    return EXIT_SUCCESS;
}

double LuaHelper::do_string(std::string lua_str)
{
    Logger(TLogLevel::logTRACE) << "LuaHelper do_string: " << lua_str << std::endl;
    
    if (luaL_dostring(lua, lua_str.c_str()) != 0)
    {
        Logger(TLogLevel::logERROR) << "Lua error in: " << lua_str << std::endl;
        return 0;
    }

    double return_value = lua_tonumber(lua, -1);
    Logger(TLogLevel::logTRACE) << "LuaHelper do_string return value: " << return_value << std::endl;
    return return_value;
}