/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <vector>
#include "Device.h"
#include "UsbHidDevice.h"
#include "configuration.h"

class SaitekMultiPanel : public UsbHidDevice
{
private:
	std::vector<PanelButton> multi_buttons;
	std::vector<PanelButton> multi_selectors;
	std::vector<PanelLight> multi_lights;
	std::vector<PanelDisplay> multi_displays;
public:
	SaitekMultiPanel(DeviceConfiguration &config);
	int connect();
	void start();
	void stop(int timeout);
};
