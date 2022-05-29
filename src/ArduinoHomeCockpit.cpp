#include <stdlib.h>
#include "ArduinoHomeCockpit.h"
#include "UsbHidDevice.h"

ArduinoHomeCockpit::ArduinoHomeCockpit(Configuration& config) :UsbHidDevice(config, 9)
{
	arduino_buttons.push_back(PanelButton(0, "STROBE"));
	arduino_buttons.push_back(PanelButton(1, "DOME"));
	arduino_buttons.push_back(PanelButton(2, "LAND"));
	arduino_buttons.push_back(PanelButton(3, "TAXI"));
	arduino_buttons.push_back(PanelButton(4, "BEACON"));
	arduino_buttons.push_back(PanelButton(5, "CAUTION"));
	arduino_buttons.push_back(PanelButton(6, "START_L"));
	arduino_buttons.push_back(PanelButton(7, "WARNING"));
	arduino_buttons.push_back(PanelButton(8, "START_R"));
	arduino_buttons.push_back(PanelButton(9, "PITOT"));
	arduino_buttons.push_back(PanelButton(10, "SEAT_BELT"));
	arduino_buttons.push_back(PanelButton(11, "PROP_SYNC"));
	arduino_buttons.push_back(PanelButton(12, "AUTO_COARS"));
	arduino_buttons.push_back(PanelButton(13, "XPDR"));

	register_buttons(arduino_buttons);
}

int ArduinoHomeCockpit::connect()
{
	return UsbHidDevice::connect();
}

