/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <string>
#include <map>
#include <atomic>
#include <mutex>
#include <limits>
#include <cstddef>
#include "XPLMDataAccess.h"
#include "XPLMMenus.h"

class GenericDisplay
{
public:
	GenericDisplay();
	GenericDisplay(bool _use_bcd); // bcd: binary encoded decimal
	GenericDisplay(GenericDisplay* other);

	static const double MAX_VALUE;
	void set_nr_bytes(int _nr_of_bytes);

	// called from UsbHidDevice worker thread
	virtual bool get_display_value(unsigned char* buffer, int _minimum_number_of_digits, int _dot_position);
	virtual int get_minimum_number_of_digits();
	virtual int get_dot_position();

	// called from XPLane flight loop
	virtual void evaluate_and_store_dataref_value();
	void add_dataref(XPLMDataRef _data_ref);
	void add_dataref(XPLMDataRef _data_ref, int _dataref_index);
	void add_const(double _const_value);
	void add_lua(std::string _lua_function);
	void set_bcd(bool _use_bcd);
	void set_blank_leading_zeros(bool _blank_leading_zeros);
	void set_minimum_number_of_digits(int _minimum_number_of_digits);
	void set_dot_position(int _dot_position);
protected:	
	double	display_value;
	double	display_value_old;
	bool display_value_changed;
	std::mutex guard;
	bool use_bcd;
	bool blank_leading_zeros;
	int minimum_number_of_digits;
	int dot_position;
	std::string lua_function;
	XPLMDataRef condition;
	XPLMDataTypeID data_ref_type;
	double	const_value;
	int nr_of_bytes;
private:
	const unsigned char BLANK_CHAR = 0xFF;
	const unsigned char ZERO_CHAR = 0x00;
	const unsigned char PERIOD_CHAR = 0xD0;

	int dataref_index;
	bool get_decimal_components(int number, unsigned char* buffer, int _minimum_number_of_digits, int _dot_position);
	bool get_binary_components(int number, unsigned char* buffer);
};
