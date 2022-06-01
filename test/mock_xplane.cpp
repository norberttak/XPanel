#include "XPLMGraphics.h"
#include "XPLMPlanes.h"
#include "XPLMDisplay.h"
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"
#include "XPLMMenus.h"
#include <string>
#include <map>
#include <queue>

std::map<std::string, int> internal_dataref;
std::map<std::string, int> internal_command_ref;

extern void XPLMDebugString(const char* inString)
{
	printf(inString);
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