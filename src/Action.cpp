#include "Action.h"

Action::Action()
{
	data = 0;
	lua_str = "";
	dataref = NULL;
}

Action::Action(XPLMCommandRef cmd, CommandType type)
{
	commandref = cmd;
	command_type = type;
}

Action::Action(XPLMDataRef dat, int d)
{
	dataref = dat;
	data = d;
}

Action::Action(std::string lua_str)
{
	lua_str = lua_str;
}

Action::~Action()
{
	if (dataref != NULL)
		XPLMUnregisterDataAccessor(dataref);
	dataref = NULL;
}

void Action::activate() 
{
	if (dataref != NULL) 
	{
		XPLMSetDatai(dataref, data);
	}

	if (commandref != NULL)
	{
		switch (command_type) {
			case CommandType::BEGIN:
				XPLMCommandBegin(commandref);
				break;

			case CommandType::END:
				XPLMCommandEnd(commandref);
				break;

			case CommandType::ONCE:
				XPLMCommandOnce(commandref);
				break;		
		}
	}

	if (!lua_str.empty())
	{
		//
	}
}