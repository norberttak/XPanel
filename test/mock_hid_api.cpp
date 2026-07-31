/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "hidapi.h"
#include <map>
#include <sstream>
#include <iostream>
#include <cstring>
#include <chrono>

#ifndef WIN32
#define _strdup strdup
#endif
#include <thread>

using namespace std::literals;

int device = 123;
int vid = 0;
int pid = 0;
bool hid_device_open = false;
std::atomic<bool> hid_read_event_done = false;
std::atomic<bool> hid_write_event_done = false;

extern int HID_API_EXPORT HID_API_CALL hid_init()
{
	hid_read_event_done.store(false);
	hid_device_open = false;
	return 0;
}

extern struct hid_device_info HID_API_EXPORT* HID_API_CALL hid_enumerate(unsigned short vid, unsigned short pid)
{
	hid_device_info* dev_info = new hid_device_info();
	dev_info->next = NULL;

	dev_info->product_id = pid;
	dev_info->vendor_id = vid;
	dev_info->manufacturer_string = const_cast<wchar_t*>(L"NorbiTest");
	dev_info->serial_number = const_cast<wchar_t*>(L"12345ABCD");

	std::stringstream ss;
	ss << vid << ' ' << pid;
	dev_info->path = _strdup(ss.str().c_str());

	return dev_info;
}

extern HID_API_EXPORT hid_device* HID_API_CALL hid_open_path(const char* path)
{
	std::stringstream ss;
	ss << path;
	ss >> vid >> pid;

	hid_device_open = true;
	return (hid_device*)&device;
}

extern void HID_API_EXPORT HID_API_CALL hid_free_enumeration(struct hid_device_info* devs)
{
	free(devs);
}

extern HID_API_EXPORT hid_device* HID_API_CALL hid_open(unsigned short vendor_id, unsigned short product_id, const wchar_t* serial_number)
{
	(void)serial_number;
	vid = vendor_id;
	pid = product_id;
	hid_device_open = true;
	return (hid_device*)&device;
}

void HID_API_EXPORT HID_API_CALL hid_close(hid_device* device)
{
	(void)device;
	hid_device_open = true;
}

extern int HID_API_EXPORT HID_API_CALL hid_exit(void)
{
	hid_device_open = false;
	return 0;
}

//size_t buffer_length = 0;
unsigned char mock_read_buffer[256];

void test_hid_mock_init()
{
	memset(mock_read_buffer, 0, sizeof(mock_read_buffer));
	hid_read_event_done.store(false);
}

/// @brief Set the read data for the next call to hid_read(). This is used in the unit tests to simulate the data that would be read from the HID device.
/// @param data 
/// @param length 
void test_hid_set_read_data(unsigned char* data, size_t length)
{
	memcpy(mock_read_buffer, data, length);
	hid_read_event_done.store(false);
}

/// @brief Wait for the hid_read() to be called and the data to be read from the mock buffer. This is used in the unit tests to ensure that the data has been processed before checking the results.
/// @param timeout_milliseconds
/// @return true if the data has been read, false if the timeout has expired
bool test_hid_read_wait_for_event(int timeout_milliseconds)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	timeout_milliseconds = timeout_milliseconds < 0 ? 0 : (int)timeout_milliseconds/10;

	do
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		timeout_milliseconds--;
	} while (!hid_read_event_done.load() && timeout_milliseconds > 0);

	return hid_read_event_done.load();
}

extern int HID_API_EXPORT HID_API_CALL hid_read(hid_device* device, unsigned char* data, size_t length)
{
	(void)device;
	if (!hid_device_open) {
		memset(data, 0, length);
		return -1;
	}

	memcpy(data, mock_read_buffer, length);
	hid_read_event_done.store(true);
	return length;
}

extern int hid_read_timeout(hid_device* dev, unsigned char* data, size_t length, int milliseconds)
{
	(void)milliseconds;
	return hid_read(dev, data, length);
}

extern int HID_API_EXPORT HID_API_CALL hid_set_nonblocking(hid_device* device, int nonblock)
{
	(void)device;
	(void)nonblock;
	return hid_device_open?0:-1;
}

unsigned char mock_write_buffer[256];

extern int HID_API_EXPORT HID_API_CALL hid_send_feature_report(hid_device* device, const unsigned char* data, size_t length)
{
	(void)device;
	if (!hid_device_open)
		return -1;

	memcpy(mock_write_buffer, data, length);
	hid_write_event_done.store(true);

	return length;
}

extern int HID_API_EXPORT HID_API_CALL hid_write(hid_device* device, const unsigned char* data, size_t length)
{
	return hid_send_feature_report(device, data, length);
}

HID_API_EXPORT const wchar_t* HID_API_CALL hid_error(hid_device* device)
{
	(void)device;
	return NULL;
}

bool test_hid_write_wait_for_event(int timeout_milliseconds)
{
	timeout_milliseconds = timeout_milliseconds < 0 ? 0 : (int)timeout_milliseconds/10;

	do
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		timeout_milliseconds--;
	} while (!hid_write_event_done.load() && timeout_milliseconds > 0);

	return hid_write_event_done.load();
}

/// @brief Read the data that was written to the mock HID device. This is used in the unit tests to verify that the correct data was sent to the device.
/// @param data 
/// @param length 
void test_hid_get_write_data(unsigned char* data, size_t length)
{
	test_hid_write_wait_for_event(300);

	memcpy(data, mock_write_buffer, length);
	hid_write_event_done.store(false);
}

int test_hid_get_vid()
{
	return vid;
}

int test_hid_get_pid()
{
	return pid;
}
