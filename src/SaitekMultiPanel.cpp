#include <stdlib.h>
#include "UsbHidDevice.h"
#include "SaitekMultiPanel.h"

SaitekMultiPanel::SaitekMultiPanel(Configuration &config):UsbHidDevice(config,4)
{	
	multi_buttons.push_back(PanelButton(0, "AP"));
	multi_buttons.push_back(PanelButton(7, "ALT"));
	multi_buttons.push_back(PanelButton(6, "VS"));
	multi_buttons.push_back(PanelButton(5, "IAS"));	
	multi_buttons.push_back(PanelButton(4, "HDG"));
	multi_buttons.push_back(PanelButton(3, "CRS"));
	multi_buttons.push_back(PanelButton(8, "AUTO_THROTTLE"));
	multi_buttons.push_back(PanelButton(0, "AP"));
	multi_buttons.push_back(PanelButton(15, "HDG"));
	multi_buttons.push_back(PanelButton(14, "NAV"));
	multi_buttons.push_back(PanelButton(13, "IAS"));
	multi_buttons.push_back(PanelButton(12, "ALT"));
	multi_buttons.push_back(PanelButton(11, "VS"));
	multi_buttons.push_back(PanelButton(10, "APR"));
	multi_buttons.push_back(PanelButton(9, "REV"));
	multi_buttons.push_back(PanelButton(23, "FLAPS_UP"));
	multi_buttons.push_back(PanelButton(22, "FLAPS_DOWN"));
	multi_buttons.push_back(PanelButton(20, "TRIM_WHEEL_UP"));
	multi_buttons.push_back(PanelButton(21, "TRIM_WHEEL_DN"));
	multi_buttons.push_back(PanelButton(2, "ADJUSTMENT_UP"));
	multi_buttons.push_back(PanelButton(1, "ADJUSTMENT_DN"));

	register_buttons(multi_buttons);
}

int SaitekMultiPanel::connect()
{
	unsigned char read_buff[13];
	UsbHidDevice::connect();

	if (hid_read(device_handle, read_buff, sizeof(read_buff)) == -1)
		return EXIT_FAILURE;

	if (hid_send_feature_report(device_handle, read_buff, sizeof(read_buff)))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}