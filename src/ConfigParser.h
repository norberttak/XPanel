/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <string>
#include <vector>
#include "Configuration.h"

class Configparser
{
private:
	const std::string RE_FLOAT = "([+-]*[0-9\\.]+)";
	const std::string RE_INT = "([+-]*[0-9]+)";
	const std::string RE_REF = "([a-zA-Z0-9\\/_-]+)";
	const std::string RE_LUA = "(.+)";

	const std::string DEVICE_TYPE_SAITEK_MULTI = "saitek_multi";
	const std::string DEVICE_TYPE_SAITEK_RADIO = "saitek_radio";
	const std::string DEVICE_TYPE_SAITEK_SWITCH = "saitek_switch";
	const std::string DEVICE_TYPE_ARDUINO_HOME_COCKPIT = "aurduino_homecockpit";
	const std::string DEVICE_TYPE_SAITEK_FIP_SCREEN = "saitek_fip_screen";
	const std::string DEVICE_TYPE_TRC1000PFD = "trc1000_pfd";
	const std::string DEVICE_TYPE_TRC1000AUDIO = "trc1000_audio";

	const std::string TOKEN_DEVICE  = "\\[device:id=\"([a-zA-Z0-9_-]+)\"\\]";
	const std::string TOKEN_VID		= "vid=\"(.+)\"";
	const std::string TOKEN_PID		= "pid=\"(.+)\"";
	const std::string TOKEN_SERIAL  = "serial=\"(.+)\"";
	const std::string TOKEN_SCRIPT	= "script_file=\"(.+)\"";
	const std::string TOKEN_ACF		= "aircraft_acf=\"(.+)\"";

	// Button
	const std::string TOKEN_BUTTON = "\\[button:id=\"([a-zA-Z0-9_-]+)\"\\]";
	const std::string TOKEN_DYN_SPEED = "dynamic_speed=\"t_low="+RE_FLOAT+"x"+RE_INT+",t_high="+RE_FLOAT+"x"+RE_INT+"\"";

	const std::string TOKEN_LOG_LEVEL = "log_level=\"(.+)\"";
	const std::string TOKEN_BUTTON_PUSH_DATAREF = "on_push=\"dataref:"+RE_REF+":"+RE_INT+"\"";
	const std::string TOKEN_BUTTON_PUSH_DATAREF_ARRAY = "on_push=\"dataref:"+RE_REF+"\\["+RE_INT+"\\]:"+RE_INT+"\"";
	const std::string TOKEN_BUTTON_PUSH_COMMANDREF = "on_push=\"commandref:"+RE_REF+":([begin|end|once]*)\"";
	const std::string TOKEN_BUTTON_PUSH_LUA = "on_push=\"lua:" + RE_LUA+"\"";

	const std::string TOKEN_BUTTON_RELEASE_DATAREF = "on_release=\"dataref:"+RE_REF+":"+RE_INT+"\"";
	const std::string TOKEN_BUTTON_RELEASE_DATAREF_ARRAY = "on_release=\"dataref:"+RE_REF+"\\["+RE_INT+"\\]:"+RE_INT+"\"";
	const std::string TOKEN_BUTTON_RELEASE_COMMANDREF = "on_release=\"commandref:"+RE_REF+":([begin|end|once]*)\"";
	const std::string TOKEN_BUTTON_RELEASE_LUA = "on_release=\"lua:" + RE_LUA + "\"";

	const std::string TOKEN_BUTTON_PUSH_DATAREF_CHANGE = "on_push=\"dataref:"+RE_REF+":"+RE_FLOAT+":"+RE_FLOAT+":"+RE_FLOAT+"\"";
	const std::string TOKEN_BUTTON_RELEASE_DATAREF_CHANGE = "on_release=\"dataref:"+RE_REF+":"+RE_FLOAT+":"+RE_FLOAT+":"+RE_FLOAT+"\"";

	const std::string TOKEN_CONDITIONAL_PUSH_DATAREF_CHANGE = "on_push=\"on_select:([a-zA-Z0-9_-]+),dataref:" + RE_REF + ":" + RE_FLOAT + ":" + RE_FLOAT + ":" + RE_FLOAT + "\"";
	const std::string TOKEN_CONDITIONAL_PUSH_COMMANDREF = "on_push=\"on_select:([a-zA-Z0-9_-]+),commandref:" + RE_REF + ":([begin|end|once]*)\"";
	const std::string TOKEN_CONDITIONAL_PUSH_LUA = "on_push=\"on_select:([a-zA-Z0-9_-]+),lua:" + RE_LUA + "\"";
	const std::string TOKEN_CONDITIONAL_RELEASE_DATAREF_CHANGE = "on_release=\"on_select:([a-zA-Z0-9_-]+),dataref:" + RE_REF + ":" + RE_FLOAT + ":" + RE_FLOAT + ":" + RE_FLOAT + "\"";
	const std::string TOKEN_CONDITIONAL_RELEASE_COMMANDREF = "on_release=\"on_select:([a-zA-Z0-9_-]+),commandref:" + RE_REF + ":([begin|end|once]*)\"";
	const std::string TOKEN_CONDITIONAL_RELEASE_LUA = "on_release=\"on_select:([a-zA-Z0-9_-]+),lua:" + RE_LUA + "\"";

	// Button light
	const std::string TOKEN_LIGHT = "\\[light:id=\"([a-zA-Z0-9_-]+)\"\\]";
	const std::string TOKEN_TRIGGER_LIT = "trigger_lit=\"dataref:"+RE_REF+":"+RE_FLOAT+"\"";
	const std::string TOKEN_TRIGGER_UNLIT = "trigger_unlit=\"dataref:"+RE_REF+":"+RE_FLOAT+"\"";
	const std::string TOKEN_TRIGGER_LIT_LUA = "trigger_lit=\"lua:" + RE_LUA + ":" + RE_FLOAT + "\"";
	const std::string TOKEN_TRIGGER_UNLIT_LUA = "trigger_unlit=\"lua:" + RE_LUA + ":" + RE_FLOAT + "\"";
	const std::string TOKEN_TRIGGER_BLINK = "trigger_blink=\"dataref:"+RE_REF+":"+RE_FLOAT+"\"";

	// Rotary Encoder
	const std::string TOKEN_ROT_ENCODER = "\\[encoder:id=\"([a-zA-Z0-9_-]+)\"\\]";
	const std::string TOKEN_ROT_ENCODER_ON_INC_COMMANDREF = "on_increment=\"commandref:" + RE_REF + ":([begin|end|once]*)\"";
	const std::string TOKEN_ROT_ENCODER_ON_DEC_COMMANDREF = "on_decrement=\"commandref:" + RE_REF + ":([begin|end|once]*)\"";
	const std::string TOKEN_ROT_ENCODER_ON_INC_LUA = "on_increment=\"lua:" + RE_LUA + "\"";
	const std::string TOKEN_ROT_ENCODER_ON_DEC_LUA = "on_decrement=\"lua:" + RE_LUA + "\"";

	// Generic display
	const std::string TOKEN_DISPLAY = "\\[display:id=\"([a-zA-Z0-9_-]+)\",bcd=\"(yes|no)\"\\]";
	const std::string TOKEN_DISPLAY_LINE = "line=\"dataref:" + RE_REF + "\"";
	const std::string TOKEN_DISPLAY_LINE_CONST = "line=\"const:" + RE_FLOAT + "\"";
	const std::string TOKEN_DISPLAY_LINE_LUA = "line=\"lua:" + RE_LUA + "\"";

	// Multi function Display
	const std::string TOKEN_MULTI_DISPLAY = "\\[multi_display:id=\"([a-zA-Z0-9_-]+)\"\\]";
	const std::string TOKEN_MULTI_DISPLAY_LINE = "line=\"on_select:([a-zA-Z0-9_-]+),dataref:"+RE_REF+"\"";
	const std::string TOKEN_MULTI_DISPLAY_LINE_CONST = "line=\"on_select:([a-zA-Z0-9_-]+),const:" + RE_FLOAT + "\"";
	const std::string TOKEN_MULTI_DISPLAY_LINE_LUA = "line=\"on_select:([a-zA-Z0-9_-]+),lua:" + RE_LUA + "\"";

	// FIP screen
	const std::string TOKEN_FIP_SCREEN = "\\[screen:id=\"([a-zA-Z0-9_-]+)\"\\]";
	const std::string TOKEN_FIP_PAGE  = "\\[page:id=\"([a-zA-Z0-9_-]+)\"\\]";	
	const std::string TOKEN_FIP_LAYER = "\\[layer:image=\"([a-zA-Z0-9_\\-/\\.]+),ref_x:" + RE_INT + ",ref_y:" + RE_INT + ",base_rot=" + RE_INT + "\"\\]";
	const std::string TOKEN_FIP_OFFSET_CONST = "offset_([xy]+)=\"const:" + RE_INT + "\"";
	const std::string TOKEN_FIP_OFFSET_DATAREF = "offset_([xy]+)=\"dataref:" + RE_REF + ",scale:" + RE_FLOAT + "\"";
	const std::string TOKEN_FIP_OFFSET_LUA = "offset_([xy]+)=\"lua:" + RE_LUA + "\"";
	const std::string TOKEN_FIP_ROTATION_CONST = "rotation=\"const:" + RE_INT + "\"";
	const std::string TOKEN_FIP_ROTATION_DATAREF = "rotation=\"dataref:" + RE_REF + ",scale:" + RE_FLOAT + "\"";
	const std::string TOKEN_FIP_ROTATION_LUA = "rotation=\"lua:" + RE_LUA + "\"";

	std::string section_id = "";
	std::string last_error_message = "";
	float time_low=0;
	int multi_low=1;
	float time_high=0;
	int multi_high=1;
	int current_line_nr=0;
	int parse_line(std::string line, Configuration& config);
public:
	int parse_file(std::string file_name, Configuration& config);
};
