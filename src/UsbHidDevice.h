#pragma once
#include"hidapi.h"
#include "Device.h"
#include "configuration.h"

class UsbHidDevice : public Device
{
public:
	UsbHidDevice(Configuration &config);
	int connect();
	void release();
protected:
	int read_device(unsigned char* buf, int buf_size);
	int write_device(unsigned char* buf, int length);
	hid_device* device_handle = NULL;
private:
	static bool hid_api_initialized;
	static int ref_count;	
	unsigned short vid=0;
	unsigned short pid=0;
};

