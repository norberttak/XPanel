/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "hidapi.h"
#include <map>
#include <sstream>
#include <iostream>

int device = 123;
int vid = 0;
int pid = 0;
bool hid_device_open = false;

extern int HID_API_EXPORT HID_API_CALL hid_init()
{
	hid_device_open = false;
	return 0;
}

extern struct hid_device_info HID_API_EXPORT* HID_API_CALL hid_enumerate(unsigned short vid, unsigned short pid)
{
	hid_device_info* dev_info = new hid_device_info();
	dev_info->next = NULL;

	dev_info->product_id = pid;
	dev_info->vendor_id = vid;
	dev_info->manufacturer_string = L"NorbiTest";
	dev_info->serial_number = L"12345ABCD";

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
	vid = vendor_id;
	pid = product_id;
	hid_device_open = true;
	return (hid_device*)&device;
}

void HID_API_EXPORT HID_API_CALL hid_close(hid_device* device)
{
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
}

void test_hid_set_read_data(unsigned char* data, size_t length)
{
	memcpy(mock_read_buffer, data, length);
}

extern int HID_API_EXPORT HID_API_CALL hid_read(hid_device* device, unsigned char* data, size_t length)
{
	if (!hid_device_open) {
		memset(data, 0, length);
		return -1;
	}

	memcpy(data, mock_read_buffer, length);
	return length;
}

extern int hid_read_timeout(hid_device* dev, unsigned char* data, size_t length, int milliseconds)
{
	(void)milliseconds;
	return hid_read(dev, data, length);
}

extern int HID_API_EXPORT HID_API_CALL hid_set_nonblocking(hid_device* device, int nonblock)
{
	return hid_device_open?0:-1;
}

unsigned char mock_write_buffer[256];

extern int HID_API_EXPORT HID_API_CALL hid_send_feature_report(hid_device* device, const unsigned char* data, size_t length)
{
	if (!hid_device_open)
		return -1;

	memcpy(mock_write_buffer, data, length);
	return length;
}

extern int HID_API_EXPORT HID_API_CALL hid_write(hid_device* device, const unsigned char* data, size_t length)
{
	return hid_send_feature_report(device, data, length);
}

HID_API_EXPORT const wchar_t* HID_API_CALL hid_error(hid_device* device)
{
	return NULL;
}

void test_hid_get_write_data(unsigned char* data, size_t length)
{
	memcpy(data, mock_write_buffer, length);
}

int test_hid_get_vid()
{
	return vid;
}

int test_hid_get_pid()
{
	return pid;
}
