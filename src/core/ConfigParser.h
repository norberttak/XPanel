/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <string>
#include <vector>
#include "Configuration.h"
#include "IniFileParser.h"

class Configparser
{
private:
	typedef int (Configparser::* f_handle_on_key_value)(IniFileSectionHeader section_header, std::string key, std::string value, Configuration& config);

	std::map<std::string, f_handle_on_key_value> process_functions;
	std::vector<std::string> tokenize(std::string line);
	void check_and_get_array_index(std::string& dataref, int& index);
	int process_ini_section(IniFileSection& section, Configuration& config);

	int process_fip_layer_section(IniFileSection& section, Configuration& config);

	int handle_on_push_or_release(IniFileSectionHeader section_header, std::string key, std::string value, Configuration& config);
	int handle_on_lit_or_unlit_or_blink(IniFileSectionHeader section_header, std::string key, std::string value, Configuration& config);
	int handle_on_dynamic_speed(IniFileSectionHeader section_header, std::string key, std::string value, Configuration& config);
	int handle_on_vid(IniFileSectionHeader section_header, std::string key, std::string value, Configuration& config);
	int handle_on_pid(IniFileSectionHeader section_header, std::string key, std::string value, Configuration& config);
	int handle_on_log_level(IniFileSectionHeader section_header, std::string key, std::string value, Configuration& config);
	int handle_on_acf_file(IniFileSectionHeader section_header, std::string key, std::string value, Configuration& config);
	int handle_on_script_file(IniFileSectionHeader section_header, std::string key, std::string value, Configuration& config);
	int handle_on_line_add(IniFileSectionHeader section_header, std::string key, std::string value, Configuration& config);
	int handle_on_set_bcd(IniFileSectionHeader section_header, std::string key, std::string value, Configuration& config);
	int handle_on_encoder_inc_or_dec(IniFileSectionHeader section_header, std::string key, std::string value, Configuration& config);
	int handle_on_fip_serial(IniFileSectionHeader section_header, std::string key, std::string value, Configuration& config);
	int handle_on_fip_offset(IniFileSectionHeader section_header, std::string key, std::string value, Configuration& config);
	int handle_on_fip_rotation(IniFileSectionHeader section_header, std::string key, std::string value, Configuration& config);
	int handle_on_fip_mask(IniFileSectionHeader section_header, std::string key, std::string value, Configuration& config);
	int handle_on_fip_text(IniFileSectionHeader section_header, std::string key, std::string value, Configuration& config);

	XPLMDataTypeID normalize_dataref_type(XPLMDataTypeID data_ref_type);

	const std::string TOKEN_VID = "vid";
	const std::string TOKEN_PID = "pid";
	const std::string TOKEN_SERIAL = "serial";
	const std::string TOKEN_SCRIPT = "script_file";
	const std::string TOKEN_ACF = "aircraft_acf";
	const std::string TOKEN_LOG_LEVEL = "log_level";

	const std::string TOKEN_DYN_SPEED_MID = "dynamic_speed_mid";
	const std::string TOKEN_DYN_SPEED_HIGH = "dynamic_speed_high";

	const std::string TOKEN_SECTION_DEVICE = "device";
	const std::string TOKEN_SECTION_BUTTON = "button";
	const std::string TOKEN_SECTION_LIGHT = "light";
	const std::string TOKEN_SECTION_ROT_ENCODER = "encoder";
	const std::string TOKEN_SECTION_DISPLAY = "display";
	const std::string TOKEN_SECTION_MULTI_DISPLAY = "multi_display";
	const std::string TOKEN_SECTION_FIP_SCREEN = "screen";

	const std::string TOKEN_FIP_PAGE = "page";
	const std::string TOKEN_FIP_LAYER = "layer";
	const std::string TOKEN_FIP_TEXT = "text";
	const std::string TOKEN_FIP_MASK = "mask";
	const std::string TOKEN_FIP_OFFSET_X = "offset_x";
	const std::string TOKEN_FIP_OFFSET_Y = "offset_y";
	const std::string TOKEN_FIP_ROTATION = "rotation";

	const std::string DEVICE_TYPE_SAITEK_MULTI = "saitek_multi";
	const std::string DEVICE_TYPE_SAITEK_RADIO = "saitek_radio";
	const std::string DEVICE_TYPE_SAITEK_SWITCH = "saitek_switch";
	const std::string DEVICE_TYPE_ARDUINO_HOME_COCKPIT = "aurduino_homecockpit";
	const std::string DEVICE_TYPE_SAITEK_FIP_SCREEN = "saitek_fip_screen";
	const std::string DEVICE_TYPE_TRC1000PFD = "trc1000_pfd";
	const std::string DEVICE_TYPE_TRC1000AUDIO = "trc1000_audio";

	const std::string TOKEN_DATAREF = "dataref";
	const std::string TOKEN_COMMANDREF = "commandref";
	const std::string TOKEN_LUA = "lua";
	const std::string TOKEN_CONST = "const";
	const std::string TOKEN_BCD = "bcd";
	const std::string TOKEN_ON_SELECT = "on_select";
	const std::string TOKEN_BEGIN = "begin";
	const std::string TOKEN_END = "end";
	const std::string TOKEN_ON_PUSH = "on_push";
	const std::string TOKEN_ON_RELEASE = "on_release";
	const std::string TOKEN_LIT = "trigger_lit";
	const std::string TOKEN_UNLIT = "trigger_unlit";
	const std::string TOKEN_BLINK = "trigger_blink";
	const std::string TOKEN_DISPLAY_LINE = "line";
	const std::string TOKEN_ENCODER_INC = "on_increment";
	const std::string TOKEN_ENCODER_DEC = "on_decrement";
	const std::string TOKEN_IMAGE = "image";
	const std::string TOKEN_TEXT = "text";
	const std::string TOKEN_TYPE = "type";

	std::string last_error_message = "";
	std::string last_device_id = "";
	float speed_mid = 0;
	int multi_mid = 1;
	float speed_high = 0;
	int multi_high = 1;
public:
	Configparser();
	~Configparser();
	int parse_file(std::string file_name, Configuration& config);
};
