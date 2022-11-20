/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <cstring>
#include <cstdlib>
#include <string>
#include "TRC1000PFD.h"
#include "UsbHidDevice.h"
#include "Logger.h"

#define WRITE_BUFFER_SIZE (8+1) // +1 for the hid report id at position 0
#define READ_BUFFER_SIZE 23
#define HID_REPORT_ID	1

#define CMD_REQUEST_BUTTON_SATE 0x41
#define CMD_REQUEST_ENCODER_0	0x42
#define CMD_REQUEST_ENCODER_1	0x43

#define RESPONSE_BUTTON_STATE	0xc1
#define RESPONSE_ENCODER_0		0xc2
#define RESPONSE_ENCODER_1		0xc3

#define BUTTON_BUFFER_OFFSET	0
#define ENCODER_0_BUFFER_OFFSET	7
#define ENCODER_1_BUFFER_OFFSET	15

TRC1000PFD::TRC1000PFD(DeviceConfiguration& config) :UsbHidDevice(config, READ_BUFFER_SIZE, WRITE_BUFFER_SIZE)
{
	// cmd response oxc1				0
	trc1000pfd_buttons.push_back(PanelButton(1 * 8 + 0, "IAS"));
	trc1000pfd_buttons.push_back(PanelButton(1 * 8 + 1, "YD"));
	trc1000pfd_buttons.push_back(PanelButton(1 * 8 + 2, "AP"));
	trc1000pfd_buttons.push_back(PanelButton(1 * 8 + 3, "HDG"));
	trc1000pfd_buttons.push_back(PanelButton(1 * 8 + 4, "FD"));
	trc1000pfd_buttons.push_back(PanelButton(1 * 8 + 5, "APR"));
	trc1000pfd_buttons.push_back(PanelButton(1 * 8 + 6, "NAV"));
	trc1000pfd_buttons.push_back(PanelButton(1 * 8 + 7, "VNV"));

	trc1000pfd_buttons.push_back(PanelButton(2 * 8 + 0, "ALT"));
	trc1000pfd_buttons.push_back(PanelButton(2 * 8 + 1, "DN"));
	trc1000pfd_buttons.push_back(PanelButton(2 * 8 + 2, "VS"));
	trc1000pfd_buttons.push_back(PanelButton(2 * 8 + 3, "UP"));
	trc1000pfd_buttons.push_back(PanelButton(2 * 8 + 4, "FLIP_NAV"));
	trc1000pfd_buttons.push_back(PanelButton(2 * 8 + 5, "SOFT_KEY_1"));
	trc1000pfd_buttons.push_back(PanelButton(2 * 8 + 6, "SOFT_KEY_2"));
	trc1000pfd_buttons.push_back(PanelButton(2 * 8 + 7, "SOFT_KEY_3"));

	trc1000pfd_buttons.push_back(PanelButton(3 * 8 + 0, "SOFT_KEY_4"));
	trc1000pfd_buttons.push_back(PanelButton(3 * 8 + 1, "SOFT_KEY_5"));
	trc1000pfd_buttons.push_back(PanelButton(3 * 8 + 2, "SOFT_KEY_6"));
	trc1000pfd_buttons.push_back(PanelButton(3 * 8 + 3, "SOFT_KEY_7"));
	trc1000pfd_buttons.push_back(PanelButton(3 * 8 + 4, "SOFT_KEY_8"));
	trc1000pfd_buttons.push_back(PanelButton(3 * 8 + 5, "SOFT_KEY_9"));
	trc1000pfd_buttons.push_back(PanelButton(3 * 8 + 6, "SOFT_KEY_10"));
	trc1000pfd_buttons.push_back(PanelButton(3 * 8 + 7, "SOFT_KEY_11"));

	trc1000pfd_buttons.push_back(PanelButton(4 * 8 + 0, "SOFT_KEY_12"));
	trc1000pfd_buttons.push_back(PanelButton(4 * 8 + 1, "FLIP_COM"));
	trc1000pfd_buttons.push_back(PanelButton(4 * 8 + 2, "MENU"));
	trc1000pfd_buttons.push_back(PanelButton(4 * 8 + 3, "DIRECT"));
	trc1000pfd_buttons.push_back(PanelButton(4 * 8 + 4, "PROC"));
	trc1000pfd_buttons.push_back(PanelButton(4 * 8 + 5, "FPL"));
	trc1000pfd_buttons.push_back(PanelButton(4 * 8 + 6, "CLR"));
	trc1000pfd_buttons.push_back(PanelButton(4 * 8 + 7, "ENT"));

	trc1000pfd_buttons.push_back(PanelButton(5 * 8 + 0, "SWITCH_NAV_12"));
	trc1000pfd_buttons.push_back(PanelButton(5 * 8 + 1, "PRESS_ALT"));
	trc1000pfd_buttons.push_back(PanelButton(5 * 8 + 2, "SWITCH_COM_12"));
	trc1000pfd_buttons.push_back(PanelButton(5 * 8 + 3, "SEL_CRS"));
	trc1000pfd_buttons.push_back(PanelButton(5 * 8 + 4, "CURSOR"));
	trc1000pfd_buttons.push_back(PanelButton(5 * 8 + 5, "SQ"));
	trc1000pfd_buttons.push_back(PanelButton(5 * 8 + 6, "SYNC_HDG"));
	trc1000pfd_buttons.push_back(PanelButton(5 * 8 + 7, "ID"));

	trc1000pfd_buttons.push_back(PanelButton(6 * 8 + 0, "PAN_PUSH"));
	trc1000pfd_buttons.push_back(PanelButton(6 * 8 + 1, "PAN_UP"));
	trc1000pfd_buttons.push_back(PanelButton(6 * 8 + 2, "PAN_UP_LEFT"));
	trc1000pfd_buttons.push_back(PanelButton(6 * 8 + 3, "PAN_RIGHT"));
	trc1000pfd_buttons.push_back(PanelButton(6 * 8 + 4, "PAN_DOWN"));
	trc1000pfd_buttons.push_back(PanelButton(6 * 8 + 5, "PAN_DOWN_LEFT"));
	register_buttons(trc1000pfd_buttons);

	// cmd response 0xc2			ENCODER_0_BUFFER_OFFSET + 0
	trc1000pfd_encoders.push_back(PanelRotaryEncoder(ENCODER_0_BUFFER_OFFSET + 1, "NAV_INNER"));
	trc1000pfd_encoders.push_back(PanelRotaryEncoder(ENCODER_0_BUFFER_OFFSET + 2, "NAV_OUTER"));
	trc1000pfd_encoders.push_back(PanelRotaryEncoder(ENCODER_0_BUFFER_OFFSET + 3, "ALT_INNER"));
	trc1000pfd_encoders.push_back(PanelRotaryEncoder(ENCODER_0_BUFFER_OFFSET + 4, "ALT_OUTER"));
	trc1000pfd_encoders.push_back(PanelRotaryEncoder(ENCODER_0_BUFFER_OFFSET + 5, "COM_INNER"));
	trc1000pfd_encoders.push_back(PanelRotaryEncoder(ENCODER_0_BUFFER_OFFSET + 6, "COM_OUTER"));
	trc1000pfd_encoders.push_back(PanelRotaryEncoder(ENCODER_0_BUFFER_OFFSET + 7, "CRS"));

	// cmd response 0xc3			ENCODER_1_BUFFER_OFFSET + 0
	trc1000pfd_encoders.push_back(PanelRotaryEncoder(ENCODER_1_BUFFER_OFFSET + 1, "BARO"));
	trc1000pfd_encoders.push_back(PanelRotaryEncoder(ENCODER_1_BUFFER_OFFSET + 2, "FMS_INNER"));
	trc1000pfd_encoders.push_back(PanelRotaryEncoder(ENCODER_1_BUFFER_OFFSET + 3, "FMS_OUTER"));
	trc1000pfd_encoders.push_back(PanelRotaryEncoder(ENCODER_1_BUFFER_OFFSET + 4, "HDG"));
	trc1000pfd_encoders.push_back(PanelRotaryEncoder(ENCODER_1_BUFFER_OFFSET + 5, "NAV_VOL"));
	trc1000pfd_encoders.push_back(PanelRotaryEncoder(ENCODER_1_BUFFER_OFFSET + 6, "COM_VOL"));
	trc1000pfd_encoders.push_back(PanelRotaryEncoder(ENCODER_1_BUFFER_OFFSET + 7, "RANGE"));
	register_rotary_encoders(trc1000pfd_encoders);

	trc1000_commands.push_back(TRC1000Command(CMD_REQUEST_BUTTON_SATE, RESPONSE_BUTTON_STATE, BUTTON_BUFFER_OFFSET,7));
	trc1000_commands.push_back(TRC1000Command(CMD_REQUEST_ENCODER_0, RESPONSE_ENCODER_0, ENCODER_0_BUFFER_OFFSET,8));
	trc1000_commands.push_back(TRC1000Command(CMD_REQUEST_ENCODER_1, RESPONSE_ENCODER_1, ENCODER_1_BUFFER_OFFSET,8));
}

TRC1000PFD::~TRC1000PFD()
{
	//
}

int TRC1000PFD::send_command(unsigned char cmd)
{
	unsigned char command_buffer[WRITE_BUFFER_SIZE];
	command_buffer[0] = HID_REPORT_ID;
	command_buffer[1] = cmd;
	if (write_device(command_buffer, WRITE_BUFFER_SIZE) == EXIT_FAILURE)
	{
		Logger(TLogLevel::logERROR) << "TRC1000PFD send_command: error writing HID device. vid=" << vid << " pid=" << pid << std::endl;
		return EXIT_FAILURE;
	}
}

void TRC1000PFD::decode_read_response(unsigned char* tmp_read_buffer)
{
	switch (tmp_read_buffer[0]) {
	case RESPONSE_BUTTON_STATE:
		memcpy((void*)&read_buffer[BUTTON_BUFFER_OFFSET], tmp_read_buffer, 8);
		break;
	case RESPONSE_ENCODER_0:
		memcpy((void*)&read_buffer[ENCODER_0_BUFFER_OFFSET], tmp_read_buffer, 8);
		break;
	case RESPONSE_ENCODER_1:
		memcpy((void*)&read_buffer[ENCODER_1_BUFFER_OFFSET], tmp_read_buffer, 8);
		break;
	default:
		break;
	}
}

int TRC1000PFD::read_all_device_registers()
{
	unsigned char tmp_read_buffer[8];
	
	send_command(CMD_REQUEST_BUTTON_SATE);
	if (read_device(&tmp_read_buffer[0], 8) == EXIT_FAILURE)
	{
		Logger(TLogLevel::logERROR) << "TRC1000PFD: error reading HID device. vid=" << vid << " pid=" << pid << std::endl;
		return EXIT_FAILURE;
	}
	decode_read_response(&tmp_read_buffer[0]);

	send_command(CMD_REQUEST_ENCODER_0);
	if (read_device(&tmp_read_buffer[0], 8) == EXIT_FAILURE)
	{
		Logger(TLogLevel::logERROR) << "TRC1000PFD: error reading HID device. vid=" << vid << " pid=" << pid << std::endl;
		return EXIT_FAILURE;
	}
	decode_read_response(&tmp_read_buffer[0]);

	send_command(CMD_REQUEST_ENCODER_1);
	if (read_device(&tmp_read_buffer[0], 8) == EXIT_FAILURE)
	{
		Logger(TLogLevel::logERROR) << "TRC1000PFD: error reading HID device. vid=" << vid << " pid=" << pid << std::endl;
		return EXIT_FAILURE;
	}
	decode_read_response(&tmp_read_buffer[0]);

	return EXIT_SUCCESS;
}

void TRC1000PFD::thread_func()
{
	Logger(TLogLevel::logTRACE) << "TRC1000PFD::thread_func: started for vid=" << vid << " pid=" << pid << std::endl;
	memset(write_buffer, 0, write_buffer_size);
	bool write_buffer_changed = false;

	_thread_finish.store(false);

	// initial read for all registers:
	read_all_device_registers();
	memcpy(read_buffer_old, read_buffer, read_buffer_size);

	while (_thread_run.load() == true)
	{
		std::this_thread::sleep_for(20ms);

		write_buffer_changed = false;
		/*write_buffer_changed |= updateLightStates();

		if (write_buffer_changed)
		{
			if (write_device(write_buffer, write_buffer_size) == EXIT_FAILURE)
			{
				Logger(TLogLevel::logERROR) << "TRC1000PFD thread_func: error writing HID device. vid=" << vid << " pid=" << pid << std::endl;
				break;
			}
		}
		*/

		if (read_all_device_registers() != EXIT_SUCCESS)
		{
			Logger(TLogLevel::logERROR) << "TRC1000PFD: terminating thread due to USB communication error. vid=" << vid << " pid=" << pid << std::endl;
			break;
		}

		process_and_store_button_states();
		process_and_store_encoder_rotations();

		memcpy(read_buffer_old, read_buffer, read_buffer_size);
	}
	_thread_finish.store(true);
	Logger(TLogLevel::logDEBUG) << "TRC1000PFD thread_func: exit vid=" << vid << " pid=" << pid << std::endl;
}

int TRC1000PFD::connect()
{
	if (UsbHidDevice::connect() != EXIT_SUCCESS)
	{
		Logger(TLogLevel::logERROR) << "TRC1000PFD connect. Error during connect" << std::endl;
		return EXIT_FAILURE;
	}

	Logger(TLogLevel::logDEBUG) << "TRC1000PFD connect. successful" << std::endl;
	return EXIT_SUCCESS;
}

void TRC1000PFD::start()
{
	Logger(TLogLevel::logDEBUG) << "TRC1000PFD::start called" << std::endl;
	UsbHidDevice::start();
}

void TRC1000PFD::stop(int timeout)
{
	Logger(TLogLevel::logDEBUG) << "TRC1000PFD::stop called" << std::endl;
	UsbHidDevice::stop(timeout);
}
