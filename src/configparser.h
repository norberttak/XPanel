#pragma once
#include <string>
#include <vector>
#include "configuration.h"

class Configparser
{
private:
	const std::string RE_FLOAT = "([+-]*[0-9\.]+)";
	const std::string RE_INT = "([+-]*[0-9]+)";
	const std::string RE_REF = "([a-zA-Z0-9\\/\\[\\]_-]+)";

	const std::string DEVICE_TYPE_SAITEK_MULTI = "saitek_multi";
	const std::string DEVICE_TYPE_SAITEK_RADIO = "saitek_radio";
	const std::string DEVICE_TYPE_SAITEK_SWITCH = "saitek_switch";
	const std::string DEVICE_TYPE_ARDUINO_HOME_COCKPIT = "aurduino_homecockpit";

	const std::string TOKEN_DEVICE  = "\\[device:id=\"([a-zA-Z09_-]+)\"\\]";
	const std::string TOKEN_VID		= "vid=\"(.+)\"";
	const std::string TOKEN_PID		= "pid=\"(.+)\"";
	const std::string TOKEN_SCRIPT	= "script_file=\"(.+)\"";
	const std::string TOKEN_ACF		= "aircraft_acf=\"(.+)\"";
	
	// Button
	const std::string TOKEN_BUTTON = "\\[button:id=\"([a-zA-Z0-9_-]+)\"\\]";

	const std::string TOKEN_LOG_LEVEL = "log_level=\"(.+)\"";
	const std::string TOKEN_BUTTON_PUSH_DATAREF = "on_push=\"dataref:"+RE_REF+":"+RE_INT+"\"";
	const std::string TOKEN_BUTTON_PUSH_DATAREF_ARRAY = "on_push=\"dataref:"+RE_REF+"\\["+RE_INT+"\\]:"+RE_INT+"\"";
	const std::string TOKEN_BUTTON_PUSH_SCRIPT = "on_push=\"script:([a-zA-Z0-9/\\(\\)_-]+)";
	const std::string TOKEN_BUTTON_PUSH_COMMANDREF = "on_push=\"commandref:"+RE_REF+":([begin|end|once]*)\"";

	const std::string TOKEN_BUTTON_RELEASE_DATAREF = "on_release=\"dataref:"+RE_REF+":"+RE_INT+"\"";
	const std::string TOKEN_BUTTON_RELEASE_DATAREF_ARRAY = "on_release=\"dataref:"+RE_REF+"\\["+RE_INT+"\\]:"+RE_INT+"\"";
	const std::string TOKEN_BUTTON_RELEASE_SCRIPT = "on_release=\"script:([a-zA-Z0-9/\\(\\)_]+)";
	const std::string TOKEN_BUTTON_RELEASE_COMMANDREF = "on_release=\"commandref:"+RE_REF+":([begin|end|once]*)\"";
	
	const std::string TOKEN_BUTTON_PUSH_DATAREF_CHANGE = "on_push=\"dataref:"+RE_REF+":"+RE_FLOAT+":"+RE_FLOAT+":"+RE_FLOAT+"\"";
	const std::string TOKEN_BUTTON_RELEASE_DATAREF_CHANGE = "on_release=\"dataref:"+RE_REF+":"+RE_FLOAT+":"+RE_FLOAT+":"+RE_FLOAT+"\"";

	// Button light
	const std::string TOKEN_LIGHT = "\\[light:id=\"([a-zA-Z0-9_-]+)\"\\]";

	// Display
	const std::string TOKEN_DISPLAY = "\\[display:id=\"([a-zA-Z0-9_-]+)\"\\]";
	/*
	const std::string TOKEN_TRIGGER_LIT = "trigger_lit=";
	const std::string TOKEN_TRIGGER_UNLIT = "trigger_unlit=";
	const std::string TOKEN_TRIGGER_BLINK = "trigger_blink=";
	*/
	std::string section_id = "";
	std::string last_error_message = "";
	int current_line_nr;
	int parse_line(std::string line, std::vector<Configuration>& config);
public:
	int parse_file(std::string file_name, std::vector<Configuration>& config);
};

