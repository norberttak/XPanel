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
	nr_of_digits = 5;
}

void MultiPurposeDisplay::set_nr_digits(int _nr_of_digits)
{
	nr_of_digits = _nr_of_digits;
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

bool MultiPurposeDisplay::get_decimal_components(int number, int max_dec_pos, unsigned char* buffer)
{
	bool negative = false;
	if (number < 0)
	{
		number = abs(number);
		negative = true;
	}

	if (number >= pow(10, max_dec_pos + 1))
		return false;

	int remain = number;
	for (int dec_pos = max_dec_pos; dec_pos >= 0; dec_pos--)
	{
		buffer[max_dec_pos - dec_pos] = remain / (int)pow(10, dec_pos);
		remain = remain % (int)pow(10, dec_pos);
	}

	if (negative)
		buffer[0] = 0xfe; // minus sign

	return true;
}

bool MultiPurposeDisplay::get_display_value(unsigned char* digits)
{
	if (turn_off)
	{
		memset(digits, 0xff, nr_of_digits); // 0xff means turn off all segments
		return true;
	}

	if (!display_value_changed)
		return false;

	guard.lock();
	double _val = display_value;
	display_value_changed = false;
	guard.unlock();

	Logger(TLogLevel::logTRACE) << "MultiPurposeDisplay: get_display_value: " << _val << std::endl;
	return get_decimal_components(_val, nr_of_digits - 1, digits);
}

/* call this function only from XPlane flight loop */
void MultiPurposeDisplay::evaluate_and_store_dataref_value()
{
	guard.lock();
	if (conditions.count(active_condition) != 0 ||
		const_values.count(active_condition) != 0 ||
		lua_functions.count(active_condition) != 0)
	{
		turn_off = false;

		switch (data_ref_types[active_condition]) {
		case xplmType_Int:
			display_value = (double)XPLMGetDatai(conditions[active_condition]);
			break;
		case xplmType_Float:
			display_value = (double)XPLMGetDataf(conditions[active_condition]);
			break;
		case xplmType_Double:
			display_value = XPLMGetDatad(conditions[active_condition]);
			break;
		default:
			if (const_values.count(active_condition) != 0)
				display_value = const_values[active_condition];
			else if (lua_functions.count(active_condition) != 0)
				display_value = LuaHelper::get_instace()->do_string("return " + lua_functions[active_condition]);

			break;
		}
	}
	else
	{
		turn_off = true;
		display_value_changed = true;
	}
	guard.unlock();

	if (abs(display_value - display_value_old) >= 0.001)
		display_value_changed = true;

	display_value_old = display_value;
}