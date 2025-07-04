/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <cstring>
#include <cstdlib>
#include <string>
#include "trc-1000/TRC1000.h"
#include "trc-1000/TRC1000PFD.h"
#include "core/UsbHidDevice.h"
#include "core/Logger.h"

constexpr int WRITE_BUFFER_SIZE = 8 + 1; // +1 for the hid report id at position 0
constexpr int READ_BUFFER_SIZE = 23;
constexpr uint8_t CMD_REQUEST_BUTTON_SATE = 0x41;
constexpr uint8_t CMD_REQUEST_ENCODER_0 = 0x42;
constexpr uint8_t CMD_REQUEST_ENCODER_1 = 0x43;
constexpr uint8_t RESPONSE_BUTTON_STATE = 0xc1;
constexpr uint8_t RESPONSE_ENCODER_0 = 0xc2;
constexpr uint8_t RESPONSE_ENCODER_1 = 0xc3;
constexpr int BUTTON_BUFFER_OFFSET = 0;
constexpr int ENCODER_0_BUFFER_OFFSET = 7;
constexpr int ENCODER_1_BUFFER_OFFSET = 15;

TRC1000PFD::TRC1000PFD(ClassConfiguration& config) :TRC1000(config, READ_BUFFER_SIZE, WRITE_BUFFER_SIZE)
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
	trc1000pfd_buttons.push_back(PanelButton(4 * 8 + 6, "ENT"));
	trc1000pfd_buttons.push_back(PanelButton(4 * 8 + 7, "CLR"));

	trc1000pfd_buttons.push_back(PanelButton(5 * 8 + 0, "SWITCH_NAV_12"));
	trc1000pfd_buttons.push_back(PanelButton(5 * 8 + 1, "PRESS_ALT"));
	trc1000pfd_buttons.push_back(PanelButton(5 * 8 + 2, "SWITCH_COM_12"));
	trc1000pfd_buttons.push_back(PanelButton(5 * 8 + 3, "SEL_CRS"));
	trc1000pfd_buttons.push_back(PanelButton(5 * 8 + 4, "CURSOR"));
	trc1000pfd_buttons.push_back(PanelButton(5 * 8 + 5, "ID"));
	trc1000pfd_buttons.push_back(PanelButton(5 * 8 + 6, "SYNC_HDG"));
	trc1000pfd_buttons.push_back(PanelButton(5 * 8 + 7, "SQ"));

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
	trc1000pfd_encoders.push_back(PanelRotaryEncoder(ENCODER_1_BUFFER_OFFSET + 4, "NAV_VOL"));
	trc1000pfd_encoders.push_back(PanelRotaryEncoder(ENCODER_1_BUFFER_OFFSET + 5, "HDG"));
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