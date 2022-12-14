#pragma once
#include <vector>
#include "UsbHidDevice.h"

#define HID_REPORT_ID	1

struct TRC1000Command
{
	TRC1000Command(unsigned char _command_code, unsigned char _response_code, int _buffer_offset, int _byte_count)
	{
		command_code = _command_code;
		response_code = _response_code;
		buffer_offset = _buffer_offset;
		byte_count = _byte_count;
	}
	unsigned char command_code;
	unsigned char response_code;
	int buffer_offset;
	int byte_count;
};

class TRC1000 : public UsbHidDevice
{
private:
	unsigned char *command_buffer;
protected:
	std::vector<TRC1000Command> trc1000_commands;
	int send_command(unsigned char cmd);
	int decode_read_response(unsigned char* tmp_read_buffer);
	int read_all_device_registers();
	int read_buffer_size;
	int write_buffer_size;
public:
	TRC1000(DeviceConfiguration& config, int _read_buffer_size, int _write_buffer_size);
	~TRC1000();
	virtual void thread_func();
	int connect();
	void start();
	void stop(int timeout);
};