/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <cstdlib>
#include <thread>
#include <atomic>
#include <chrono>
#include <utility>
#include <vector>
#include <string>

#include"hidapi.h"
#include "Device.h"
#include "Configuration.h"

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

class UsbHidDevice : public Device
{
public:
	UsbHidDevice(DeviceConfiguration& config, int _read_buffer_size, int _write_buffer_size);
	~UsbHidDevice();
	virtual void thread_func();
	int get_vid() { return vid; }
	int get_pid() { return pid; }
	int get_stored_button_state(std::string button_name);
	TriggerType get_stored_light_state(std::string light_name);
protected:
	int connect();
	virtual void start();
	virtual void stop(int time_out);
	void release();
	int read_device(unsigned char* buf, int buf_size);
	int write_device(unsigned char* buf, int length);
	void register_buttons(std::vector<PanelButton>& _buttons);
	void register_selectors(std::vector<PanelButton>& _selectors);
	void register_lights(std::vector<PanelLight>& _lights);
	void register_displays(std::vector<PanelDisplay>& _displays);
	hid_device* device_handle = NULL;
private:
	std::vector<PanelButton> buttons;
	std::map<std::string, int> stored_button_states;
	std::map<std::string, TriggerType> stored_light_states;
	std::vector<PanelButton> selectors;
	std::vector<PanelLight> lights;
	std::vector<PanelDisplay> panel_displays;
	std::atomic<bool> _thread_run;
	std::atomic<bool> _thread_finish;
	unsigned char* read_buffer;
	unsigned char* read_buffer_old;
	unsigned char* write_buffer;
	int write_buffer_size;
	int read_buffer_size;
	static bool hid_api_initialized;
	static int ref_count;
	int update_cycle = 0;
	unsigned short vid = 0;
	unsigned short pid = 0;
	bool get_bit_value(unsigned char* buffer, int bit);
	bool is_bit_changed(unsigned char* buffer, unsigned char* buffer_old, int bit);
	void set_bit_value(unsigned char* buf, int bit_nr, int bit_val);
	void invert_bit_value(unsigned char* buf, int bit_nr);
	bool updateLightStates();
	bool updateOneDisplay(std::pair<std::string, GenericDisplay*> config_display);
	bool updateDisplays();
	void process_and_store_button_states();
	void process_selector_switch();
	std::string hidapi_error(hid_device *dev);
};
