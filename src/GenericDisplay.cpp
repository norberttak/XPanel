/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <cmath>
#include <cfloat>
#include "GenericDisplay.h"
#include "LuaHelper.h"
#include "Logger.h"

GenericDisplay::GenericDisplay(bool _use_bcd)
{
	use_bcd = _use_bcd;
	display_value = 0;
	display_value_old = display_value;
	display_value_changed = false;
	data_ref_type = xplmType_Unknown;
	lua_function = "";
	const_value = DBL_MIN;
}

GenericDisplay::GenericDisplay():GenericDisplay(true)
{
	//
}

void GenericDisplay::set_nr_bytes(int _nr_of_bytes)
{
	nr_of_bytes = _nr_of_bytes;
}

void GenericDisplay::add_dataref(XPLMDataRef _data_ref)
{
	condition = _data_ref;
	data_ref_type = XPLMGetDataRefTypes(_data_ref);
}

void GenericDisplay::add_const(double _const_value)
{
	const_value = _const_value;
	data_ref_type = xplmType_Unknown;
}

void GenericDisplay::add_lua(std::string _lua_function)
{
	lua_function = _lua_function;
	data_ref_type = xplmType_Unknown;
}

/* call this function only from XPlane flight loop */
void GenericDisplay::evaluate_and_store_dataref_value()
{
	guard.lock();
	switch (data_ref_type) {
	case xplmType_Int:
		display_value = (double)XPLMGetDatai(condition);
		break;
	case xplmType_Float:
		display_value = (double)XPLMGetDataf(condition);
		break;
	case xplmType_Double:
		display_value = XPLMGetDatad(condition);
		break;
	default:
		if (const_value > DBL_MIN)
			display_value = const_value;
		else if (!lua_function.empty())
			display_value = LuaHelper::get_instace()->do_string("return " + lua_function);
		break;
	}
	guard.unlock();

	if (abs(display_value - display_value_old) >= 0.001)
		display_value_changed = true;

	display_value_old = display_value;
}

bool GenericDisplay::get_decimal_components(int number, unsigned char* buffer)
{
	bool negative = false;
	if (number < 0)
	{
		number = abs(number);
		negative = true;
	}

	if (number > pow(10, nr_of_bytes))
		return false;

	int remain = number;
	for (int dec_pos = nr_of_bytes - 1; dec_pos >= 0; dec_pos--)
	{
		buffer[nr_of_bytes - 1 - dec_pos] = remain / (int)pow(10, dec_pos);
		remain = remain % (int)pow(10, dec_pos);
	}

	if (negative)
		buffer[0] = 0xfe; // minus sign

	return true;
}

bool GenericDisplay::get_binary_components(int number, unsigned char* buffer)
{
	bool negative = false;
	if (number < 0)
	{
		number = abs(number);
		negative = true;
	}

	if (number > pow(2, 8*nr_of_bytes))
		return false;

	for (int i = 0; i < nr_of_bytes; i++)
	{
		buffer[i] = number & 0xFF;
		number = number >> 8;
	}

	if (negative)
		buffer[nr_of_bytes - 1] |= 0x80; // set the MSB to 1 -> negative number

	return true;
}

// called from UsbHidDevice worker thread
bool GenericDisplay::get_display_value(unsigned char* buffer)
{
	if (!display_value_changed)
		return false;

	guard.lock();
	double _val = display_value;
	display_value_changed = false;
	guard.unlock();

	Logger(TLogLevel::logTRACE) << "GenericDisplay: get_display_value: " << _val << std::endl;
	return use_bcd ? get_decimal_components(_val, buffer) : get_binary_components(int(_val), buffer);
}
