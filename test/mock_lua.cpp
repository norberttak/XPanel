#include "lua.hpp"
#include <iostream>
#include <cstdlib>

extern lua_State* luaL_newstate()
{
	return NULL;
}

extern void luaL_openlibs(lua_State* L)
{

}

extern const char* lua_pushstring(lua_State* L, const char* s)
{
	return s;
}

extern void lua_setglobal(lua_State* L, const char* name)
{

}

extern void lua_close(lua_State* L)
{

}