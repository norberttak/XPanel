#pragma once
#include <string>
#include "XPLMDataAccess.h"
#include "XPLMMenus.h"

typedef enum 
{
	BEGIN,
	END,
	ONCE,
	NONE
} CommandType;

class Action
{
public:
	Action();
	Action(XPLMDataRef dat, int d);
	Action(XPLMCommandRef cmd, CommandType type);
	Action(std::string lua_str);
	~Action();
	void activate();
private:
	int data = 0;
	std::string lua_str = "";
	XPLMDataRef dataref = NULL;
	CommandType command_type = CommandType::NONE;
	XPLMCommandRef commandref = NULL;
};


