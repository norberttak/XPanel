#pragma once
#include <stdlib.h>
#include <thread>
#include <atomic>
#include <chrono>
#include <utility>
#include <vector>

#include"hidapi.h"
#include "Device.h"
#include "configuration.h"

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
};

struct PanelDisplay
{
	PanelDisplay(int _reg_index, std::string _config_name)
	{
		reg_index = _reg_index;
		config_name = _config_name;
	}
	int reg_index;
	std::string config_name;
};

class UsbHidDevice : public Device
{
public:
	UsbHidDevice(DeviceConfiguration &config, int _read_buffer_size, int _write_buffer_size);
	~UsbHidDevice();
	virtual void thread_func();
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
	unsigned short vid=0;
	unsigned short pid=0;
	bool get_bit_value(unsigned char* buffer, int bit);
	bool is_bit_changed(unsigned char* buffer, unsigned char* buffer_old, int bit);
	void set_bit_value(unsigned char* buf, int bit_nr, int bit_val);
	void invert_bit_value(unsigned char* buf, int bit_nr);
	bool updateLightStates();
	bool updateDisplays();
	void process_and_store_button_states();
	void process_selector_switch();
};

