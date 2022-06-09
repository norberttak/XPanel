#include <stdlib.h>
#include "UsbHidDevice.h"
#include "SaitekMultiPanel.h"
#include "logger.h"

SaitekMultiPanel::SaitekMultiPanel(Configuration &config):UsbHidDevice(config,4, 13)
{	
	multi_buttons.push_back(PanelButton(0, "SW_ALT"));
	multi_buttons.push_back(PanelButton(1, "SW_VS"));
	multi_buttons.push_back(PanelButton(2, "SW_IAS"));
	multi_buttons.push_back(PanelButton(3, "SW_HDG"));
	multi_buttons.push_back(PanelButton(4, "SW_CRS"));
	multi_buttons.push_back(PanelButton(5, "KNOB_PLUS"));
	multi_buttons.push_back(PanelButton(6, "KNOB_MINUS"));
	multi_buttons.push_back(PanelButton(7, "AP"));
	multi_buttons.push_back(PanelButton(8, "HDG"));
	multi_buttons.push_back(PanelButton(9, "NAV"));
	multi_buttons.push_back(PanelButton(10, "IAS"));
	multi_buttons.push_back(PanelButton(11, "ALT"));
	multi_buttons.push_back(PanelButton(12, "VS"));
	multi_buttons.push_back(PanelButton(13, "APR"));
	multi_buttons.push_back(PanelButton(14, "REV"));
	multi_buttons.push_back(PanelButton(15, "AUTO_THROTTLE"));
	multi_buttons.push_back(PanelButton(16, "FLAPS_UP"));
	multi_buttons.push_back(PanelButton(17, "FLAPS_DOWN"));
	multi_buttons.push_back(PanelButton(18, "TRIM_WHEEL_DOWN"));
	multi_buttons.push_back(PanelButton(19, "TRIM_WHEEL_UP"));

	register_buttons(multi_buttons);

	multi_lights.push_back(PanelLight(11 * 8 + 0, "AP_L"));
	multi_lights.push_back(PanelLight(11 * 8 + 1, "HDG_L"));
	multi_lights.push_back(PanelLight(11 * 8 + 2, "NAV_L"));
	multi_lights.push_back(PanelLight(11 * 8 + 3, "IAS_L"));
	multi_lights.push_back(PanelLight(11 * 8 + 4, "ALT_L"));
	multi_lights.push_back(PanelLight(11 * 8 + 5, "VS_L"));
	multi_lights.push_back(PanelLight(11 * 8 + 6, "APR_L"));
	multi_lights.push_back(PanelLight(11 * 8 + 7, "REV_L"));

	register_lights(multi_lights);
}

int SaitekMultiPanel::connect()
{
	unsigned char buff[13];
	UsbHidDevice::connect();
	
	memset(buff, 0, sizeof(buff)); // clear all button lights
	if (write_device(buff, sizeof(buff)) != EXIT_SUCCESS)
	{
		Logger(TLogLevel::logERROR) << "SaitekMultiPanel connect. error in write_device" << std::endl;
		return EXIT_FAILURE;
	}

	Logger(TLogLevel::logDEBUG) << "SaitekMultiPanel connect. successful" << std::endl;
	return EXIT_SUCCESS;
}

void SaitekMultiPanel::start()
{
 	UsbHidDevice::start();
}

void SaitekMultiPanel::stop(int timeout)
{
	UsbHidDevice::stop(timeout);
}