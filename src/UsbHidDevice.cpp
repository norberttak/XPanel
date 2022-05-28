#include "UsbHidDevice.h"

int UsbHidDevice::ref_count = 0;
bool UsbHidDevice::hid_api_initialized = FALSE;

UsbHidDevice::UsbHidDevice(Configuration &config):Device(config)
{
	vid = config.vid;
	pid = config.pid;

	if (!hid_api_initialized)
	{
		hid_init();
		hid_api_initialized = TRUE;
	}
}

int UsbHidDevice::read_device(unsigned char* buf, int buf_size)
{
	if (device_handle == NULL)
		return EXIT_FAILURE;

	if (hid_read(device_handle, buf, buf_size) == -1)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

int UsbHidDevice::write_device(unsigned char* buf, int length)
{
	if (device_handle == NULL)
		return EXIT_FAILURE;

	if (hid_send_feature_report(device_handle, buf, length) == -1)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

int UsbHidDevice::connect()
{	
	device_handle = hid_open(vid, pid, NULL);
	if (!device_handle)
		return EXIT_FAILURE;

	ref_count++;

	if (hid_set_nonblocking(device_handle, 1) == -1)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

void UsbHidDevice::release()
{
	if (--ref_count <= 0)
		hid_exit();
}
