/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <cstring>
#include <cstdlib>
#include "UsbHidDevice.h"
#include "SaitekSwitchPanel.h"
#include "Logger.h"

#define WRITE_BUFFER_SIZE 2
#define READ_BUFFER_SIZE 4

SaitekSwitchPanel::SaitekSwitchPanel(DeviceConfiguration& config) :UsbHidDevice(config, READ_BUFFER_SIZE, WRITE_BUFFER_SIZE)
{
	// push buttons
	switch_buttons.push_back(PanelButton((0 * 8) + 0, "BATTERY"));
	switch_buttons.push_back(PanelButton((0 * 8) + 1, "ALTERNATOR"));
	switch_buttons.push_back(PanelButton((0 * 8) + 2, "AVIONICS"));
	switch_buttons.push_back(PanelButton((0 * 8) + 3, "FUEL_PUMP"));
	switch_buttons.push_back(PanelButton((0 * 8) + 4, "DE_ICE"));
	switch_buttons.push_back(PanelButton((0 * 8) + 5, "PITOT_HEAT"));
	switch_buttons.push_back(PanelButton((0 * 8) + 6, "COWL_FLAPS"));
	switch_buttons.push_back(PanelButton((0 * 8) + 7, "PANEL_LIGHTS"));

	switch_buttons.push_back(PanelButton((1 * 8) + 0, "BEACON"));
	switch_buttons.push_back(PanelButton((1 * 8) + 1, "NAV"));
	switch_buttons.push_back(PanelButton((1 * 8) + 2, "STROBE"));
	switch_buttons.push_back(PanelButton((1 * 8) + 3, "TAXI"));
	switch_buttons.push_back(PanelButton((1 * 8) + 4, "LANDING"));
	switch_buttons.push_back(PanelButton((1 * 8) + 5, "MAG_OFF"));
	switch_buttons.push_back(PanelButton((1 * 8) + 6, "MAG_RIGHT"));
	switch_buttons.push_back(PanelButton((1 * 8) + 7, "MAG_LEFT"));

	switch_buttons.push_back(PanelButton((2 * 8) + 0, "MAG_BOTH"));
	switch_buttons.push_back(PanelButton((2 * 8) + 1, "ENG_START"));
	switch_buttons.push_back(PanelButton((2 * 8) + 2, "GEAR_UP"));
	switch_buttons.push_back(PanelButton((2 * 8) + 3, "GEAR_DOWN"));

	register_buttons(switch_buttons);

	switch_lights.push_back(PanelLight((1 * 8) + 0, "GEAR_NOSE_GREEN"));
	switch_lights.push_back(PanelLight((1 * 8) + 1, "GEAR_LEFT_GREEN"));
	switch_lights.push_back(PanelLight((1 * 8) + 2, "GEAR_RIGHT_GREEN"));

	switch_lights.push_back(PanelLight((1 * 8) + 3, "GEAR_NOSE_RED"));
	switch_lights.push_back(PanelLight((1 * 8) + 4, "GEAR_LEFT_RED"));
	switch_lights.push_back(PanelLight((1 * 8) + 5, "GEAR_RIGHT_RED"));
	/* Yellow color lights can be turned on by issuing green + red */

	register_lights(switch_lights);
}

int SaitekSwitchPanel::connect()
{
	unsigned char buff[WRITE_BUFFER_SIZE];
	if (UsbHidDevice::connect() != EXIT_SUCCESS)
	{
		Logger(TLogLevel::logERROR) << "SaitekSwitchPanel connect. Error during connect" << std::endl;
		return EXIT_FAILURE;
	}

	memset(buff, 0, sizeof(buff)); // clear all LED lights
	if (write_device(buff, sizeof(buff)) != EXIT_SUCCESS)
	{
		Logger(TLogLevel::logERROR) << "SaitekSwitchPanel connect. error in write_device" << std::endl;
		return EXIT_FAILURE;
	}

	Logger(TLogLevel::logDEBUG) << "SaitekSwitchPanel connect. successful" << std::endl;
	return EXIT_SUCCESS;
}

void SaitekSwitchPanel::start()
{
	Logger(TLogLevel::logDEBUG) << "SaitekSwitchPanel::start called" << std::endl;
	UsbHidDevice::start();
}

void SaitekSwitchPanel::stop(int timeout)
{
	Logger(TLogLevel::logDEBUG) << "SaitekSwitchPanel::stop called" << std::endl;

	// Blank LED lights before exit
	unsigned char buff[WRITE_BUFFER_SIZE];
	memset(buff, 0, sizeof(buff)); // clear all LED lights
	if (write_device(buff, sizeof(buff)) != EXIT_SUCCESS)
	{
		Logger(TLogLevel::logERROR) << "SaitekSwitchPanel stop. error in write_device" << std::endl;
		return;
	}

	UsbHidDevice::stop(timeout);
}
