#include "UsbHidDevice.h"
#include "logger.h"

int UsbHidDevice::ref_count = 0;
bool UsbHidDevice::hid_api_initialized = FALSE;

UsbHidDevice::UsbHidDevice(Configuration& config, int _read_buffer_size, int _write_buffer_size) :Device(config)
{
	read_buffer = (unsigned char*)calloc(_read_buffer_size, sizeof(unsigned char));
	read_buffer_old = (unsigned char*)calloc(_read_buffer_size, sizeof(unsigned char));
	write_buffer = (unsigned char*)calloc(_write_buffer_size, sizeof(unsigned char));

	read_buffer_size = _read_buffer_size;
	write_buffer_size = _write_buffer_size;

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
	free(read_buffer);
	free(read_buffer_old);
	free(write_buffer);
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
		Logger(TLogLevel::logERROR) << "error in UsbHidDevice::hid_read" << std::endl;
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

	if (hid_send_feature_report(device_handle, buf, length) == -1)
	{
		return EXIT_FAILURE;
		Logger(TLogLevel::logERROR) << "error in UsbHidDevice::hid_send_feature_report" << std::endl;
	}

	return EXIT_SUCCESS;
}

int UsbHidDevice::connect()
{
	device_handle = hid_open(vid, pid, NULL);
	if (!device_handle) {
		Logger(TLogLevel::logERROR) << "error opening hid device vid=" << vid << " pid=" << pid << std::endl;
		return EXIT_FAILURE;
	}
	ref_count++;

	if (hid_set_nonblocking(device_handle, 1) == -1) {
		Logger(TLogLevel::logERROR) << "error in hid_set_nonblocking vid=" << vid << " pid=" << pid << std::endl;
		return EXIT_FAILURE;
	}

	read_device(read_buffer, read_buffer_size);
	memcpy(read_buffer_old, read_buffer, read_buffer_size);

	Logger(TLogLevel::logDEBUG) << "UsbHidDevice connect successful" << std::endl;
	return EXIT_SUCCESS;
}

void UsbHidDevice::set_bit_value(unsigned char* buf, int bit_nr, int bit_val)
{
	int reg = bit_nr / 8;
	unsigned char bit_mask = ((unsigned char)0x01 << (bit_nr % 8));
	if (bit_val == 1)
		buf[reg] = buf[reg] | bit_mask;
	else
		buf[reg] = buf[reg] & (~bit_mask);
}

void UsbHidDevice::invert_bit_value(unsigned char* buf, int bit_nr)
{
	int reg = bit_nr / 8;
	unsigned char bit_mask = ((unsigned char)0x01 << (bit_nr % 8));

	buf[reg] = buf[reg] ^ bit_mask;
}

bool UsbHidDevice::get_bit_value(unsigned char* buf, int bit)
{
	int reg = bit / 8;
	unsigned char bit_mask = (1 << (bit % 8));
	return (buf[reg] & bit_mask) == bit_mask ? 1 : 0;
}

bool UsbHidDevice::is_bit_changed(unsigned char* buf, unsigned char* buf_old, int bit)
{
	return get_bit_value(buf, bit) != get_bit_value(buf_old, bit);
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

void UsbHidDevice::register_lights(std::vector<PanelLight>& _lights)
{
	lights = _lights;
}

void UsbHidDevice::thread_func()
{
	Logger(TLogLevel::logTRACE) << "UsbHidDevice::thread_func started" << std::endl;
	memset(write_buffer, 0, write_buffer_size);
	bool write_buffer_changed = false;

	while (_thread_run.load() == TRUE)
	{
		std::this_thread::sleep_for(50ms);

		// set LED states
		write_buffer_changed = false;
		for (auto it : config.light_triggers)
		{
			int bit_nr = -1;
			for (auto light : lights)
			{
				if (light.config_name == it.first)
					bit_nr = light.bit;
			}

			// light doesn't exist in this configuration
			if (bit_nr == -1)
				continue;

			for (auto trigger : it.second)
			{
				TriggerType light_change = trigger->get_and_clear_stored_action();

				switch (light_change) {
				case TriggerType::LIT:
					Logger(TLogLevel::logDEBUG) << " " << it.first << " activated LIT" << std::endl;
					set_bit_value(write_buffer, bit_nr, 1);
					write_buffer_changed = true;
					break;
				case TriggerType::UNLIT:
					Logger(TLogLevel::logDEBUG) << " " << it.first << " activated UNLIT" << std::endl;
					set_bit_value(write_buffer, bit_nr, 0);
					write_buffer_changed = true;
					break;
				case TriggerType::BLINK:
					invert_bit_value(write_buffer, bit_nr);
					write_buffer_changed = true;
					Logger(TLogLevel::logWARNING) << " " << it.first << ": light blink function not yet implemented" << std::endl;
					break;
				case TriggerType::NO_CHANGE:
					//
					break;
				default:
					Logger(TLogLevel::logERROR) << " " << it.first << ": unknown trigger type: " << light_change << std::endl;
				}
			}
		}

		if (write_buffer_changed)
		{
			write_device(write_buffer, write_buffer_size);
		}

		// read and handle button push/release events
		if (read_device(read_buffer, read_buffer_size) == EXIT_FAILURE)
		{
			Logger(TLogLevel::logDEBUG) << "UsbHidDevice error reading HID device" << std::endl;
			continue;
		}

		for (auto button : buttons)
		{
			if (is_bit_changed(read_buffer, read_buffer_old, button.bit))
			{
				Logger(TLogLevel::logTRACE) << "UsbHidDevice " << button.config_name << " button bit changed " << std::endl;
				if (get_bit_value(read_buffer, button.bit) && config.push_actions.find(button.config_name.c_str()) != config.push_actions.end())
				{
					for (auto act : config.push_actions[button.config_name.c_str()])
					{
						Logger(TLogLevel::logTRACE) << "UsbHidDevice " << button.config_name << " button push action called" << std::endl;
						ActionQueue::get_instance()->push(act);
					}
				}
				else if (!get_bit_value(read_buffer, button.bit) && config.release_actions.find(button.config_name.c_str()) != config.release_actions.end())
				{
					for (auto act : config.release_actions[button.config_name.c_str()])
					{
						Logger(TLogLevel::logTRACE) << "UsbHidDevice " << button.config_name << " button release action called" << std::endl;
						ActionQueue::get_instance()->push(act);
					}
				}
			}
		}

		memcpy(read_buffer_old, read_buffer, read_buffer_size);
	}
}

void UsbHidDevice::release()
{
	Logger(TLogLevel::logDEBUG) << "UsbHidDevice release" << std::endl;
	if (--ref_count <= 0)
		hid_exit();
}
