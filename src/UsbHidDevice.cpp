#include "UsbHidDevice.h"
#include "logger.h"

int UsbHidDevice::ref_count = 0;
bool UsbHidDevice::hid_api_initialized = FALSE;

UsbHidDevice::UsbHidDevice(DeviceConfiguration& config, int _read_buffer_size, int _write_buffer_size) :Device(config)
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
		Logger(TLogLevel::logDEBUG) << "UsbHidDevice: call hid_init()" << std::endl;
		hid_init();
		hid_api_initialized = TRUE;
	}
}

UsbHidDevice::~UsbHidDevice()
{
	Logger(TLogLevel::logTRACE) << "UsbHidDevice::~UsbHidDevice() vid=" << vid << " pid=" << pid << std::endl;
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

	Logger(TLogLevel::logDEBUG) << "UsbHidDevice connect successful. vid=" << vid << " pid=" << pid << std::endl;
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
	Logger(TLogLevel::logDEBUG) << "UsbHidDevice::start" << std::endl;
	_thread_run.store(TRUE);
}

void UsbHidDevice::stop(int time_out_msec)
{
	Logger(TLogLevel::logDEBUG) << "UsbHidDevice::stop" << std::endl;
	_thread_run.store(FALSE);
	int time_to_wait = time_out_msec;
	while (_thread_finish.load() == FALSE && time_to_wait > 0)
	{
		std::this_thread::sleep_for(1ms);
		time_to_wait--;
	}
	Logger(TLogLevel::logDEBUG) << "UsbHidDevice::stop. thread finish ? " << _thread_finish.load() << " remaining time=" << time_to_wait << std::endl;
}

void UsbHidDevice::register_buttons(std::vector<PanelButton>& _buttons)
{
	buttons = _buttons;
}

void UsbHidDevice::register_selectors(std::vector<PanelButton>& _selectors)
{
	selectors = _selectors;
}

void UsbHidDevice::register_lights(std::vector<PanelLight>& _lights)
{
	lights = _lights;
}

void UsbHidDevice::register_displays(std::vector<PanelDisplay>& _displays)
{
	panel_displays = _displays;
}

#define BLINK_TIME_PERIOD 25 // 25*20*2ms = 1 sec blink period time

bool UsbHidDevice::updateLightStates()
{
	bool write_buffer_changed = false;

	if (update_cycle > BLINK_TIME_PERIOD)
		update_cycle = 0;
	else
		update_cycle++;

	for (auto it : config.light_triggers)
	{
		std::vector<PanelLight>::iterator panel_light_it;

		for (panel_light_it = lights.begin(); panel_light_it != lights.end(); ++panel_light_it)
		{
			if (!panel_light_it->config_name.empty() && panel_light_it->config_name == it.first)
				break;
		}

		// light doesn't exist in this configuration
		if (panel_light_it == lights.end())
			continue;

		for (auto trigger : it.second)
		{
			TriggerType light_change = trigger->get_and_clear_stored_action();
			switch (light_change) {
			case TriggerType::LIT:
				Logger(TLogLevel::logDEBUG) << " " << it.first << " activated LIT" << std::endl;
				set_bit_value(write_buffer, panel_light_it->bit, 1);
				write_buffer_changed = true;
				panel_light_it->blink_active = false;
				break;
			case TriggerType::UNLIT:
				Logger(TLogLevel::logDEBUG) << " " << it.first << " activated UNLIT" << std::endl;
				set_bit_value(write_buffer, panel_light_it->bit, 0);
				write_buffer_changed = true;
				panel_light_it->blink_active = false;
				break;
			case TriggerType::BLINK:
				Logger(TLogLevel::logDEBUG) << " " << it.first << " activated BLINK" << std::endl;
				panel_light_it->blink_active = true;
				break;
			case TriggerType::NO_CHANGE:
				//
				break;
			default:
				Logger(TLogLevel::logERROR) << " " << it.first << ": unknown trigger type: " << light_change << std::endl;
			}
		}

		if (panel_light_it->blink_active && update_cycle == BLINK_TIME_PERIOD)
		{
			invert_bit_value(write_buffer, panel_light_it->bit);
			write_buffer_changed = true;
		}
	}
	return write_buffer_changed;
}

bool UsbHidDevice::updateOneDisplay(std::pair<std::string, GenericDisplay*> config_display)
{
	bool write_buffer_changed = false;

	int reg_index = -1;
	for (auto display : panel_displays)
	{
		if (display.config_name == config_display.first)
		{
			reg_index = display.reg_index;
			break;
		}
	}
	if (reg_index != -1 && config_display.second != NULL)
	{
		write_buffer_changed |= config_display.second->get_display_value(&write_buffer[reg_index]);
	}

	return write_buffer_changed;
}

bool UsbHidDevice::updateDisplays()
{
	bool write_buffer_changed = false;
	for (auto config_display : config.multi_displays)
	{
		write_buffer_changed |= updateOneDisplay(config_display);
	}

	for (auto config_display : config.generic_displays)
	{
		write_buffer_changed |= updateOneDisplay(config_display);
	}

	return write_buffer_changed;
}

void UsbHidDevice::process_selector_switch()
{
	for (auto sel : selectors)
	{
		if (!is_bit_changed(read_buffer, read_buffer_old, sel.bit))
			continue;

		if (get_bit_value(read_buffer, sel.bit))
		{
			for (auto button : buttons)
			{
				for (auto act : config.push_actions[button.config_name.c_str()])
				{
					act->set_condition_active(sel.config_name);
				}
				for (auto act : config.release_actions[button.config_name.c_str()])
				{
					act->set_condition_active(sel.config_name);
				}
			}

			/* update condition for displays */
			for (auto display : panel_displays)
			{
				if (config.multi_displays.count(display.config_name) > 0 && config.multi_displays[display.config_name] != NULL && config.multi_displays[display.config_name]->is_registered_selector(sel.config_name))
				{
					Logger(TLogLevel::logTRACE) << "UsbHidDevice " << sel.config_name << " mode selector switch for " << display.config_name << " is activated" << std::endl;
					config.multi_displays[display.config_name]->set_condition_active(sel.config_name);
				}
			}
		}
		else
		{
			for (auto button : buttons)
			{
				for (auto act : config.push_actions[button.config_name.c_str()])
				{
					act->set_condition_inactive(sel.config_name);
				}
				for (auto act : config.release_actions[button.config_name.c_str()])
				{
					act->set_condition_inactive(sel.config_name);
				}
			}
		}
	}
}

void UsbHidDevice::process_and_store_button_states()
{
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
}

void UsbHidDevice::thread_func()
{
	Logger(TLogLevel::logTRACE) << "UsbHidDevice::thread_func: started for vid=" << vid << " pid=" << pid << std::endl;
	memset(write_buffer, 0, write_buffer_size);
	bool write_buffer_changed = false;

	_thread_finish.store(false);

	while (_thread_run.load() == TRUE)
	{
		std::this_thread::sleep_for(20ms);

		write_buffer_changed = false;
		write_buffer_changed |= updateLightStates();
		write_buffer_changed |= updateDisplays();

		if (write_buffer_changed)
			if (write_device(write_buffer, write_buffer_size) == EXIT_FAILURE)
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
	}
}