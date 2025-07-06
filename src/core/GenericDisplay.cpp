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

// MAX_VALUE now defined as constexpr in header

GenericDisplay::GenericDisplay(bool _use_bcd)
{
	use_bcd = _use_bcd;
	display_value = 0;
	display_value_old = display_value;
	display_value_changed = false;
	data_ref_type = xplmType_Unknown;
	lua_function = "";
	const_value = DBL_MIN;
	blank_leading_zeros = true;
	minimum_number_of_digits = 1;
	dot_position = -1; //no dot display
}

GenericDisplay::GenericDisplay(GenericDisplay* other)
{
	display_value = other->display_value;
	display_value_old = other->display_value_old;
	display_value_changed = other->display_value_changed;
	use_bcd = other->use_bcd;
	lua_function = other->lua_function;
	condition = other->condition;
	data_ref_type = other->data_ref_type;
	const_value = other->const_value;
	nr_of_bytes = other->nr_of_bytes;
	minimum_number_of_digits = other->minimum_number_of_digits;
	dot_position = other->dot_position;
	dataref_index = other->dataref_index;
	blank_leading_zeros = other->blank_leading_zeros;
}

GenericDisplay::GenericDisplay():GenericDisplay(true)
{
	//
}

void GenericDisplay::set_nr_bytes(int _nr_of_bytes)
{
	nr_of_bytes = _nr_of_bytes;
}

void GenericDisplay::set_minimum_number_of_digits(int _minimum_number_of_digits)
{
	minimum_number_of_digits = _minimum_number_of_digits;
	Logger(TLogLevel::logDEBUG) << "GenericDisplay: set minimum number of digits: " << _minimum_number_of_digits << std::endl;
}

int GenericDisplay::get_minimum_number_of_digits()
{
	return minimum_number_of_digits;
}

void GenericDisplay::set_dot_position(int _dot_position)
{
	dot_position = _dot_position;
	Logger(TLogLevel::logDEBUG) << "GenericDisplay: set dot position: " << _dot_position << std::endl;
}

int GenericDisplay::get_dot_position()
{
	return dot_position;
}

void GenericDisplay::set_bcd(bool _use_bcd)
{
	use_bcd = _use_bcd;
	Logger(TLogLevel::logDEBUG) << "GenericDisplay: set bcd: " << (_use_bcd ? "yes" : "no") << std::endl;
}

void GenericDisplay::set_blank_leading_zeros(bool _blank_leading_zeros)
{
	blank_leading_zeros = _blank_leading_zeros;
	Logger(TLogLevel::logDEBUG) << "GenericDisplay: set blank leading zero: " << (_blank_leading_zeros ? "yes" : "no") << std::endl;
}

void GenericDisplay::add_dataref(XPLMDataRef _data_ref)
{
	dataref_index = -1;
	condition = _data_ref;
	data_ref_type = XPLMGetDataRefTypes(_data_ref);
}

void GenericDisplay::add_dataref(XPLMDataRef _data_ref, int _dataref_index)
{
	dataref_index = _dataref_index;
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
	case xplmType_IntArray:
		int outValuei;
		XPLMGetDatavi(condition, &outValuei, dataref_index, 1);
		display_value = (double)outValuei;
		break;
	case xplmType_FloatArray:
		float outValuef;
		XPLMGetDatavf(condition, &outValuef, dataref_index, 1);
		display_value = (double)outValuef;
		break;
	default:
		if (const_value > DBL_MIN)
			display_value = const_value;
		else if (!lua_function.empty())
			LuaHelper::get_instace()->do_string("return " + lua_function, display_value);
		break;
	}
	guard.unlock();

	if (abs(display_value - display_value_old) >= 0.001)
		display_value_changed = true;

	display_value_old = display_value;
}

bool GenericDisplay::get_decimal_components(int number, unsigned char* buffer, int _minimum_number_of_digits, int _dot_position)
{
	bool negative = false;
	if (number < 0)
	{
		number = abs(number);
		negative = true;
	}

	for (int i = 0; i < nr_of_bytes; i++)
		buffer[i] = ZERO_CHAR;

	if (number != 0)
	{
		// how many decimal positions do we need?
		const int base_10_of_number = (int)log10(number);

		int remain = number;
		int buffer_index = nr_of_bytes > (base_10_of_number + 1) ? nr_of_bytes - (base_10_of_number+1) : 0;

		for (int dec_pos = base_10_of_number; dec_pos >= 0 && buffer_index < nr_of_bytes; dec_pos--)
		{
			buffer[buffer_index] = remain / (int)pow(10, dec_pos);
			if (dec_pos == _dot_position)
				buffer[buffer_index] += PERIOD_CHAR; // turn on period dot

			remain = remain % (int)pow(10, dec_pos);
			buffer_index++;
		}

		if (remain != 0)
			buffer[nr_of_bytes - 1] += PERIOD_CHAR; // a dot at the end of display indicates the truncated line
	}

	if (blank_leading_zeros)
	{
		for (int i = 0; i < nr_of_bytes - _minimum_number_of_digits; i++)
		{
			if (buffer[i] == ZERO_CHAR)
				buffer[i] = BLANK_CHAR;
			else
				break;
		}
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
bool GenericDisplay::get_display_value(unsigned char* buffer, int _minimum_number_of_digits, int _dot_position)
{
	if (!display_value_changed)
		return false;

	guard.lock();
	double _val = display_value;
	display_value_changed = false;
	guard.unlock();

	bool buffer_changed = false;
	if (_val > MAX_VALUE) // display shall be turned off
	{
		for (int i = 0; i < nr_of_bytes; i++)
		{
			buffer[i] = BLANK_CHAR;
		}
		buffer_changed = true;
	}
	else
	{
		buffer_changed = use_bcd ? get_decimal_components((int)_val, buffer, _minimum_number_of_digits, _dot_position) : get_binary_components(int(_val), buffer);
	}

	return buffer_changed;
}