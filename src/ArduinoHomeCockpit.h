#pragma once
#include <stdlib.h>
#include "UsbHidDevice.h"

class ArduinoHomeCockpit :public UsbHidDevice
{
private:
	std::vector<PanelButton> arduino_buttons;
public:
	ArduinoHomeCockpit(Configuration& config);
	int connect();
};

