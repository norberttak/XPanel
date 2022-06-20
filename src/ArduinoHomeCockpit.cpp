#include <stdlib.h>
#include "ArduinoHomeCockpit.h"
#include "UsbHidDevice.h"

ArduinoHomeCockpit::ArduinoHomeCockpit(DeviceConfiguration& config) :UsbHidDevice(config, 9, 9)
{
	// register B
	arduino_buttons.push_back(PanelButton(8, "STROBE"));
	arduino_buttons.push_back(PanelButton(9, "DOME"));
	arduino_buttons.push_back(PanelButton(10, "LAND"));
	arduino_buttons.push_back(PanelButton(11, "TAXI"));
	arduino_buttons.push_back(PanelButton(12, "BEACON"));
	arduino_buttons.push_back(PanelButton(13, "CAUTION"));
	arduino_buttons.push_back(PanelButton(14, "START_L"));
	arduino_buttons.push_back(PanelButton(15, "WARNING"));
	// register C
	arduino_buttons.push_back(PanelButton(16, "START_R"));
	arduino_buttons.push_back(PanelButton(17, "PITOT"));
	arduino_buttons.push_back(PanelButton(18, "SEAT_BELT"));
	arduino_buttons.push_back(PanelButton(19, "PROP_SYNC"));
	arduino_buttons.push_back(PanelButton(20, "AUTO_COARS"));
	arduino_buttons.push_back(PanelButton(21, "XPDR"));

	register_buttons(arduino_buttons);
}

int ArduinoHomeCockpit::connect()
{
	return UsbHidDevice::connect();
}

void ArduinoHomeCockpit::start()
{
	UsbHidDevice::start();
}

void ArduinoHomeCockpit::stop(int timeout)
{
	UsbHidDevice::stop(timeout);
}