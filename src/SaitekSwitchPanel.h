/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <vector>
#include "Device.h"
#include "UsbHidDevice.h"
#include "Configuration.h"

class SaitekSwitchPanel : public UsbHidDevice
{
private:
	std::vector<PanelButton> switch_buttons;
	std::vector<PanelLight> switch_lights;
public:
	SaitekSwitchPanel(DeviceConfiguration& config);
	int connect();
	void start();
	void stop(int timeout);
};