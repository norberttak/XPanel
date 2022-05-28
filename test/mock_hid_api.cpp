#pragma once
#include"hidapi.h"
#include <map>

int device = 123;
int vid = 0;
int pid = 0;

extern int HID_API_EXPORT HID_API_CALL hid_init()
{
	return 0;
}

extern HID_API_EXPORT hid_device* HID_API_CALL hid_open(unsigned short vendor_id, unsigned short product_id, const wchar_t* serial_number)
{
	vid = vendor_id;
	pid = product_id;
	return (hid_device*)&device;
}

extern int HID_API_EXPORT HID_API_CALL hid_exit(void)
{
	return 0;
}

size_t buffer_length = 0;
unsigned char read_buffer[256];

void test_hid_set_read_data(unsigned char* data, size_t length)
{
	memcpy(read_buffer, data, length);
}

extern int HID_API_EXPORT HID_API_CALL hid_read(hid_device* device, unsigned char* data, size_t length)
{
	memcpy(data, read_buffer, length);
	return 0;
}

extern int HID_API_EXPORT HID_API_CALL hid_set_nonblocking(hid_device* device, int nonblock)
{
	return 0;
}

unsigned char write_buffer[256];

extern int HID_API_EXPORT HID_API_CALL hid_send_feature_report(hid_device* device, const unsigned char* data, size_t length)
{
	memcpy(write_buffer, data, length);
	return 0;
}

void test_hid_get_write_data(unsigned char* data, size_t length)
{
	memcpy(data, write_buffer, length);
}

int test_hid_get_vid()
{
	return vid;
}

int test_hid_get_pid()
{
	return pid;
}


