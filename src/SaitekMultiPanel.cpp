/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <cstring>
#include <cstdlib>
#include "UsbHidDevice.h"
#include "SaitekMultiPanel.h"
#include "Logger.h"

#define WRITE_BUFFER_SIZE 13
#define READ_BUFFER_SIZE 4

SaitekMultiPanel::SaitekMultiPanel(DeviceConfiguration& config) :UsbHidDevice(config, READ_BUFFER_SIZE, WRITE_BUFFER_SIZE)
{

	// mode selector switch
	multi_selectors.push_back(PanelButton(0, "SW_ALT"));
	multi_selectors.push_back(PanelButton(1, "SW_VS"));
	multi_selectors.push_back(PanelButton(2, "SW_IAS"));
	multi_selectors.push_back(PanelButton(3, "SW_HDG"));
	multi_selectors.push_back(PanelButton(4, "SW_CRS"));
	register_selectors(multi_selectors);

	// push buttons
	multi_buttons.push_back(PanelButton(5, "KNOB_PLUS"));
	multi_buttons.push_back(PanelButton(6, "KNOB_MINUS"));
	multi_buttons.push_back(PanelButton(7, "AP"));
	multi_buttons.push_back(PanelButton(8, "HDG"));
	multi_buttons.push_back(PanelButton(9, "NAV"));
	multi_buttons.push_back(PanelButton(10, "IAS"));
	multi_buttons.push_back(PanelButton(11, "ALT"));
	multi_buttons.push_back(PanelButton(12, "VS"));
	multi_buttons.push_back(PanelButton(13, "APR"));
	multi_buttons.push_back(PanelButton(14, "REV"));
	multi_buttons.push_back(PanelButton(15, "AUTO_THROTTLE"));
	multi_buttons.push_back(PanelButton(16, "FLAPS_UP"));
	multi_buttons.push_back(PanelButton(17, "FLAPS_DOWN"));
	multi_buttons.push_back(PanelButton(18, "TRIM_WHEEL_DOWN"));
	multi_buttons.push_back(PanelButton(19, "TRIM_WHEEL_UP"));

	register_buttons(multi_buttons);

	multi_lights.push_back(PanelLight(11 * 8 + 0, "AP_L"));
	multi_lights.push_back(PanelLight(11 * 8 + 1, "HDG_L"));
	multi_lights.push_back(PanelLight(11 * 8 + 2, "NAV_L"));
	multi_lights.push_back(PanelLight(11 * 8 + 3, "IAS_L"));
	multi_lights.push_back(PanelLight(11 * 8 + 4, "ALT_L"));
	multi_lights.push_back(PanelLight(11 * 8 + 5, "VS_L"));
	multi_lights.push_back(PanelLight(11 * 8 + 6, "APR_L"));
	multi_lights.push_back(PanelLight(11 * 8 + 7, "REV_L"));

	register_lights(multi_lights);

	int display_width = 5;

	multi_displays.push_back(PanelDisplay(1, display_width, "MULTI_DISPLAY_UP"));
	multi_displays.push_back(PanelDisplay(6, display_width, "MULTI_DISPLAY_DOWN"));

	register_displays(multi_displays);

	for (auto &config_display : config.multi_displays)
	{
		config_display.second->set_nr_bytes(display_width);
	}
}

int SaitekMultiPanel::connect()
{
	unsigned char buff[WRITE_BUFFER_SIZE];
	if (UsbHidDevice::connect() != EXIT_SUCCESS)
	{
		Logger(TLogLevel::logERROR) << "SaitekRadioPanel connect. Error during connect" << std::endl;
		return EXIT_FAILURE;
	}

	read_device(read_buffer, read_buffer_size);
	memcpy(read_buffer_old, read_buffer, read_buffer_size);

	memset(buff, 0, sizeof(buff)); // clear all button lights
	if (send_feature_report(buff, sizeof(buff)) != EXIT_SUCCESS)
	{
		Logger(TLogLevel::logERROR) << "SaitekMultiPanel connect. error in write_device" << std::endl;
		return EXIT_FAILURE;
	}

	Logger(TLogLevel::logDEBUG) << "SaitekMultiPanel connect. successful" << std::endl;
	return EXIT_SUCCESS;
}

void SaitekMultiPanel::start()
{
	Logger(TLogLevel::logDEBUG) << "SaitekMultiPanel::start called" << std::endl;
	UsbHidDevice::start();
}

void SaitekMultiPanel::stop(int timeout)
{
	Logger(TLogLevel::logDEBUG) << "SaitekMultiPanel::stop called" << std::endl;

	// Blank the display before exit
	unsigned char buff[WRITE_BUFFER_SIZE] = {0,15,15,15,15,15,15,15,15,15,15,0,0};
	if (send_feature_report(buff, sizeof(buff)) != EXIT_SUCCESS)
	{
		Logger(TLogLevel::logERROR) << "SaitekMultiPanel stop. error in write_device" << std::endl;
		return;
	}

	UsbHidDevice::stop(timeout);
}
