#pragma once
#include <string>
#include <map>
#include <atomic>
#include <mutex>
#include "XPLMDataAccess.h"
#include "XPLMMenus.h"

class MultiPurposeDisplay
{
public:
	MultiPurposeDisplay();
	// called during init phase
	void add_condition(std::string selector_sw_name, XPLMDataRef data);
	void add_condition(std::string selector_sw_name, double const_value);
	void add_condition(std::string selector_sw_name, std::string lua_str);

	// called from UsbHidDevice worker thread
	void set_condition_active(std::string selector_sw_name);
	bool get_display_value(unsigned char* digits);
	bool is_registered_selector(std::string);

	// called from XPLane flight loop
	void evaluate_and_store_dataref_value();
	void set_nr_digits(int _nr_of_digits);
private:
	std::map<std::string, XPLMDataRef> conditions;
	std::map<std::string, double> const_values;
	std::map<std::string, std::string> lua_functions;
	std::map<std::string, XPLMDataTypeID> data_ref_types;
	std::string active_condition;
	bool get_decimal_components(int number, int max_dec_pos, unsigned char* buffer);
	double	display_value;
	double	display_value_old;
	bool display_value_changed;
	int nr_of_digits;
	std::mutex guard;
	bool turn_off;
};

