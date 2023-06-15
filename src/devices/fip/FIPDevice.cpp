/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
 
#include <cstring>
#include "core/Logger.h"
#include "FIPDevice.h"

FIPDevice::FIPDevice(DeviceConfiguration& _config) :Device(_config,2,1)
{
	f_device = NULL;
	//bit index 0 not used
	fip_buttons.push_back(PanelButton(1, "RIGHT_KNOB_PLUS"));
	fip_buttons.push_back(PanelButton(2, "RIGHT_KNOB_MINUS"));
	fip_buttons.push_back(PanelButton(3, "LEFT_KNOB_PLUS"));
	fip_buttons.push_back(PanelButton(4, "LEFT_KNOB_MINUS"));
	fip_buttons.push_back(PanelButton(5, "S1"));
	fip_buttons.push_back(PanelButton(6, "S2"));
	fip_buttons.push_back(PanelButton(7, "S3"));
	fip_buttons.push_back(PanelButton(8, "S4"));
	fip_buttons.push_back(PanelButton(9, "S5"));
	fip_buttons.push_back(PanelButton(10, "S6"));
	register_buttons(fip_buttons);

	//LED index 0 not used
	fip_lights.push_back(PanelLight(1, "S1_L"));
	fip_lights.push_back(PanelLight(2, "S2_L"));
	fip_lights.push_back(PanelLight(3, "S3_L"));
	fip_lights.push_back(PanelLight(4, "S4_L"));
	fip_lights.push_back(PanelLight(5, "S5_L"));
	fip_lights.push_back(PanelLight(6, "S6_L"));
	register_lights(fip_lights);

	page_condition_name_old = "";
	page_index_old = -1;
}

void FIPDevice::start()
{
	Logger(TLogLevel::logDEBUG) << "FIPDevice::start" << std::endl;

	_thread_run.store(true);
}

void FIPDevice::stop(int time_out_msec)
{
	Logger(TLogLevel::logDEBUG) << "FIPDevice::stop" << std::endl;
	_thread_run.store(false);
	int time_to_wait = time_out_msec;
	while (_thread_finish.load() == false && time_to_wait > 0)
	{
		std::this_thread::sleep_for(1ms);
		time_to_wait--;
	}
	Logger(TLogLevel::logDEBUG) << "FIPDevice::stop. thread finish ? " << _thread_finish.load() << " remaining time=" << time_to_wait << std::endl;
}

int FIPDevice::connect()
{
	f_device = fip_open(config.serial_number);
	if (f_device == NULL)
	{
		Logger(logERROR) << "FIPDevice: unable to open device with serial number: " << config.serial_number << std::endl;
		return EXIT_FAILURE;
	}
	Logger(logTRACE) << "FIPDevice: device open successful. serial: " << config.serial_number << std::endl;
	return EXIT_SUCCESS;
}

void FIPDevice::release()
{
	if (f_device != NULL)
	{
		fip_close(f_device);
		f_device = NULL;
	}
	Logger(logTRACE) << "FIPDevice: closed" << std::endl;
}

void FIPDevice::set_led(int led_index, int led_state)
{
	Logger(logTRACE) << "FIPDevice: set led " << led_index << " to " << led_state << std::endl;
	fip_set_led(f_device, led_index, led_state);
}

uint16_t FIPDevice::get_button_state()
{
	uint16_t button_states = fip_get_button_states(f_device);
	Logger(logTRACE) << "FIPDevice: button states=" << button_states << std::endl;
	return button_states;
}

void FIPDevice::set_text(int page_index, std::string text)
{
	fip_set_text(f_device, page_index, text);
}

void FIPDevice::render_screen(FIPScreen* screen)
{
	int current_page_index = fip_get_current_page(f_device);
	void* byte_buffer = NULL;
	screen->render_page(current_page_index, &byte_buffer);
	fip_set_image(f_device, current_page_index, byte_buffer);
}

void FIPDevice::process_page_conditions(FIPScreen* screen)
{
	int current_page = fip_get_current_page(f_device);
	std::string page_condition_name = "PAGE_" + screen->get_page_name(current_page);

	if (page_condition_name_old == page_condition_name)
		return;

	if (!page_condition_name_old.empty())
	{
		for (auto &button : buttons)
		{
			for (auto &act : config.push_actions[button.config_name.c_str()])
			{
				act->set_condition_inactive(page_condition_name_old);
			}
			for (auto &act : config.release_actions[button.config_name.c_str()])
			{
				act->set_condition_inactive(page_condition_name_old);
			}
		}
	}

	for (auto &button : buttons)
	{
		for (auto &act : config.push_actions[button.config_name.c_str()])
		{
			act->set_condition_active(page_condition_name);
		}
		for (auto &act : config.release_actions[button.config_name.c_str()])
		{
			act->set_condition_active(page_condition_name);
		}
	}

	page_condition_name_old = page_condition_name;
}

void FIPDevice::thread_func()
{
	Logger(TLogLevel::logTRACE) << "FIPDevice::thread_func: started for serial=" << config.serial_number << std::endl;
	_thread_finish.store(false);

	// create page registers in physical device
	for (const auto &it_screen : config.fip_screens)
	{
		for (int page_index = 0; page_index <= it_screen.second->get_last_page_index(); page_index++)
		{
			fip_add_page(f_device, page_index, page_index == 0 ? true : false);
		}
	}
	
	bool write_buffer_changed = false;
	
	while (_thread_run.load() == true)
	{
		std::this_thread::sleep_for(20ms);
		for (auto &screen : config.fip_screens)
		{
			render_screen(screen.second);
			process_page_conditions(screen.second);
		}
		
		write_buffer_changed = false;

		// if page has changed we have to re-send LED states to device
		int page_index = fip_get_current_page(f_device);
		write_buffer_changed = (page_index != page_index_old);
		page_index_old = page_index;

		write_buffer_changed |= updateLightStates();		
		if (write_buffer_changed)
		{
			for (int i = 0; i < write_buffer_size * 8; i++)
			{
				int led_state = 0;
				if ((write_buffer[i / 8] & (0x01 << (i % 8))) == (0x01 << (i % 8)))
					led_state = 1;
				else
					led_state = 0;

				fip_set_led(f_device, i, led_state);
			}
		}

		uint16_t button_states = fip_get_button_states(f_device);
		for (int i=0; i<read_buffer_size; i++)
			read_buffer[i] = (unsigned char)((button_states >> (i*8)) & 0xFF);

		process_and_store_button_states();
		process_selector_switch();
		memcpy(read_buffer_old, read_buffer, read_buffer_size);
	}
	_thread_finish.store(true);
	Logger(TLogLevel::logDEBUG) << "FIPDevice thread_func: exit serial=" << config.serial_number << std::endl;
}
