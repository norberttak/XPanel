#include <stdlib.h>
#include "UsbHidDevice.h"
#include "SaitekMultiPanel.h"
#include <thread>

std::atomic<bool> _thread_run(FALSE);

SaitekMultiPanel::SaitekMultiPanel(Configuration &config):UsbHidDevice(config)
{	
	memset((void *)multibuf, 0, sizeof(multibuf));
	multi_buttons.push_back(MultiPanelButton(0, "AP"));
	multi_buttons.push_back(MultiPanelButton(7, "ALT"));
	multi_buttons.push_back(MultiPanelButton(6, "VS"));
	multi_buttons.push_back(MultiPanelButton(5, "IAS"));	
	multi_buttons.push_back(MultiPanelButton(4, "HDG"));
	multi_buttons.push_back(MultiPanelButton(3, "CRS"));
	multi_buttons.push_back(MultiPanelButton(8, "AUTO_THROTTLE"));
	multi_buttons.push_back(MultiPanelButton(0, "AP"));
	multi_buttons.push_back(MultiPanelButton(15, "HDG"));
	multi_buttons.push_back(MultiPanelButton(14, "NAV"));
	multi_buttons.push_back(MultiPanelButton(13, "IAS"));
	multi_buttons.push_back(MultiPanelButton(12, "ALT"));
	multi_buttons.push_back(MultiPanelButton(11, "VS"));
	multi_buttons.push_back(MultiPanelButton(10, "APR"));
	multi_buttons.push_back(MultiPanelButton(9, "REV"));
	multi_buttons.push_back(MultiPanelButton(23, "FLAPS_UP"));
	multi_buttons.push_back(MultiPanelButton(22, "FLAPS_DOWN"));
	multi_buttons.push_back(MultiPanelButton(20, "TRIM_WHEEL_UP"));
	multi_buttons.push_back(MultiPanelButton(21, "TRIM_WHEEL_DN"));
	multi_buttons.push_back(MultiPanelButton(2, "ADJUSTMENT_UP"));
	multi_buttons.push_back(MultiPanelButton(1, "ADJUSTMENT_DN"));
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

bool SaitekMultiPanel::is_bit_changed(unsigned char* buf, unsigned char* buf_old, int bit)
{
	return ((*(unsigned __int32*)buf) & ((unsigned __int32)0x00000001) << bit) != (*((unsigned __int32*)buf_old) & ((unsigned __int32)0x00000001) << bit);
}

bool SaitekMultiPanel::bit_value(unsigned char* buf, int bit)
{
	return ((*((unsigned __int32*)buf) & ((unsigned __int32)0x00000001) << bit) == ((unsigned __int32)0x00000001) << bit);
}

void SaitekMultiPanel::thread_func()
{
	while (_thread_run.load() == TRUE)
	{
		if (read_device(multibuf, sizeof(multibuf)) == EXIT_FAILURE)
			continue;

		for (auto button : multi_buttons)
		{
			if (is_bit_changed(multibuf, multibuf_old, button.bit))
			{
				if (bit_value(multibuf, button.bit) && config.push_actions.find(button.config_name.c_str()) != config.push_actions.end())
					config.push_actions[button.config_name.c_str()].activate();
				else if (!bit_value(multibuf, button.bit) && config.release_actions.find(button.config_name.c_str()) != config.release_actions.end())
					config.release_actions[button.config_name.c_str()].activate();
			}
		}

		memcpy(multibuf_old, multibuf, sizeof(multibuf));
		std::this_thread::sleep_for(10ms);
	}
}

void SaitekMultiPanel::start()
{
	_thread_run.store(TRUE);
}

void SaitekMultiPanel::stop(int time_out)
{
	_thread_run.store(FALSE);
}