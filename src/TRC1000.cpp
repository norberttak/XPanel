/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <cstring>
#include "TRC1000.h"
#include "Logger.h"

TRC1000::TRC1000(DeviceConfiguration& config, int _read_buffer_size, int _write_buffer_size) :UsbHidDevice(config, _read_buffer_size, _write_buffer_size)
{
	read_buffer_size = _read_buffer_size;
	write_buffer_size = _write_buffer_size;
	command_buffer = (unsigned char*)calloc(write_buffer_size, sizeof(unsigned char));
}

TRC1000::~TRC1000()
{
	free(command_buffer);
}

int TRC1000::send_command(unsigned char cmd)
{
	command_buffer[0] = HID_REPORT_ID;
	command_buffer[1] = cmd;
	if (write_device(command_buffer, write_buffer_size) == EXIT_FAILURE)
	{
		Logger(TLogLevel::logERROR) << "TRC1000 send_command: error writing HID device. vid=" << vid << " pid=" << pid << std::endl;
		return EXIT_FAILURE;
	}
}

void TRC1000::decode_read_response(unsigned char* tmp_read_buffer)
{
	for (auto command : trc1000_commands)
	{
		if (command.response_code == tmp_read_buffer[0])
			memcpy((void*)&read_buffer[command.buffer_offset], tmp_read_buffer, 8);
	}
}

int TRC1000::read_all_device_registers()
{
	unsigned char tmp_read_buffer[8];

	for (auto command : trc1000_commands)
	{
		send_command(command.command_code);
		if (read_device(&tmp_read_buffer[0], 8) == EXIT_FAILURE)
		{
			Logger(TLogLevel::logERROR) << "TRC1000: error reading HID device. vid=" << vid << " pid=" << pid << std::endl;
			return EXIT_FAILURE;
		}
		decode_read_response(&tmp_read_buffer[0]);
	}

	return EXIT_SUCCESS;
}

void TRC1000::thread_func()
{
	Logger(TLogLevel::logTRACE) << "TRC1000::thread_func: started for vid=" << vid << " pid=" << pid << std::endl;
	memset(write_buffer, 0, write_buffer_size);
	bool write_buffer_changed = false;

	_thread_finish.store(false);

	// initial read for all registers:
	read_all_device_registers();
	memcpy(read_buffer_old, read_buffer, read_buffer_size);

	memset(write_buffer, 0, write_buffer_size);
	write_buffer[0] = HID_REPORT_ID;
	write_buffer[1] = 0x01; //set LED command

	while (_thread_run.load() == true)
	{
		std::this_thread::sleep_for(20ms);

		// Update LED states
		write_buffer_changed = false;
		write_buffer_changed |= updateLightStates();
		if (write_buffer_changed)
		{
			if (write_device(write_buffer, write_buffer_size) == EXIT_FAILURE)
			{
				Logger(TLogLevel::logERROR) << "TRC1000 thread_func: error writing HID device. vid=" << vid << " pid=" << pid << std::endl;
				break;
			}
		}

		// Read button & encoder states
		if (read_all_device_registers() != EXIT_SUCCESS)
		{
			Logger(TLogLevel::logERROR) << "TRC1000: terminating thread due to USB communication error. vid=" << vid << " pid=" << pid << std::endl;
			break;
		}

		process_and_store_button_states();
		process_and_store_encoder_rotations();

		memcpy(read_buffer_old, read_buffer, read_buffer_size);
	}
	_thread_finish.store(true);
	Logger(TLogLevel::logDEBUG) << "TRC1000 thread_func: exit vid=" << vid << " pid=" << pid << std::endl;
}

int TRC1000::connect()
{
	if (UsbHidDevice::connect() != EXIT_SUCCESS)
	{
		Logger(TLogLevel::logERROR) << "TRC1000 connect. Error during connect vid=" << vid << " pid=" << pid << std::endl;
		return EXIT_FAILURE;
	}

	Logger(TLogLevel::logDEBUG) << "TRC1000 connect. successful vid=" << vid << " pid=" << pid << std::endl;
	return EXIT_SUCCESS;
}

void TRC1000::start()
{
	Logger(TLogLevel::logDEBUG) << "TRC1000::start called vid=" << vid << " pid=" << pid << std::endl;
	UsbHidDevice::start();
}

void TRC1000::stop(int timeout)
{
	Logger(TLogLevel::logDEBUG) << "TRC1000::stop called vid=" << vid << " pid=" << pid << std::endl;
	UsbHidDevice::stop(timeout);
}