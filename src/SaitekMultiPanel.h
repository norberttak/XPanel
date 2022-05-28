#pragma once
#include "device.h"
#include "UsbHidDevice.h"
#include "configuration.h"
#include <vector>
#include <atomic>

struct MultiPanelButton
{
	MultiPanelButton(int _bit, std::string _config_name)
	{
		bit = _bit;
		config_name = _config_name;
	}
	int bit;
	std::string config_name;
};

class SaitekMultiPanel : public UsbHidDevice
{
private:
	unsigned char multibuf[4];
	unsigned char multibuf_old[4];

	std::vector<MultiPanelButton> multi_buttons;
	bool bit_value(unsigned char* buffer, int bit);
	bool is_bit_changed(unsigned char* buffer, unsigned char* buffer_old, int bit);
public:
	SaitekMultiPanel(Configuration &config);
	int connect();
	void thread_func();
	void start();
	void stop(int time_out);
};

