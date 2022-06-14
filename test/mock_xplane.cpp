#include "XPLMGraphics.h"
#include "XPLMPlanes.h"
#include "XPLMDisplay.h"
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"
#include "XPLMMenus.h"
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <queue>
#include "configuration.h"

std::map<std::string, int> internal_dataref;
std::map<std::string, int> internal_command_ref;

extern XPLMDataTypeID XPLMGetDataRefTypes(XPLMDataRef inDataRef)
{
	return xplmType_Int;
}

extern void XPLMDebugString(const char* inString)
{
	std::ofstream flog("test_log.txt", std::ofstream::app);
	flog << inString;
	flog.close();
}

extern XPLMDataRef XPLMFindDataRef(const char* datarefstr)
{
	if (internal_dataref.find(datarefstr) == internal_dataref.end())
		internal_dataref[datarefstr] = 0;

	return (XPLMDataRef)&internal_dataref[datarefstr];
}

extern void XPLMUnregisterDataAccessor(XPLMDataRef dataref)
{

}

extern void XPLMSetDatai(XPLMDataRef dataref, int inValue)
{
	*(int*)dataref = inValue;
}

extern void XPLMSetDataf(XPLMDataRef dataref, float inValue)
{
	*(int*)dataref = (int)inValue;
}

extern float XPLMGetDataf(XPLMDataRef dataref)
{
	int val_int = *(int*)dataref;
	return (float)val_int;
}

extern double XPLMGetDatad(XPLMDataRef dataref)
{
	int val_int = *(int*)dataref;
	return (double)val_int;
}

extern int XPLMGetDatai(XPLMDataRef dataref)
{
	return *(int*)dataref;
}

extern void XPLMSetDatavi(XPLMDataRef dataref, int* inValues, int inOffset, int inCount)
{
	*(int*)dataref = *inValues;
}

int test_get_dataref_value(const char* datarefstr)
{
	return internal_dataref[datarefstr];
}

extern XPLMCommandRef XPLMFindCommand(const char* commandref)
{
	if (internal_command_ref.find(commandref) == internal_command_ref.end())
		internal_command_ref[commandref] = 0;

	return (XPLMCommandRef)&internal_command_ref[commandref];
}

std::queue<std::string> command_queue;

std::string test_get_last_command()
{
	if (command_queue.empty())
		return "EMPTY";

	std::string cmd_str = command_queue.front();
	command_queue.pop();
	return cmd_str;
}

int test_get_command_queue_size()
{
	return command_queue.size();
}

extern void XPLMCommandBegin(XPLMCommandRef command_ref)
{
	for (std::map<std::string, int>::iterator it = internal_command_ref.begin(); it != internal_command_ref.end(); ++it)
	{
		if (&it->second == (int*)command_ref)
		{
			command_queue.push(it->first + "_BEGIN");
			break;
		}
	}
}

extern void XPLMCommandEnd(XPLMCommandRef command_ref)
{
	for (std::map<std::string, int>::iterator it = internal_command_ref.begin(); it != internal_command_ref.end(); ++it)
	{
		if (&it->second == (int*)command_ref)
		{
			command_queue.push(it->first + "_END");
			break;
		}
	}
}

extern void XPLMCommandOnce(XPLMCommandRef command_ref)
{
	for (std::map<std::string, int>::iterator it = internal_command_ref.begin(); it != internal_command_ref.end(); ++it)
	{
		if (&it->second == (int*)command_ref)
		{
			command_queue.push(it->first + "_ONCE");
			break;
		}
	}
}

void test_flight_loop(std::vector<Configuration> config)
{
	// process button light changes
	for (auto it : config)
	{
		for (auto triggers : it.light_triggers)
		{
			for (auto trigger : triggers.second)
			{
				trigger->evaluate_and_store_action();
			}
		}

		for (auto display : it.multi_displays)
		{
			display.second->evaluate_and_store_dataref_value();
		}
	}

	// process button push/release events
	ActionQueue::get_instance()->activate_actions_in_queue();
}