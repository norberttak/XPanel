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

class SaitekRadioPanel : public UsbHidDevice
{
private:
	std::vector<PanelButton> radio_buttons;
	std::vector<PanelButton> radio_selectors_up;
	std::vector<PanelButton> radio_selectors_down;
	std::vector<PanelDisplay> radio_displays;
public:
	SaitekRadioPanel(ClassConfiguration& config);
	int connect();
	int connect(hid_device* _device_handle);
	void start();
	void stop(int timeout);
};
