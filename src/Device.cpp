/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Device.h"
#include "Logger.h"

Device::Device(DeviceConfiguration& _config, int _read_buffer_size, int _write_buffer_size):
	config(_config), read_buffer_size(_read_buffer_size), write_buffer_size(_write_buffer_size)
{
	read_buffer = (unsigned char*)calloc(_read_buffer_size, sizeof(unsigned char));
	read_buffer_old = (unsigned char*)calloc(_read_buffer_size, sizeof(unsigned char));
	write_buffer = (unsigned char*)calloc(_write_buffer_size, sizeof(unsigned char));
}

Device::~Device()
{
	free(read_buffer);
	free(read_buffer_old);
	free(write_buffer);
}

void Device::set_bit_value(unsigned char* buf, int bit_nr, int bit_val)
{
	int reg = bit_nr / 8;
	unsigned char bit_mask = ((unsigned char)0x01 << (bit_nr % 8));
	if (bit_val == 1)
		buf[reg] = buf[reg] | bit_mask;
	else
		buf[reg] = buf[reg] & (~bit_mask);
}

void Device::invert_bit_value(unsigned char* buf, int bit_nr)
{
	int reg = bit_nr / 8;
	unsigned char bit_mask = ((unsigned char)0x01 << (bit_nr % 8));

	buf[reg] = buf[reg] ^ bit_mask;
}

bool Device::get_bit_value(unsigned char* buf, int bit)
{
	int reg = bit / 8;
	unsigned char bit_mask = (1 << (bit % 8));
	return (buf[reg] & bit_mask) == bit_mask ? 1 : 0;
}

bool Device::is_bit_changed(unsigned char* buf, unsigned char* buf_old, int bit)
{
	return get_bit_value(buf, bit) != get_bit_value(buf_old, bit);
}

void Device::register_buttons(std::vector<PanelButton>& _buttons)
{
	buttons = _buttons;
}

void Device::register_selectors(std::vector<PanelButton>& _selectors)
{
	selectors = _selectors;
}

void Device::register_lights(std::vector<PanelLight>& _lights)
{
	lights = _lights;
}

void Device::register_displays(std::vector<PanelDisplay>& _displays)
{
	panel_displays = _displays;
}

void Device::register_rotary_encoders(std::vector<PanelRotaryEncoder>& _encoders)
{
	encoders = _encoders;
}

int Device::get_stored_button_state(std::string button_name)
{
	if (stored_button_states.count(button_name) == 0)
	{
		Logger(TLogLevel::logWARNING) << "get_stored_button_state: unknown button name: " << button_name << std::endl;
		return -1;
	}

	return stored_button_states[button_name];
}

TriggerType Device::get_stored_light_state(std::string light_name)
{
	if (stored_light_states.count(light_name) == 0)
	{
		Logger(TLogLevel::logWARNING) << "get_stored_light_state: unknown light name: " << light_name << std::endl;
		return TriggerType::UNKNOWN;
	}

	return stored_light_states[light_name];
}

#define BLINK_TIME_PERIOD 25 // 25*20*2ms = 1 sec blink period time

bool Device::updateLightStates()
{
	bool write_buffer_changed = false;

	if (update_cycle > BLINK_TIME_PERIOD)
		update_cycle = 0;
	else
		update_cycle++;

	for (auto &it : config.light_triggers)
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

		for (auto &trigger : it.second)
		{
			TriggerType light_change = trigger->get_and_clear_stored_action();
			switch (light_change) {
			case TriggerType::LIT:
				Logger(TLogLevel::logDEBUG) << " " << it.first << " activated LIT" << std::endl;
				set_bit_value(write_buffer, panel_light_it->bit, 1);
				write_buffer_changed = true;
				panel_light_it->blink_active = false;
				stored_light_states[it.first] = light_change;
				break;
			case TriggerType::UNLIT:
				Logger(TLogLevel::logDEBUG) << " " << it.first << " activated UNLIT" << std::endl;
				set_bit_value(write_buffer, panel_light_it->bit, 0);
				write_buffer_changed = true;
				panel_light_it->blink_active = false;
				stored_light_states[it.first] = light_change;
				break;
			case TriggerType::BLINK:
				Logger(TLogLevel::logDEBUG) << " " << it.first << " activated BLINK" << std::endl;
				panel_light_it->blink_active = true;
				stored_light_states[it.first] = light_change;
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

void Device::process_selector_switch()
{
	for (auto &sel : selectors)
	{
		if (!is_bit_changed(read_buffer, read_buffer_old, sel.bit))
			continue;

		if (get_bit_value(read_buffer, sel.bit))
		{
			stored_button_states[sel.config_name] = 1;

			for (auto &button : buttons)
			{
				for (auto &act : config.push_actions[button.config_name.c_str()])
				{
					act->set_condition_active(sel.config_name);
				}
				for (auto &act : config.release_actions[button.config_name.c_str()])
				{
					act->set_condition_active(sel.config_name);
				}
			}

			/* update condition for displays */
			for (auto &display : panel_displays)
			{
				if (config.multi_displays.count(display.config_name) > 0 && config.multi_displays[display.config_name] != NULL && config.multi_displays[display.config_name]->is_registered_selector(sel.config_name))
				{
					Logger(TLogLevel::logTRACE) << "Device " << sel.config_name << " mode selector switch for " << display.config_name << " is activated" << std::endl;
					config.multi_displays[display.config_name]->set_condition_active(sel.config_name);
				}
			}
		}
		else
		{
			stored_button_states[sel.config_name] = 0;

			for (auto &button : buttons)
			{
				for (auto &act : config.push_actions[button.config_name.c_str()])
				{
					act->set_condition_inactive(sel.config_name);
				}
				for (auto &act : config.release_actions[button.config_name.c_str()])
				{
					act->set_condition_inactive(sel.config_name);
				}
			}
		}
	}
}

void Device::process_and_store_button_states()
{
	for (auto &button : buttons)
	{
		if (is_bit_changed(read_buffer, read_buffer_old, button.bit))
		{
			stored_button_states[button.config_name] = get_bit_value(read_buffer, button.bit);

			Logger(TLogLevel::logTRACE) << "Device " << button.config_name << " button bit changed " << std::endl;
			if (get_bit_value(read_buffer, button.bit) && config.push_actions.find(button.config_name.c_str()) != config.push_actions.end())
			{
				for (auto &act : config.push_actions[button.config_name])
				{
					Logger(TLogLevel::logTRACE) << "Device " << button.config_name << " button push action called" << std::endl;
					ActionQueue::get_instance()->push(act);
				}
			}
			else if (!get_bit_value(read_buffer, button.bit) && config.release_actions.find(button.config_name.c_str()) != config.release_actions.end())
			{
				for (auto &act : config.release_actions[button.config_name])
				{
					Logger(TLogLevel::logTRACE) << "Device " << button.config_name << " button release action called" << std::endl;
					ActionQueue::get_instance()->push(act);
				}
			}
		}
	}
}

int Device::compute_rotation_delta_with_overflow(unsigned char rot, unsigned char rot_old)
{
	int rot_diff = rot - rot_old;
	if (rot_diff >= 128)
	{
		rot_diff = rot_diff - 255 -1;
	}
	else if (rot_diff <= -128)
	{
		rot_diff = rot_diff + 255 + 1;
	}
	return rot_diff;
}

void Device::process_and_store_encoder_rotations()
{
	for (auto &encoder : encoders)
	{
		int encoder_delta = compute_rotation_delta_with_overflow(read_buffer[encoder.reg_index], read_buffer_old[encoder.reg_index]);		

		if (encoder_delta == 0)
			continue;

		encoder_delta += encoder.accumulated_change;

		if (encoder_delta > 0 && config.encoder_inc_actions.find(encoder.config_name) != config.encoder_inc_actions.end())
		{
			Logger(TLogLevel::logTRACE) << "encoder " << encoder.config_name << " incremented (" << (int)read_buffer_old[encoder.reg_index] << "->" << (int)read_buffer[encoder.reg_index] << ") d = " << encoder_delta << std::endl;
			while (encoder_delta >= encoder.pulse_per_click) {
				for (auto &act : config.encoder_inc_actions[encoder.config_name]) 
				{
					ActionQueue::get_instance()->push(act);
				}
				encoder_delta -= encoder.pulse_per_click;
			}			
		}
		else if (encoder_delta < 0 && config.encoder_dec_actions.find(encoder.config_name) != config.encoder_dec_actions.end())
		{
			Logger(TLogLevel::logTRACE) << "encoder " << encoder.config_name << " decremented (" << (int)read_buffer_old[encoder.reg_index] << "->" << (int)read_buffer[encoder.reg_index] << ") d = " << encoder_delta << std::endl;
			while (abs(encoder_delta) >= encoder.pulse_per_click) {
				for (auto &act : config.encoder_dec_actions[encoder.config_name])
				{
					ActionQueue::get_instance()->push(act);
				}
				encoder_delta += encoder.pulse_per_click;
			}
		}
		encoder.accumulated_change = encoder_delta;
		Logger(TLogLevel::logTRACE) << "encoder " << encoder.config_name << " accumulated change = " << encoder.accumulated_change << std::endl;
	}
}
