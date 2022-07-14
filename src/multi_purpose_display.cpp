#include <limits>
#include "multi_purpose_display.h"
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

void MultiPurposeDisplay::set_condition_active(std::string selector_sw_name)
{
	if (conditions.find(selector_sw_name) == conditions.end())
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
	_registered = (conditions.count(selector_sw_name) != 0);
	guard.unlock();
	Logger(TLogLevel::logTRACE) << "MultiPurposeDisplay:is_registered_selector:" << selector_sw_name << " " << _registered << std::endl;
	return _registered;
}

bool MultiPurposeDisplay::get_decimal_components(int number, int max_dec_pos, unsigned char* buffer)
{
	if (number >= pow(10, max_dec_pos + 1))
		return false;

	int remain = number;
	for (int dec_pos = max_dec_pos; dec_pos >= 0; dec_pos--)
	{
		buffer[max_dec_pos - dec_pos] = remain / (int)pow(10, dec_pos);
		remain = remain % (int)pow(10, dec_pos);
	}
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
	if (conditions.count(active_condition) != 0)
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
			//return;
			break;
		}
	}
	else
	{
		turn_off = true;
	}
	guard.unlock();

	if (abs(display_value - display_value_old) >= 0.001)
		display_value_changed = true;

	display_value_old = display_value;
}