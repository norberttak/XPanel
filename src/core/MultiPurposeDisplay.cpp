/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <cfloat>
#include <limits>
#include "MultiPurposeDisplay.h"
#include "LuaHelper.h"
#include "Logger.h"

MultiPurposeDisplay::MultiPurposeDisplay():
	GenericDisplay()
{
	active_condition = "";
	display_value = 0;
	display_value_old = display_value;
	display_value_changed = false;
	nr_of_bytes = 5;
}

MultiPurposeDisplay::MultiPurposeDisplay(MultiPurposeDisplay* other)
{
	active_condition = other->active_condition;
	display_value = other->display_value;
	display_value_old = other->display_value_old;
	display_value_changed = other->display_value_changed;
	nr_of_bytes = other->nr_of_bytes;
	turn_off = other->turn_off;

	conditions = other->conditions;
	const_values = other->const_values;
	lua_functions = other->lua_functions;
	data_ref_types = other->data_ref_types;
	minimum_number_of_digits = other->minimum_number_of_digits;
	blank_leading_zeros = other->blank_leading_zeros;
	minimum_number_of_digits_for_condtions = other->minimum_number_of_digits_for_condtions;
}

void MultiPurposeDisplay::add_condition(std::string selector_sw_name, XPLMDataRef data)
{
	if (conditions.find(selector_sw_name) != conditions.end())
	{
		Logger(TLogLevel::logWARNING) << "MultiPurposeDisplay: selector switch (" << selector_sw_name << ") already registered. Overwrite it." << std::endl;
	}
	conditions[selector_sw_name] = data;
	data_ref_types[selector_sw_name] = XPLMGetDataRefTypes(data);
}

void MultiPurposeDisplay::add_condition(std::string selector_sw_name, double const_value)
{
	if (conditions.find(selector_sw_name) != conditions.end())
	{
		Logger(TLogLevel::logWARNING) << "MultiPurposeDisplay: selector switch (" << selector_sw_name << ") already registered. Overwrite it." << std::endl;
	}
	const_values[selector_sw_name] = const_value;
	data_ref_types[selector_sw_name] = xplmType_Unknown;
}

void MultiPurposeDisplay::add_condition(std::string selector_sw_name, std::string lua_str)
{
	if (conditions.find(selector_sw_name) != conditions.end())
	{
		Logger(TLogLevel::logWARNING) << "MultiPurposeDisplay: selector switch (" << selector_sw_name << ") already registered. Overwrite it." << std::endl;
	}

	lua_functions[selector_sw_name] = lua_str;
	data_ref_types[selector_sw_name] = xplmType_Unknown;
}

void MultiPurposeDisplay::set_condition_active(std::string selector_sw_name)
{
	if (conditions.find(selector_sw_name) == conditions.end() &&
		const_values.find(selector_sw_name) == const_values.end() &&
		lua_functions.find(selector_sw_name) == lua_functions.end())
	{
		Logger(TLogLevel::logERROR) << "MultiPurposeDisplay: unregistered selector switch: " << selector_sw_name << std::endl;
		return;
	}
	guard.lock();
	active_condition = selector_sw_name;
	guard.unlock();
}

bool MultiPurposeDisplay::is_registered_selector(std::string selector_sw_name)
{
	bool _registered;
	guard.lock();
	_registered = (conditions.count(selector_sw_name) != 0 ||
		const_values.count(selector_sw_name) != 0 ||
		lua_functions.count(selector_sw_name) != 0);
	guard.unlock();
	Logger(TLogLevel::logTRACE) << "MultiPurposeDisplay:is_registered_selector:" << selector_sw_name << " " << _registered << std::endl;
	return _registered;
}

void MultiPurposeDisplay::set_minimum_number_of_digits(std::string _condition, int _minimum_number_of_digits)
{
	minimum_number_of_digits_for_condtions[_condition] = _minimum_number_of_digits;
	Logger(TLogLevel::logDEBUG) << "MultiPurposeDisplay: set minimum number of digits [" + _condition + "]: " << _minimum_number_of_digits << std::endl;
}

int MultiPurposeDisplay::get_minimum_number_of_digits()
{
	int result = 1;

	guard.lock();
	
	if (minimum_number_of_digits_for_condtions.count(active_condition) != 0)
		result = minimum_number_of_digits_for_condtions[active_condition];
	else
		result = 1; //default 1

	guard.unlock();

	return result;
}

/* call this function only from XPlane flight loop */
void MultiPurposeDisplay::evaluate_and_store_dataref_value()
{
	guard.lock();

	if (conditions.count(active_condition) != 0 && data_ref_types[active_condition] != 0)
	{
		condition = conditions[active_condition];
		data_ref_type = data_ref_types[active_condition];
	}
	else
	{
		condition = NULL;
		data_ref_type = xplmType_Unknown;
	}

	if (lua_functions.count(active_condition))
		lua_function = lua_functions[active_condition];
	else
		lua_function = "";

	if (const_values.count(active_condition))
		const_value = const_values[active_condition];
	else
		const_value = DBL_MIN;

	guard.unlock();

	GenericDisplay::evaluate_and_store_dataref_value();

	if (abs(display_value - display_value_old) >= 0.001)
		display_value_changed = true;

	display_value_old = display_value;
}
