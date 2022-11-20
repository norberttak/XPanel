#pragma once
#include "UsbHidDevice.h"

class TRC1000PFD : public UsbHidDevice
{
private:
	std::vector<PanelButton> trc1000pfd_buttons;
	std::vector<PanelRotaryEncoder> trc1000pfd_encoders;
	int send_command(unsigned char cmd);
	void decode_read_response(unsigned char* tmp_read_buffer);
	int read_all_device_registers();
public:
	TRC1000PFD(DeviceConfiguration& config);
	~TRC1000PFD();
	virtual void thread_func();
	int connect();
	void start();
	void stop(int timeout);

};

