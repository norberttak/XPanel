#include <limits>
#include "multi_purpose_display.h"
#include "lua_helper.h"
#include "logger.h"

MultiPurposeDisplay::MultiPurposeDisplay()
{
	active_condition = "";
	display_value = 0;
	display_value_old = display_value;
	display_value_changed = false;
	nr_of_bytes = 5;
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