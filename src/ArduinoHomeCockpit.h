#pragma once
#include <stdlib.h>
#include "UsbHidDevice.h"

class ArduinoHomeCockpit :public UsbHidDevice
{
private:
	std::vector<PanelButton> arduino_buttons;
public:
	ArduinoHomeCockpit(DeviceConfiguration& config);
	int connect();
	void start();
	void stop(int timeout);
};

