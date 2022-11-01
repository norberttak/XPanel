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

class UsbHidDevice : public Device
{
public:
	UsbHidDevice(DeviceConfiguration& config, int _read_buffer_size, int _write_buffer_size);
	~UsbHidDevice();
	virtual void thread_func();
	int get_vid() { return vid; }
	int get_pid() { return pid; }
protected:
	int connect();
	virtual void start();
	virtual void stop(int time_out);
	void release();
	int read_device(unsigned char* buf, int buf_size);
	int write_device(unsigned char* buf, int length);
	hid_device* device_handle = NULL;
private:
	std::atomic<bool> _thread_run;
	std::atomic<bool> _thread_finish;
	static bool hid_api_initialized;
	static int ref_count;
	unsigned short vid = 0;
	unsigned short pid = 0;
	bool updateOneDisplay(std::pair<std::string, GenericDisplay*> config_display);
	bool updateDisplays();
	std::string hidapi_error(hid_device *dev);
};
