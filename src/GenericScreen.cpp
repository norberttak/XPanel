/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <math.h>
#include "Logger.h"
#include "LuaHelper.h"
#include "GenericScreen.h"

GenericScreen::GenericScreen()
{
	screen_action_queue.clear();
}

GenericScreen::~GenericScreen()
{
	for (auto &action : screen_action_queue)
	{
		delete action;
	}
	screen_action_queue.clear();
}

void GenericScreen::add_screen_action(ScreenAction* screen_action)
{
	screen_action_queue.push_back(screen_action);
}

int GenericScreen::read_data_ref_int(XPLMDataRef data_ref, XPLMDataTypeID data_ref_type)
{
	int value_int = 0;
	switch (data_ref_type)
	{
	case xplmType_Int:
		value_int = XPLMGetDatai(data_ref);
		break;
	case xplmType_Float:
		value_int = (int)round(XPLMGetDataf(data_ref));
		break;
	case xplmType_Double:
		value_int = (int)round(XPLMGetDatad(data_ref));
		break;
	default:
		Logger(logWARNING) << "Generic screen: Unknown dataref_type" << std::endl;
	}
	return value_int;
}

int GenericScreen::read_data_ref_int(XPLMDataRef data_ref, XPLMDataTypeID data_ref_type, int index)
{
	int value_int = 0;
	float value_float = 0;

	switch (data_ref_type)
	{
	case xplmType_IntArray:
		XPLMGetDatavi(data_ref, &value_int, index, 1);
		break;
	case xplmType_FloatArray:
		XPLMGetDatavf(data_ref, &value_float, index, 1);
		value_int = (int)value_float;
		break;
	default:
		Logger(logWARNING) << "Generic screen: Unknown dataref_type" << std::endl;
	}
	return value_int;
}

std::string GenericScreen::read_dataref_str(XPLMDataRef data_ref)
{
	char str_buffer[32];
	XPLMGetDatab(data_ref, str_buffer, 0, 32);

	std::string result;
	result = str_buffer;

	return result;
}