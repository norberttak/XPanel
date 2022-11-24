/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <string>
#include <map>
#include <list>
#include <thread>
#include "Action.h"
#include "Configuration.h"
#include <queue>
#include "Action.h"
#include <thread>
#include <chrono>
using namespace std::literals;

struct PanelButton
{
	PanelButton(int _bit, std::string _config_name)
	{
		bit = _bit;
		config_name = _config_name;
	}
	int bit;
	std::string config_name;
};

struct PanelLight
{
	PanelLight(int _bit, std::string _config_name)
	{
		bit = _bit;
		config_name = _config_name;
	}
	int bit;
	std::string config_name;
	bool blink_active = false;
};

struct PanelDisplay
{
	PanelDisplay(int _reg_index, int _width, std::string _config_name)
	{
		reg_index = _reg_index;
		width = _width;
		config_name = _config_name;
	}
	int reg_index;
	int width;
	std::string config_name;
};

struct PanelRotaryEncoder
{
	PanelRotaryEncoder(int _reg_index, int _width, std::string _config_name)
	{
		reg_index = _reg_index;
		width = _width;
		config_name = _config_name;
		accumulated_change = 0;
		pulse_per_click = 2;
	}

	PanelRotaryEncoder(int _reg_index, std::string _config_name)
	{
		reg_index = _reg_index;
		width = 1;
		config_name = _config_name;
		accumulated_change = 0;
		pulse_per_click = 2;
	}

	int reg_index;
	int width;
	int accumulated_change;
	int pulse_per_click;
	std::string config_name;
};

class Device
{
private:
	int compute_rotation_delta_with_overflow(unsigned char rot, unsigned char rot_old);
protected:
	DeviceConfiguration config;
	std::vector<PanelButton> selectors;
	std::vector<PanelButton> buttons;
	std::vector<PanelDisplay> panel_displays;
	std::vector< PanelRotaryEncoder> encoders;
	std::map<std::string, int> stored_button_states;
	std::map<std::string, TriggerType> stored_light_states;
	std::vector<PanelLight> lights;
	unsigned char* read_buffer;
	unsigned char* read_buffer_old;
	unsigned char* write_buffer;
	int write_buffer_size;
	int read_buffer_size;
	virtual void register_displays(std::vector<PanelDisplay>& _displays);
	virtual void register_buttons(std::vector<PanelButton>& _buttons);
	virtual void register_selectors(std::vector<PanelButton>& _selectors);
	virtual void register_lights(std::vector<PanelLight>& _lights);
	virtual void register_rotary_encoders(std::vector<PanelRotaryEncoder>& _encoders);
public:
	std::thread* thread_handle = NULL;
	Device(DeviceConfiguration &_config, int _read_buffer_size, int _write_buffer_size);
	~Device();
	int get_stored_button_state(std::string button_name);
	TriggerType get_stored_light_state(std::string light_name);
	bool updateLightStates();
	void process_and_store_button_states();
	void process_selector_switch();
	void process_and_store_encoder_rotations();
	virtual void stop(int time_out)=0;
	virtual void thread_func(void)=0;
	virtual int connect() = 0;
	virtual void release() = 0;
	virtual void start() = 0;
private:
	int update_cycle = 0;
	bool get_bit_value(unsigned char* buffer, int bit);
	bool is_bit_changed(unsigned char* buffer, unsigned char* buffer_old, int bit);
	void set_bit_value(unsigned char* buf, int bit_nr, int bit_val);
	void invert_bit_value(unsigned char* buf, int bit_nr);
};
