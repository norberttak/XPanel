/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <cstring>
#include <cstdlib>
#include "UsbHidDevice.h"
#include "SaitekRadioPanel.h"
#include "Logger.h"

#define WRITE_BUFFER_SIZE 23
#define READ_BUFFER_SIZE 5

SaitekRadioPanel::SaitekRadioPanel(DeviceConfiguration& config) :UsbHidDevice(config, READ_BUFFER_SIZE, WRITE_BUFFER_SIZE)
{

	// mode selector switch
	radio_selectors.push_back(PanelButton(0 * 8 + 0, "SW_UP_COM1"));
	radio_selectors.push_back(PanelButton(0 * 8 + 1, "SW_UP_COM2"));
	radio_selectors.push_back(PanelButton(0 * 8 + 2, "SW_UP_NAV1"));
	radio_selectors.push_back(PanelButton(0 * 8 + 3, "SW_UP_NAV2"));
	radio_selectors.push_back(PanelButton(0 * 8 + 4, "SW_UP_ADF"));
	radio_selectors.push_back(PanelButton(0 * 8 + 5, "SW_UP_DME"));
	radio_selectors.push_back(PanelButton(0 * 8 + 6, "SW_UP_IDT"));

	radio_selectors.push_back(PanelButton(0 * 8 + 7, "SW_DOWN_COM1"));
	radio_selectors.push_back(PanelButton(1 * 8 + 0, "SW_DOWN_COM2"));
	radio_selectors.push_back(PanelButton(1 * 8 + 1, "SW_DOWN_NAV1"));
	radio_selectors.push_back(PanelButton(1 * 8 + 2, "SW_DOWN_NAV2"));
	radio_selectors.push_back(PanelButton(1 * 8 + 3, "SW_DOWN_ADF"));
	radio_selectors.push_back(PanelButton(1 * 8 + 4, "SW_DOWN_DME"));
	radio_selectors.push_back(PanelButton(1 * 8 + 5, "SW_DOWN_IDT"));
	register_selectors(radio_selectors);

	// buttons & rotation knobs
	radio_buttons.push_back(PanelButton(2 * 8 + 2, "KNOB_UP_BIG_PLUS"));
	radio_buttons.push_back(PanelButton(2 * 8 + 3, "KNOB_UP_BIG_MINUS"));
	radio_buttons.push_back(PanelButton(2 * 8 + 0, "KNOB_UP_SMALL_PLUS"));
	radio_buttons.push_back(PanelButton(2 * 8 + 1, "KNOB_UP_SMALL_MINUS"));
	radio_buttons.push_back(PanelButton(2 * 8 + 6, "KNOB_DOWN_BIG_PLUS"));
	radio_buttons.push_back(PanelButton(2 * 8 + 7, "KNOB_DOWN_BIG_MINUS"));
	radio_buttons.push_back(PanelButton(2 * 8 + 4, "KNOB_DOWN_SMALL_PLUS"));
	radio_buttons.push_back(PanelButton(2 * 8 + 5, "KNOB_DOWN_SMALL_MINUS"));
	radio_buttons.push_back(PanelButton(1 * 8 + 6, "ACT_STBY_UP"));
	radio_buttons.push_back(PanelButton(1 * 8 + 7, "ACT_STBY_DOWN"));
	register_buttons(radio_buttons);

	int display_width = 5;

	radio_displays.push_back(PanelDisplay(6, display_width, "RADIO_DISPLAY_STBY_UP"));
	radio_displays.push_back(PanelDisplay(1, display_width, "RADIO_DISPLAY_ACTIVE_UP"));
	radio_displays.push_back(PanelDisplay(16, display_width, "RADIO_DISPLAY_STBY_DOWN"));
	radio_displays.push_back(PanelDisplay(11, display_width, "RADIO_DISPLAY_ACTIVE_DOWN"));

	register_displays(radio_displays);

	for (auto config_display : config.multi_displays)
	{
		config_display.second->set_nr_bytes(display_width);
	}
}

int SaitekRadioPanel::connect()
{
	if (UsbHidDevice::connect() != EXIT_SUCCESS)
	{
		Logger(TLogLevel::logERROR) << "SaitekRadioPanel connect. Error during connect" << std::endl;
		return EXIT_FAILURE;
	}

	unsigned char buff[WRITE_BUFFER_SIZE];
	memset(buff, 0xff, sizeof(buff)); // clear all displays
	if (write_device(buff, sizeof(buff)) != EXIT_SUCCESS)
	{
		Logger(TLogLevel::logERROR) << "SaitekRadioPanel connect. error in write_device" << std::endl;
		return EXIT_FAILURE;
	}

	Logger(TLogLevel::logDEBUG) << "SaitekRadioPanel connect. successful" << std::endl;
	return EXIT_SUCCESS;
}

void SaitekRadioPanel::start()
{
	Logger(TLogLevel::logDEBUG) << "SaitekRadioPanel::start called" << std::endl;
	UsbHidDevice::start();
}

void SaitekRadioPanel::stop(int timeout)
{
	Logger(TLogLevel::logDEBUG) << "SaitekRadioPanel::stop called" << std::endl;

	// Blank the display before exit
	unsigned char buff[WRITE_BUFFER_SIZE];
	memset(buff, 0xFF, sizeof(buff));
	buff[0] = 0;

	if (write_device(buff, sizeof(buff)) != EXIT_SUCCESS)
	{
		Logger(TLogLevel::logERROR) << "SaitekRadioPanel stop. error in write_device" << std::endl;
		return;
	}

	UsbHidDevice::stop(timeout);
}
