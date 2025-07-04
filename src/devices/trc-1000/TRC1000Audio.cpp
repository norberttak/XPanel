/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <cstring>
#include <cstdlib>
#include <string>
#include "trc-1000/TRC1000.h"
#include "trc-1000/TRC1000Audio.h"
#include "core/UsbHidDevice.h"
#include "core/Logger.h"
    
constexpr int WRITE_BUFFER_SIZE = 8 + 1; // +1 for the hid report id at position 0
constexpr int READ_BUFFER_SIZE = 8;
constexpr uint8_t CMD_REQUEST_BUTTON_SATE = 0x42;
constexpr uint8_t CMD_REQUEST_ENCODER_0 = 0x41;
constexpr uint8_t RESPONSE_BUTTON_STATE = 0xc2;
constexpr uint8_t RESPONSE_ENCODER_0 = 0xc1;
constexpr int BUTTON_BUFFER_OFFSET = 0;
constexpr int ENCODER_0_BUFFER_OFFSET = 5;

TRC1000Audio::TRC1000Audio(ClassConfiguration& config) :TRC1000(config, READ_BUFFER_SIZE, WRITE_BUFFER_SIZE)
{
	// cmd response oxc2				0
	trc1000audio_buttons.push_back(PanelButton(1 * 8 + 0, "COM1MIC"));
	trc1000audio_buttons.push_back(PanelButton(1 * 8 + 1, "COM2MIC"));
	trc1000audio_buttons.push_back(PanelButton(1 * 8 + 2, "COM3MIC"));
	trc1000audio_buttons.push_back(PanelButton(1 * 8 + 3, "COM1/2"));
	trc1000audio_buttons.push_back(PanelButton(1 * 8 + 4, "PA"));
	trc1000audio_buttons.push_back(PanelButton(1 * 8 + 5, "MKRMUTE"));

	trc1000audio_buttons.push_back(PanelButton(2 * 8 + 0, "DME"));
	trc1000audio_buttons.push_back(PanelButton(2 * 8 + 1, "ADF"));
	trc1000audio_buttons.push_back(PanelButton(2 * 8 + 2, "AUX"));
	trc1000audio_buttons.push_back(PanelButton(2 * 8 + 3, "MANSQ"));
	trc1000audio_buttons.push_back(PanelButton(2 * 8 + 4, "PILOT"));
	trc1000audio_buttons.push_back(PanelButton(2 * 8 + 5, "COM1"));

	trc1000audio_buttons.push_back(PanelButton(3 * 8 + 0, "COM2"));
	trc1000audio_buttons.push_back(PanelButton(3 * 8 + 1, "COM3"));
	trc1000audio_buttons.push_back(PanelButton(3 * 8 + 2, "TEL"));
	trc1000audio_buttons.push_back(PanelButton(3 * 8 + 3, "SPKR"));
	trc1000audio_buttons.push_back(PanelButton(3 * 8 + 4, "HISENSE"));
	trc1000audio_buttons.push_back(PanelButton(3 * 8 + 5, "NAV1"));

	trc1000audio_buttons.push_back(PanelButton(4 * 8 + 0, "NAV2"));
	trc1000audio_buttons.push_back(PanelButton(4 * 8 + 1, "PLAY"));
	trc1000audio_buttons.push_back(PanelButton(4 * 8 + 2, "COPLT"));
	trc1000audio_buttons.push_back(PanelButton(4 * 8 + 3, "DISPBACKUP"));
	trc1000audio_buttons.push_back(PanelButton(4 * 8 + 4, "VOLSQ"));

	register_buttons(trc1000audio_buttons);

	// cmd response 0xc1			ENCODER_0_BUFFER_OFFSET + 0
	trc1000audio_encoders.push_back(PanelRotaryEncoder(ENCODER_0_BUFFER_OFFSET + 1, "VOLCOPLT"));
	trc1000audio_encoders.push_back(PanelRotaryEncoder(ENCODER_0_BUFFER_OFFSET + 2, "VOLPILOT"));
	register_rotary_encoders(trc1000audio_encoders);

	// LEDs
	trc1000audio_lights.push_back(PanelLight(2 * 8 + 0, "COM1/2"));
	trc1000audio_lights.push_back(PanelLight(2 * 8 + 1, "COM2MIC"));
	trc1000audio_lights.push_back(PanelLight(2 * 8 + 2, "COM3MIC"));
	trc1000audio_lights.push_back(PanelLight(2 * 8 + 3, "COM1MIC"));
	trc1000audio_lights.push_back(PanelLight(2 * 8 + 4, "COPLT"));

	trc1000audio_lights.push_back(PanelLight(3 * 8 + 0, "AUX"));
	trc1000audio_lights.push_back(PanelLight(3 * 8 + 1, "MANSQ"));
	trc1000audio_lights.push_back(PanelLight(3 * 8 + 2, "PILOT"));
	trc1000audio_lights.push_back(PanelLight(3 * 8 + 3, "COM1"));
	trc1000audio_lights.push_back(PanelLight(3 * 8 + 4, "PA"));
	trc1000audio_lights.push_back(PanelLight(3 * 8 + 5, "MKRMUTE"));
	trc1000audio_lights.push_back(PanelLight(3 * 8 + 6, "DME"));
	trc1000audio_lights.push_back(PanelLight(3 * 8 + 7, "ADF"));

	trc1000audio_lights.push_back(PanelLight(4 * 8 + 0, "HISENSE"));
	trc1000audio_lights.push_back(PanelLight(4 * 8 + 1, "NAV2"));
	trc1000audio_lights.push_back(PanelLight(4 * 8 + 2, "NAV1"));
	trc1000audio_lights.push_back(PanelLight(4 * 8 + 3, "PLAY"));
	trc1000audio_lights.push_back(PanelLight(4 * 8 + 4, "COM2"));
	trc1000audio_lights.push_back(PanelLight(4 * 8 + 5, "TEL"));
	trc1000audio_lights.push_back(PanelLight(4 * 8 + 6, "COM3"));
	trc1000audio_lights.push_back(PanelLight(4 * 8 + 7, "SPKR"));
	register_lights(trc1000audio_lights);

	trc1000_commands.push_back(TRC1000Command(CMD_REQUEST_BUTTON_SATE, RESPONSE_BUTTON_STATE, BUTTON_BUFFER_OFFSET,5));
	trc1000_commands.push_back(TRC1000Command(CMD_REQUEST_ENCODER_0, RESPONSE_ENCODER_0, ENCODER_0_BUFFER_OFFSET,3));
}

TRC1000Audio::~TRC1000Audio()
{
	//
}
