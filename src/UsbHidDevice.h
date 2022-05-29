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

class UsbHidDevice : public Device
{
public:
	UsbHidDevice(Configuration &config, int buffer_size);
	~UsbHidDevice();
	void thread_func();
	void start();
	void stop(int time_out);
protected:
	int connect();
	void release();
	int read_device(unsigned char* buf, int buf_size);
	int write_device(unsigned char* buf, int length);
	void register_buttons(std::vector<PanelButton>& _buttons);
	hid_device* device_handle = NULL;	
private:
	std::vector<PanelButton> buttons;
	std::atomic<bool> _thread_run;
	unsigned char* buffer;
	unsigned char* buffer_old;
	static bool hid_api_initialized;
	static int ref_count;	
	unsigned short vid=0;
	unsigned short pid=0;
	bool bit_value(unsigned char* buffer, int bit);
	bool is_bit_changed(unsigned char* buffer, unsigned char* buffer_old, int bit);
};

