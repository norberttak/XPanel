/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <atomic>
#include <mutex>
#include "core/Device.h"
#include "FIPDriver.h"
#include "FIPScreen.h"
class FIPDevice : public Device
{
private:
	FIP_device_handle* f_device;
	std::atomic<bool> _thread_run;
	std::atomic<bool> _thread_finish;
	std::vector<PanelButton> fip_buttons;
	std::vector<PanelLight> fip_lights;
	std::string page_condition_name_old;
	int page_index_old;
	void process_page_conditions(FIPScreen* screen);
public:
	FIPDevice(DeviceConfiguration& _config);
	void stop(int time_out_msec);
	int connect();
	void release();
	void start();
	void render_screen(FIPScreen* screen);
	void thread_func();
	void set_text(int page_index, std::string text);
	void set_led(int led_index, int led_state);
	uint16_t get_button_state();
};

