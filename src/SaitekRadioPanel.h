#pragma once
#include <vector>
#include "device.h"
#include "UsbHidDevice.h"
#include "configuration.h"

class SaitekRadioPanel : public UsbHidDevice
{
private:
	std::vector<PanelButton> radio_buttons;
	std::vector<PanelButton> radio_selectors;
	std::vector<PanelDisplay> radio_displays;
public:
	SaitekRadioPanel(DeviceConfiguration& config);
	int connect();
	void start();
	void stop(int timeout);
};

