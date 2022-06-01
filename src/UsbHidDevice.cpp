#include "UsbHidDevice.h"

int UsbHidDevice::ref_count = 0;
bool UsbHidDevice::hid_api_initialized = FALSE;

UsbHidDevice::UsbHidDevice(Configuration &config, int buffer_size) :Device(config)
{
	buffer		= (unsigned char*)calloc(buffer_size, sizeof(unsigned char));
	buffer_old  = (unsigned char*)calloc(buffer_size, sizeof(unsigned char));

	_thread_run.store(FALSE);
	vid = config.vid;
	pid = config.pid;

	if (!hid_api_initialized)
	{
		hid_init();
		hid_api_initialized = TRUE;
	}
}

UsbHidDevice::~UsbHidDevice()
{
	free(buffer);
	free(buffer_old);
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

	read_device(buffer, sizeof(buffer));
	memcpy(buffer_old, buffer, sizeof(buffer));

	return EXIT_SUCCESS;
}

bool UsbHidDevice::bit_value(unsigned char* buf, int bit)
{
	int reg = bit / 8;
	int bit_mask = (1 << (bit % 8));
	return (buf[reg] & bit_mask) == bit_mask ? 1 : 0;
}

bool UsbHidDevice::is_bit_changed(unsigned char* buf, unsigned char* buf_old, int bit)
{
	return bit_value(buf, bit) != bit_value(buf_old, bit);
}

void UsbHidDevice::start()
{
	_thread_run.store(TRUE);
}

void UsbHidDevice::stop(int time_out)
{
	_thread_run.store(FALSE);
}

void UsbHidDevice::register_buttons(std::vector<PanelButton>& _buttons)
{
	buttons = _buttons;
}

void UsbHidDevice::thread_func()
{
	while (_thread_run.load() == TRUE)
	{
		std::this_thread::sleep_for(50ms);

		if (read_device(buffer, sizeof(buffer)) == EXIT_FAILURE)
			continue;

		for (auto button : buttons)
		{
			if (is_bit_changed(buffer, buffer_old, button.bit))
			{
				if (bit_value(buffer, button.bit) && config.push_actions.find(button.config_name.c_str()) != config.push_actions.end())
				{
					ActionQueue::get_instance()->push(&config.push_actions[button.config_name.c_str()]);
				}
				else if (!bit_value(buffer, button.bit) && config.release_actions.find(button.config_name.c_str()) != config.release_actions.end())
				{
					ActionQueue::get_instance()->push(&config.release_actions[button.config_name.c_str()]);
				}
			}
		}

		memcpy(buffer_old, buffer, sizeof(buffer));		
	}
}

void UsbHidDevice::release()
{
	if (--ref_count <= 0)
		hid_exit();
}
