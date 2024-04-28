/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <cstring>
#include <cstdlib>
#include <string>
#include "UsbHidDevice.h"
#include "Logger.h"

int UsbHidDevice::ref_count = 0;
bool UsbHidDevice::hid_api_initialized = false;

UsbHidDevice::UsbHidDevice(ClassConfiguration& config, int _read_buffer_size, int _write_buffer_size) :Device(config, _read_buffer_size, _write_buffer_size)
{
	_thread_run.store(false);
	vid = config.vid;
	pid = config.pid;

	if (!hid_api_initialized)
	{
		Logger(TLogLevel::logDEBUG) << "UsbHidDevice: call hid_init()" << std::endl;
		if (hid_init() == -1)
		{
			Logger(TLogLevel::logERROR) << "UsbHidDevice: error in hid_init: " << hidapi_error(NULL) << std::endl;
			hid_api_initialized = false;
		}
		else
		{
			Logger(TLogLevel::logDEBUG) << "UsbHidDevice: successful hid_init" << std::endl;
			hid_api_initialized = true;
		}
	}
}

UsbHidDevice::~UsbHidDevice()
{
	Logger(TLogLevel::logTRACE) << "UsbHidDevice::~UsbHidDevice() vid=" << vid << " pid=" << pid << std::endl;
}

int UsbHidDevice::read_device(unsigned char* buf, int buf_size)
{
	if (device_handle == NULL)
	{
		Logger(TLogLevel::logERROR) << "error in UsbHidDevice::read_device. device handle is null" << std::endl;
		return EXIT_FAILURE;
	}
	if (hid_read(device_handle, buf, buf_size) == -1)
	{
		Logger(TLogLevel::logERROR) << "error in UsbHidDevice::hid_read" << " reason=" << hidapi_error(device_handle) << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int UsbHidDevice::read_device_timeout(unsigned char* buf, int buf_size, int milliseconds)
{
	if (device_handle == NULL)
	{
		Logger(TLogLevel::logERROR) << "error in UsbHidDevice::read_device_timeout. device handle is null" << std::endl;
		return EXIT_FAILURE;
	}
	
	int bytes_read = hid_read_timeout(device_handle, buf, buf_size, milliseconds);
	if (bytes_read == -1)
	{
		Logger(TLogLevel::logERROR) << "error in UsbHidDevice::hid_read_timeout" << " reason=" << hidapi_error(device_handle) << std::endl;
		return EXIT_FAILURE;
	}
	else if (bytes_read != buf_size) {
		// timeout on device read
		Logger(TLogLevel::logDEBUG) << "error in UsbHidDevice::hid_read_timeout" << " bytes read = " << bytes_read << " expected " << buf_size << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int UsbHidDevice::send_feature_report(unsigned char* buf, int length)
{
	if (device_handle == NULL)
	{
		Logger(TLogLevel::logERROR) << "error in UsbHidDevice::send_feature_report. device handle is null" << std::endl;
		return EXIT_FAILURE;
	}

	buf[0] = 0x00; // don't use report id
	if (hid_send_feature_report(device_handle, buf, length) == -1)
	{
		Logger(TLogLevel::logERROR) << "error in UsbHidDevice::send_feature_report" << " reason=" << hidapi_error(device_handle) << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int UsbHidDevice::write_device(unsigned char* buf, int length)
{
	if (device_handle == NULL)
	{
		Logger(TLogLevel::logERROR) << "error in UsbHidDevice::write_device. device handle is null" << std::endl;
		return EXIT_FAILURE;
	}

	if (hid_write(device_handle, buf, length) == -1)
	{
		Logger(TLogLevel::logERROR) << "error in UsbHidDevice::write_device" << " reason=" << hidapi_error(device_handle) << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int UsbHidDevice::connect()
{
    struct hid_device_info *dev_info = hid_enumerate(vid, pid);
    if (!dev_info) {
        Logger(TLogLevel::logERROR) << "error enumerating hid device with vid=" << vid << " pid=" << pid << " reason=" << hidapi_error(NULL) << std::endl;
        hid_free_enumeration(dev_info);
        return EXIT_FAILURE;
    }

    device_handle = hid_open_path(dev_info->path);
    if (!device_handle) {
        Logger(TLogLevel::logERROR) << "error opening hid device vid=" << vid << " pid=" << pid << " reason=" << hidapi_error(NULL) << std::endl;
        hid_free_enumeration(dev_info);
        return EXIT_FAILURE;
    }		

    if (hid_set_nonblocking(device_handle, 1) == -1) {
        Logger(TLogLevel::logERROR) << "error in hid_set_nonblocking vid=" << vid << " pid=" << pid << " reason=" << hidapi_error(device_handle) << std::endl;
        hid_free_enumeration(dev_info);
        return EXIT_FAILURE;
    }

    ref_count++;

    if (dev_info->next) {
        Logger(TLogLevel::logWARNING) << "found more than one device with vid=" << vid << " pid=" << pid << " Only the first device is used now" << std::endl; 
    }

    Logger(TLogLevel::logDEBUG) << "device opened: vid=" << vid << " pid=" << pid << std::endl;

    hid_free_enumeration(dev_info);

    return EXIT_SUCCESS;
}

int UsbHidDevice::connect(hid_device* _device_handle)
{
	ref_count++;
	device_handle = _device_handle;
	Logger(TLogLevel::logDEBUG) << "device connect: vid=" << vid << " pid=" << pid << std::endl;

	return EXIT_SUCCESS;
}

void UsbHidDevice::start()
{
	Logger(TLogLevel::logDEBUG) << "UsbHidDevice::start" << std::endl;
	_thread_run.store(true);
}

void UsbHidDevice::stop(int time_out_msec)
{
	Logger(TLogLevel::logDEBUG) << "UsbHidDevice::stop" << std::endl;
	_thread_run.store(false);
	int time_to_wait = time_out_msec;
	while (_thread_finish.load() == false && time_to_wait > 0)
	{
		std::this_thread::sleep_for(1ms);
		time_to_wait--;
	}
	Logger(TLogLevel::logDEBUG) << "UsbHidDevice::stop. thread finish ? " << _thread_finish.load() << " remaining time=" << time_to_wait << std::endl;
}

bool UsbHidDevice::updateOneDisplay(std::pair<std::string, GenericDisplay*> config_display)
{
	bool write_buffer_changed = false;

	int reg_index = -1;
	for (auto &display : panel_displays)
	{
		if (display.config_name == config_display.first)
		{
			reg_index = display.reg_index;
			break;
		}
	}
	if (reg_index != -1 && config_display.second != NULL)
	{
		int minimum_number_of_digits = config_display.second->get_minimum_number_of_digits();
		write_buffer_changed |= config_display.second->get_display_value(&write_buffer[reg_index], minimum_number_of_digits);
	}

	return write_buffer_changed;
}

bool UsbHidDevice::updateDisplays()
{
	bool write_buffer_changed = false;
	for (auto &config_display : config.multi_displays)
	{
		write_buffer_changed |= updateOneDisplay(config_display);
	}

	for (auto &config_display : config.generic_displays)
	{
		write_buffer_changed |= updateOneDisplay(config_display);
	}

	return write_buffer_changed;
}

void UsbHidDevice::thread_func()
{
	Logger(TLogLevel::logTRACE) << "UsbHidDevice::thread_func: started for vid=" << vid << " pid=" << pid << std::endl;
	memset(write_buffer, 0, write_buffer_size);
	bool write_buffer_changed = false;

	_thread_finish.store(false);

	while (_thread_run.load() == true)
	{
		std::this_thread::sleep_for(20ms);

		write_buffer_changed = false;
		write_buffer_changed |= updateLightStates();
		write_buffer_changed |= updateDisplays();

		if (write_buffer_changed)
			if (send_feature_report(write_buffer, write_buffer_size) == EXIT_FAILURE)
			{
				Logger(TLogLevel::logERROR) << "UsbHidDevice thread_func: error writing HID device. vid=" << vid << " pid=" << pid << std::endl;
				break;
			}


		// read and handle button push/release events
		if (read_device(read_buffer, read_buffer_size) == EXIT_FAILURE)
		{
			Logger(TLogLevel::logERROR) << "UsbHidDevice thread_func: error reading HID device. vid=" << vid << " pid=" << pid << std::endl;
			break;
		}
		process_and_store_button_states();
		process_selector_switch();
		memcpy(read_buffer_old, read_buffer, read_buffer_size);
	}
	_thread_finish.store(true);
	Logger(TLogLevel::logDEBUG) << "UsbHidDevice thread_func: exit vid=" << vid << " pid=" << pid << std::endl;
}

void UsbHidDevice::release()
{
	if (device_handle != NULL)
	{
		Logger(TLogLevel::logDEBUG) << "UsbHidDevice::release called for vid=" << vid << " pid=" << pid << std::endl;
		hid_close(device_handle);
	}
	device_handle = NULL;

	Logger(TLogLevel::logDEBUG) << "UsbHidDevice release" << std::endl;
	if (--ref_count <= 0)
	{
		Logger(TLogLevel::logDEBUG) << "All USB HID devices are closed. call hid_exit()" << std::endl;
		hid_exit();
		hid_api_initialized = false;
	}
}

std::string UsbHidDevice::hidapi_error(hid_device *dev)
{
	const wchar_t* hid_err = hid_error(dev);
	std::wstring ws(hid_err);
	std::string hid_err_str(ws.begin(), ws.end());
	return hid_err_str;
}
