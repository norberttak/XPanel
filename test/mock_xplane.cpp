/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "XPLMGraphics.h"
#include "XPLMPlanes.h"
#include "XPLMPlugin.h"
#include "XPLMDisplay.h"
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"
#include "XPLMMenus.h"
#include "XPLMProcessing.h"
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <queue>
#include <list>
#include <set>
#include "core/Configuration.h"
#include "fip/FIPScreen.h"
#include "core/Logger.h"

#ifndef WIN32
#include <cstring>
#include <algorithm>

// Portable stand-ins for the MS CRT secure string functions used below.
template <size_t N>
static inline void strcpy_s(char (&dest)[N], const char* src)
{
	strncpy(dest, src, N - 1);
	dest[N - 1] = '\0';
}

static inline void strcpy_s(char* dest, size_t dest_size, const char* src)
{
	strncpy(dest, src, dest_size - 1);
	dest[dest_size - 1] = '\0';
}

static inline void strncpy_s(char* dest, size_t dest_size, const char* src, size_t count)
{
	size_t n = std::min(count, dest_size - 1);
	strncpy(dest, src, n);
	dest[n] = '\0';
}
#endif

std::map<std::string, int> internal_dataref;
std::map<std::string, int> internal_command_ref;

// Names in this set make XPLMFindDataRef/XPLMFindCommand report "not found" (NULL)
// regardless of whether they were previously looked up, so tests can simulate a
// dataref/commandref that hasn't appeared yet.
std::set<std::string> test_unavailable_names;

void test_set_name_unavailable(const std::string& name)
{
	test_unavailable_names.insert(name);
}

void test_set_name_available(const std::string& name)
{
	test_unavailable_names.erase(name);
}

XPLMFlightLoop_f registered_flight_loop;
XPLMMenuHandler_f menu_handler = NULL;

char test_aircraft_file_name[256];
char test_aircraft_path[512];
void test_set_aircraft_path_and_filename(char* file_name, char* path)
{
	strcpy_s(test_aircraft_file_name, file_name);
	strcpy_s(test_aircraft_path, path);
}

extern void XPLMGetNthAircraftModel(int inIndex, char *outFileName, char *outPath)
{
	(void)inIndex;
	strcpy_s(outFileName, 256, test_aircraft_file_name);
	strcpy_s(outPath, 512, test_aircraft_path);
}

extern XPLMPluginID XPLMFindPluginBySignature(const char* inSignature)
{
	(void)inSignature;
	return (XPLMPluginID)0;
}

extern void XPLMGetPluginInfo(XPLMPluginID inPlugin, char* outName, char* outFilePath, char* outSignature, char* outDescription)
{
	(void)inPlugin;
	if (outName)
		strcpy_s(outName, 16, "XPanel ver " PLUGIN_VERSION);
	
	if (outFilePath)
		strcpy_s(outFilePath, 16, "./");
	
	if (outSignature)
		strcpy_s(outSignature, 16, PLUGIN_SIGNATURE);

	if (outDescription)
		strcpy_s(outDescription, 16, "xpanel");
}

extern XPLMMenuID XPLMFindPluginsMenu()
{
	return NULL;
}

extern int XPLMAppendMenuItem(XPLMMenuID inMenu, const char* inItemName, void* inItemRef, int inDeprecated)
{
	(void)inMenu;
	(void)inItemName;
	(void)inItemRef;
	(void)inDeprecated;
	return 0;
}

extern XPLMMenuID XPLMCreateMenu(const char* inName, XPLMMenuID inParentMenu, int inParentItem, XPLMMenuHandler_f inHandler, void* inMenuRef)
{
	(void)inName;
	(void)inParentMenu;
	(void)inParentItem;
	(void)inMenuRef;
	menu_handler = inHandler;
	return 0;
}

extern void XPLMDestroyMenu(XPLMMenuID inMenuID)
{
	(void)inMenuID;
}

void test_call_menu_handler(XPLMMenuID inMenuID, void* menuRef, void* itemRef)
{
	(void)inMenuID;
	if (menu_handler != NULL)
		(*menu_handler)(menuRef, itemRef);
}

extern XPLMDataTypeID XPLMGetDataRefTypes(XPLMDataRef inDataRef)
{
	(void)inDataRef;
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
	if (test_unavailable_names.count(datarefstr) > 0)
		return NULL;

	if (internal_dataref.find(datarefstr) == internal_dataref.end())
		internal_dataref[datarefstr] = 0;

	Logger(TLogLevel::logTRACE) << "TEST " << "XPLMFindDataRef " << datarefstr << ":" << (XPLMDataRef)&internal_dataref[datarefstr] << std::endl;
	return (XPLMDataRef)&internal_dataref[datarefstr];
}

extern void XPLMUnregisterDataAccessor(XPLMDataRef dataref)
{
	(void)dataref;
}

extern void XPLMSetDatai(XPLMDataRef dataref, int inValue)
{
	Logger(TLogLevel::logTRACE) << "TEST " << "XPLMSetDatai " << dataref << "=" << inValue << std::endl;
	*(int*)dataref = inValue;
}

extern void XPLMSetDatad(XPLMDataRef dataref, double inValue)
{
	Logger(TLogLevel::logTRACE) << "TEST " << "XPLMSetDatad " << dataref << "=" << inValue << std::endl;
	*(int*)dataref = (int)inValue;
}

extern void XPLMSetDataf(XPLMDataRef dataref, float inValue)
{
	Logger(TLogLevel::logTRACE) << "TEST " << "XPLMSetDataf " << dataref << "=" << inValue << std::endl;
	*(int*)dataref = (int)inValue;
}

extern float XPLMGetDataf(XPLMDataRef dataref)
{
	int val_int = *(int*)dataref;
	Logger(TLogLevel::logTRACE) << "TEST " << "XPLMGetDataf " << dataref << "=" << val_int << std::endl;
	return (float)val_int;
}

extern double XPLMGetDatad(XPLMDataRef dataref)
{
	int val_int = *(int*)dataref;
	Logger(TLogLevel::logTRACE) << "TEST " << "XPLMGetDatad " << dataref << "=" << val_int << std::endl;
	return (double)val_int;
}

extern int XPLMGetDatavi(XPLMDataRef dataref, int* inValues, int inOffset, int inCount)
{
	(void)inValues;
	(void)inOffset;
	(void)inCount;
	int val_int = *(int*)dataref;
	Logger(TLogLevel::logTRACE) << "TEST " << "XPLMGetDatavi " << dataref << "=" << val_int << std::endl;
	return (double)val_int;
}

extern int XPLMGetDatavd(XPLMDataRef dataref, double* inValues, int inOffset, int inCount)
{
	(void)inValues;
	(void)inOffset;
	(void)inCount;
	int val_int = *(int*)dataref;
	Logger(TLogLevel::logTRACE) << "TEST " << "XPLMGetDatavd " << dataref << "=" << val_int << std::endl;
	return (double)val_int;
}

extern int XPLMGetDatavf(XPLMDataRef dataref, float* inValues, int inOffset, int inCount)
{
	(void)inValues;
	(void)inOffset;
	(void)inCount;
	int val_int = *(int*)dataref;
	Logger(TLogLevel::logTRACE) << "TEST " << "XPLMGetDatavf " << dataref << "=" << val_int << std::endl;
	return (double)val_int;
}

extern int XPLMGetDatai(XPLMDataRef dataref)
{
	Logger(TLogLevel::logTRACE) << "TEST " << "XPLMGetDatai " << dataref << "=" << *(int*)dataref << std::endl;
	return *(int*)dataref;
}

extern int XPLMGetDatab(XPLMDataRef dataref, void* outValue, int inOffset, int inMaxBytes)
{
	(void)inOffset;
	strncpy_s((char*)outValue, strlen((char*)dataref), (char*)dataref, inMaxBytes);
	return 0;
}

extern void XPLMSetDatavi(XPLMDataRef dataref, int* inValues, int inOffset, int inCount)
{
	(void)inOffset;
	(void)inCount;
	*(int*)dataref = *inValues;
}

extern void XPLMSetDatavf(XPLMDataRef dataref, float* inValues, int inOffset, int inCount)
{
	(void)inOffset;
	(void)inCount;
	*(int*)dataref = (int)*inValues;
}

int test_get_dataref_value(const char* datarefstr)
{
	return internal_dataref[datarefstr];
}

void test_set_dataref_value(const char* datarefstr, int value)
{
	internal_dataref[datarefstr] = value;
}

extern XPLMCommandRef XPLMFindCommand(const char* commandref)
{
	if (test_unavailable_names.count(commandref) > 0)
		return NULL;

	if (internal_command_ref.find(commandref) == internal_command_ref.end())
		internal_command_ref[commandref] = 0;

	return (XPLMCommandRef)&internal_command_ref[commandref];
}

std::list<std::string> command_queue;

void test_clear_command_queue()
{
	command_queue.clear();
}

std::string test_get_last_command()
{
	if (command_queue.empty())
		return "EMPTY";

	std::string cmd_str = command_queue.front();
	command_queue.pop_front();
	return cmd_str;
}

bool test_is_command_in_queue(std::string cmd_str)
{
	bool cmd_found = false;
	
	while (!command_queue.empty() && !cmd_found)
	{
		if (command_queue.front() == cmd_str)
			cmd_found = true;

		command_queue.pop_front();
	}
	return cmd_found;
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
			command_queue.push_back(it->first + "_BEGIN");
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
			command_queue.push_back(it->first + "_END");
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
			command_queue.push_back(it->first + "_ONCE");
			break;
		}
	}
}

extern void XPLMRegisterFlightLoopCallback(XPLMFlightLoop_f inFlightLoop, float inInterval, void* inRefcon)
{
	(void)inInterval;
	(void)inRefcon;
	registered_flight_loop = inFlightLoop;
}

extern void XPLMUnregisterFlightLoopCallback(XPLMFlightLoop_f inFlightLoop, void* inRefcon)
{
	(void)inRefcon;
	// Only clear the tracked callback if it's the one being unregistered - otherwise
	// unregistering a short-lived callback (e.g. wait_for_available_callback) after a
	// different, still-active callback (e.g. flight_loop_callback) has already been
	// registered would incorrectly wipe out that still-active registration.
	if (registered_flight_loop == inFlightLoop)
		registered_flight_loop = NULL;
}

void test_call_registered_flight_loop()
{
	if (registered_flight_loop != NULL)
		(*registered_flight_loop)(1, 1, 1, NULL);
}

void test_flight_loop(std::vector<ClassConfiguration> &config)
{
	// process button light changes
	for (auto& it : config)
	{
		for (auto& triggers : it.light_triggers)
		{
			for (auto& trigger : triggers.second)
			{
				trigger->evaluate_and_store_action();
			}
		}

		for (auto& display : it.multi_displays)
		{
			display.second->evaluate_and_store_dataref_value();
		}

		for (auto& display : it.generic_displays)
		{
			display.second->evaluate_and_store_dataref_value();
		}

		for (auto& fip_screen : it.fip_screens)
		{
			fip_screen.second->evaluate_and_store_screen_action();
		}
	}

	// process button push/release events
	ActionQueue::get_instance()->activate_actions_in_queue();
}

void test_flight_loop(Device* dev)
{
	for (const auto& triggers : dev->get_config().light_triggers)
	{
		for (auto& trigger : triggers.second)
		{
			trigger->evaluate_and_store_action();
		}
	}

	// check and set 7 segment display states
	for (const auto& display : dev->get_config().multi_displays)
	{
		display.second->evaluate_and_store_dataref_value();
	}

	for (const auto& display : dev->get_config().generic_displays)
	{
		display.second->evaluate_and_store_dataref_value();
	}

	// update the FIP devices
	for (auto& screen : dev->get_config().fip_screens)
	{
		screen.second->evaluate_and_store_screen_action();
	}

	// process button push/release events
	ActionQueue::get_instance()->activate_actions_in_queue();
}