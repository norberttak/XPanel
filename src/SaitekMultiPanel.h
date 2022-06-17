#pragma once
#include <vector>
#include "device.h"
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
	SaitekMultiPanel(Configuration &config);
	int connect();
	void start();
	void stop(int timeout);
};

