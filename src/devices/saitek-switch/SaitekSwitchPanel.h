/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <vector>
#include "core/Device.h"
#include "core/UsbHidDevice.h"
#include "core/Configuration.h"

class SaitekSwitchPanel : public UsbHidDevice
{
private:
	std::vector<PanelButton> switch_buttons;
	std::vector<PanelLight> switch_lights;
public:
	SaitekSwitchPanel(DeviceConfiguration& config);
	int connect(hid_device* _device_handle);
	int connect();
	void start();
	void stop(int timeout);
};