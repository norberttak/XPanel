#pragma once
#include <vector>
#include "device.h"
#include "UsbHidDevice.h"
#include "configuration.h"

class SaitekMultiPanel : public UsbHidDevice
{
private:
	std::vector<PanelButton> multi_buttons;
public:
	SaitekMultiPanel(Configuration &config);
	int connect();
	void start();
	void stop(int timeout);
};

