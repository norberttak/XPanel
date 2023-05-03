/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <regex>
#include "ConfigParser.h"
#include "Trigger.h"
#include "MultiPurposeDisplay.h"
#include "FIPScreen.h"
#include "Logger.h"

int Configparser::parse_file(std::string file_name, Configuration& config)
{
	last_error_message = "";
	std::ifstream input_file(file_name);
	Logger(TLogLevel::logINFO) << "parse config file: " << file_name << std::endl;
	if (!input_file.is_open()) {
		Logger(TLogLevel::logERROR) << "parser: error open config file: " << file_name << std::endl;
		return EXIT_FAILURE;
	}

	std::string line;
	std::stringstream error_details;
	current_line_nr = 0;
	while (std::getline(input_file, line)) {
		current_line_nr++;
		if (parse_line(line, config) != EXIT_SUCCESS)
		{
			Logger(TLogLevel::logERROR) << "parser: error parse line (at line " << current_line_nr << "): " << line << std::endl;
			input_file.close();
			return EXIT_FAILURE;
		}
	}

	input_file.close();
	return EXIT_SUCCESS;
}

int Configparser::parse_line(std::string line, Configuration& config)
{
	// comments are marked by semicolon (;)
	size_t pos = line.find(';', 0);
	if (pos != std::string::npos)
		line.erase(line.find(';', 0), line.length() - pos);

	// remove the leading and trailing whitespace
	std::regex r("^\\s+");
	line = std::regex_replace(line, r, "");
	r = "\\s+$";
	line = std::regex_replace(line, r, "");

	if (line.size() == 0)
		return EXIT_SUCCESS;

	std::cmatch m;
	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_DEVICE)))
	{
		section_id = m[1];

		DeviceConfiguration* c = new DeviceConfiguration();
		config.device_configs.push_back(*c);

		if (section_id == DEVICE_TYPE_SAITEK_MULTI)
			config.device_configs.back().device_type = SAITEK_MULTI;
		else if (section_id == DEVICE_TYPE_SAITEK_RADIO)
			config.device_configs.back().device_type = SAITEK_RADIO;
		else if (section_id == DEVICE_TYPE_SAITEK_SWITCH)
			config.device_configs.back().device_type = SAITEK_SWITCH;
		else if (section_id == DEVICE_TYPE_ARDUINO_HOME_COCKPIT)
			config.device_configs.back().device_type = HOME_COCKPIT;
		else if (section_id == DEVICE_TYPE_SAITEK_FIP_SCREEN)
			config.device_configs.back().device_type = LOGITECH_FIP;
		else if (section_id == DEVICE_TYPE_TRC1000PFD)
			config.device_configs.back().device_type = TRC1000_PFD;
		else if (section_id == DEVICE_TYPE_TRC1000AUDIO)
			config.device_configs.back().device_type = TRC1000_AUDIO;
		else
		{
			Logger(TLogLevel::logERROR) << "parser: unknown device type (at line " << current_line_nr << "): " << line << std::endl;
			return EXIT_FAILURE;
		}
		Logger(TLogLevel::logDEBUG) << "parser: new device detected: "<< section_id << std::endl;
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_VID)))
	{
		std::stringstream ss;
		ss << std::hex << m[1];
		ss >> config.device_configs.back().vid;
		Logger(TLogLevel::logDEBUG) << "parser: vid=" << config.device_configs.back().vid << std::endl;
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_PID)))
	{
		std::stringstream ss;
		ss << std::hex << m[1];
		ss >> config.device_configs.back().pid;
		Logger(TLogLevel::logDEBUG) << "parser: pid=" << config.device_configs.back().pid << std::endl;
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_SERIAL)))
	{
		std::stringstream ss;
		ss << std::hex << m[1];
		ss >> config.device_configs.back().serial_number;
		Logger(TLogLevel::logDEBUG) << "parser: serial=" << config.device_configs.back().serial_number  << std::endl;
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_SCRIPT)))
	{
		if (section_id != "")
			Logger(TLogLevel::logWARNING) << "Script shall be defined in the common part of the config file!" << std::endl;

		config.script_file = m[1];
		Logger(TLogLevel::logDEBUG) << "parser: script file=" << config.script_file << std::endl;
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_ACF)))
	{
		if (section_id != "")
			Logger(TLogLevel::logWARNING) << "ACF file shall be defined in the common part of the config file!" << std::endl;

		config.aircraft_acf = m[1];
		Logger(TLogLevel::logDEBUG) << "parser: ACF file=" << config.aircraft_acf << std::endl;
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_LOG_LEVEL)))
	{
		if (section_id != "")
			Logger(TLogLevel::logWARNING) << "Log level shall be defined in the common part of the config file!" << std::endl;

		TLogLevel level;
		if (m[1] == "DEBUG")
			level = TLogLevel::logDEBUG;
		else if (m[1] == "INFO")
			level = TLogLevel::logINFO;
		else if (m[1] == "WARNING")
			level = TLogLevel::logWARNING;
		else if (m[1] == "ERROR")
			level = TLogLevel::logERROR;
		else if (m[1] == "TRACE")
			level = TLogLevel::logTRACE;
		else {
			Logger(TLogLevel::logERROR) << "parser: unknown log level: " << line << std::endl;
			return EXIT_FAILURE;
		}

		Logger::set_log_level(level);
		Logger(TLogLevel::logDEBUG) << "parser: log level set to " << level << std::endl;

		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_BUTTON)))
	{
		section_id = m[1];
		Logger(TLogLevel::logDEBUG) << "parser: button detected " << section_id << std::endl;
		multi_mid = 1; multi_high = 1;
		speed_mid = 0; speed_high = 0;
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_DYN_SPEED)))
	{
		if (m[1] == "mid") {
			speed_mid = stof(m[2]);
			multi_mid = stoi(m[3]);
		}
		else 
		{
			speed_high = stof(m[2]);
			multi_high = stoi(m[3]);
		}		

		Logger(TLogLevel::logDEBUG) << "parser: dynamic speed config: " << m[1] << ":" << m[2] <<"x"<< m[3] << std::endl;
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_BUTTON_PUSH_DATAREF)))
	{
		XPLMDataRef dataRef = XPLMFindDataRef(m[1].str().c_str());
		if (dataRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid data ref (at line: " << current_line_nr << "): " << line << std::endl;
			return EXIT_FAILURE;
		}
		Action* push_action = new Action(dataRef, stoi(m[2]));
		Logger(TLogLevel::logDEBUG) << "parser: button push dataref " << m[1].str() << std::endl;
		config.device_configs.back().push_actions[section_id].push_back(push_action);
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_BUTTON_PUSH_DATAREF_ARRAY)))
	{
		XPLMDataRef dataRef = XPLMFindDataRef(m[1].str().c_str());
		if (dataRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid data ref (at line: " << current_line_nr << "): " << line << std::endl;
			return EXIT_FAILURE;
		}
		Action* push_action = new Action(dataRef, stoi(m[2]), stoi(m[3]));
		Logger(TLogLevel::logDEBUG) << "parser: button push dataref array " << m[1].str() << std::endl;
		config.device_configs.back().push_actions[section_id].push_back(push_action);
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_BUTTON_PUSH_LUA)))
	{
		Action* push_action = new Action(m[1]);
		Logger(TLogLevel::logDEBUG) << "parser: button push lua string " << m[1].str() << std::endl;
		config.device_configs.back().push_actions[section_id].push_back(push_action);
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_BUTTON_RELEASE_DATAREF)))
	{
		XPLMDataRef dataRef = XPLMFindDataRef(m[1].str().c_str());
		if (dataRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid data ref (at line: " << current_line_nr << "): " << line << std::endl;
			return EXIT_FAILURE;
		}
		Action* release_action = new Action(dataRef, stoi(m[2]));
		Logger(TLogLevel::logDEBUG) << "parser: button release dataref " << m[1].str() << std::endl;
		config.device_configs.back().release_actions[section_id].push_back(release_action);
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_BUTTON_RELEASE_DATAREF_ARRAY)))
	{
		XPLMDataRef dataRef = XPLMFindDataRef(m[1].str().c_str());
		if (dataRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid data ref (at line: " << current_line_nr << "): " << line << std::endl;
			return EXIT_FAILURE;
		}
		Action* release_action = new Action(dataRef, stoi(m[2]), stoi(m[3]));
		Logger(TLogLevel::logDEBUG) << "parser: button release dataref array " << m[1].str() << std::endl;
		config.device_configs.back().release_actions[section_id].push_back(release_action);
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_BUTTON_RELEASE_LUA)))
	{
		Action* release_action = new Action(m[1]);
		Logger(TLogLevel::logDEBUG) << "parser: button release lua string " << m[1].str() << std::endl;
		config.device_configs.back().release_actions[section_id].push_back(release_action);
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_BUTTON_PUSH_DATAREF_CHANGE)))
	{
		XPLMDataRef dataRef = XPLMFindDataRef(m[1].str().c_str());
		if (dataRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid data ref (at line: " << current_line_nr << "): " << line << std::endl;
			return EXIT_FAILURE;
		}
		Action* push_action = new Action(dataRef, stof(m[2]), stof(m[3]), stof(m[4]));
		Logger(TLogLevel::logDEBUG) << "parser: button push dataref " << m[1].str() << std::endl;
		push_action->set_dynamic_speed_params(speed_mid, multi_mid, speed_high, multi_high);
		config.device_configs.back().push_actions[section_id].push_back(push_action);
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_BUTTON_RELEASE_DATAREF_CHANGE)))
	{
		XPLMDataRef dataRef = XPLMFindDataRef(m[1].str().c_str());
		if (dataRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid data ref (at line: " << current_line_nr << "): " << line << std::endl;
			return EXIT_FAILURE;
		}
		Action* release_action = new Action(dataRef, stof(m[2]), stof(m[3]), stof(m[4]));
		Logger(TLogLevel::logDEBUG) << "parser: button release dataref " << m[1].str() << std::endl;
		release_action->set_dynamic_speed_params(speed_mid, multi_mid, speed_high, multi_high);
		config.device_configs.back().release_actions[section_id].push_back(release_action);
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_CONDITIONAL_PUSH_DATAREF_CHANGE)))
	{
		XPLMDataRef dataRef = XPLMFindDataRef(m[2].str().c_str());
		if (dataRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid data ref (at line: " << current_line_nr << "): " << line << std::endl;
			return EXIT_FAILURE;
		}
		Action* push_action = new Action(dataRef, stof(m[3]), stof(m[5]), stof(m[4]));
		push_action->set_dynamic_speed_params(speed_mid, multi_mid, speed_high, multi_high);
		push_action->add_condition(m[1]);
		config.device_configs.back().push_actions[section_id].push_back(push_action);
		Logger(TLogLevel::logDEBUG) << "parser: button conditional push dataref. on:" << m[1].str() << " dataref:" << m[2].str() << std::endl;
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_CONDITIONAL_RELEASE_DATAREF_CHANGE)))
	{
		XPLMDataRef dataRef = XPLMFindDataRef(m[2].str().c_str());
		if (dataRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid data ref (at line: " << current_line_nr << "): " << line << std::endl;
			return EXIT_FAILURE;
		}
		Action* release_action = new Action(dataRef, stof(m[3]), stof(m[5]), stof(m[4]));
		release_action->set_dynamic_speed_params(speed_mid, multi_mid, speed_high, multi_high);
		release_action->add_condition(m[1]);
		config.device_configs.back().release_actions[section_id].push_back(release_action);
		Logger(TLogLevel::logDEBUG) << "parser: button conditional release dataref. on:" << m[1].str() << " dataref:" << m[2].str() << std::endl;
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_BUTTON_PUSH_COMMANDREF)))
	{
		XPLMCommandRef commandRef = XPLMFindCommand(m[1].str().c_str());
		if (commandRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid command ref (at line: " << current_line_nr << "): " << line << std::endl;
			return EXIT_FAILURE;
		}
		CommandType command_type = CommandType::NONE;

		if (m.size() >= 3)
		{
			if (m[2] == "begin")
				command_type = CommandType::BEGIN;
			else if (m[2] == "end")
				command_type = CommandType::END;
			else
				command_type = CommandType::ONCE;
		}

		Action* push_action = new Action(commandRef, command_type);
		push_action->set_dynamic_speed_params(speed_mid, multi_mid, speed_high, multi_high);
		Logger(TLogLevel::logDEBUG) << "parser: button push command " << m[1].str() << std::endl;
		config.device_configs.back().push_actions[section_id].push_back(push_action);
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_BUTTON_RELEASE_COMMANDREF)))
	{
		XPLMCommandRef commandRef = XPLMFindCommand(m[1].str().c_str());
		if (commandRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid command ref (at line: " << current_line_nr << "): " << line << std::endl;
			return EXIT_FAILURE;
		}
		CommandType command_type = CommandType::ONCE;

		if (m.size() >= 3)
		{
			if (m[2] == "begin")
				command_type = CommandType::BEGIN;
			else if (m[2] == "end")
				command_type = CommandType::END;
			else
				command_type = CommandType::ONCE;
		}

		Action* release_action = new Action(commandRef, command_type);
		release_action->set_dynamic_speed_params(speed_mid, multi_mid, speed_high, multi_high);
		Logger(TLogLevel::logDEBUG) << "parser: button release command " << m[1].str() << std::endl;
		config.device_configs.back().release_actions[section_id].push_back(release_action);
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_CONDITIONAL_PUSH_COMMANDREF)))
	{
		XPLMCommandRef commandRef = XPLMFindCommand(m[2].str().c_str());
		if (commandRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid command ref (at line: " << current_line_nr << "): " << line << std::endl;
			return EXIT_FAILURE;
		}
		CommandType command_type = CommandType::NONE;

		if (m.size() >= 4)
		{
			if (m[3] == "begin")
				command_type = CommandType::BEGIN;
			else if (m[3] == "end")
				command_type = CommandType::END;
			else
				command_type = CommandType::ONCE;
		}

		Action* push_action = new Action(commandRef, command_type);
		push_action->add_condition(m[1]);
		push_action->set_dynamic_speed_params(speed_mid, multi_mid, speed_high, multi_high);
		Logger(TLogLevel::logDEBUG) << "parser: button conditional (" << m[1] << ") push command " << m[2].str() << std::endl;
		config.device_configs.back().push_actions[section_id].push_back(push_action);
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_CONDITIONAL_PUSH_LUA)))
	{
		Action* push_action = new Action(m[2]);
		push_action->add_condition(m[1]);
		Logger(TLogLevel::logDEBUG) << "parser: button conditional (" << m[1] << ") push lua " << m[2].str() << std::endl;
		config.device_configs.back().push_actions[section_id].push_back(push_action);
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_CONDITIONAL_RELEASE_COMMANDREF)))
	{
		XPLMCommandRef commandRef = XPLMFindCommand(m[2].str().c_str());
		if (commandRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid command ref (at line: " << current_line_nr << "): " << line << std::endl;
			return EXIT_FAILURE;
		}
		CommandType command_type = CommandType::NONE;

		if (m.size() >= 4)
		{
			if (m[3] == "begin")
				command_type = CommandType::BEGIN;
			else if (m[3] == "end")
				command_type = CommandType::END;
			else
				command_type = CommandType::ONCE;
		}

		Action* release_action = new Action(commandRef, command_type);
		release_action->add_condition(m[1]);
		release_action->set_dynamic_speed_params(speed_mid, multi_mid, speed_high, multi_high);
		Logger(TLogLevel::logDEBUG) << "parser: button conditional (" << m[1] << ") release command " << m[2].str() << std::endl;
		config.device_configs.back().release_actions[section_id].push_back(release_action);
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_CONDITIONAL_RELEASE_LUA)))
	{
		Action* release_action = new Action(m[2]);
		release_action->add_condition(m[1]);		
		Logger(TLogLevel::logDEBUG) << "parser: button conditional (" << m[1] << ") push release " << m[2].str() << std::endl;
		config.device_configs.back().release_actions[section_id].push_back(release_action);
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_ROT_ENCODER)))
	{
		section_id = m[1];
		Logger(TLogLevel::logDEBUG) << "parser: rotation encoder detected " << section_id << std::endl;
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_ROT_ENCODER_ON_INC_COMMANDREF)))
	{
		XPLMCommandRef commandRef = XPLMFindCommand(m[1].str().c_str());
		if (commandRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid command ref (at line: " << current_line_nr << "): " << line << std::endl;
			return EXIT_FAILURE;
		}
		CommandType command_type = CommandType::NONE;

		if (m.size() >= 3)
		{
			if (m[2] == "begin")
				command_type = CommandType::BEGIN;
			else if (m[2] == "end")
				command_type = CommandType::END;
			else
				command_type = CommandType::ONCE;
		}

		Action* inc_action = new Action(commandRef, command_type);
		Logger(TLogLevel::logDEBUG) << "parser: rot encoder inc command " << m[1].str() << std::endl;
		inc_action->set_dynamic_speed_params(speed_mid, multi_mid, speed_high, multi_high);
		config.device_configs.back().encoder_inc_actions[section_id].push_back(inc_action);
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_ROT_ENCODER_ON_DEC_COMMANDREF)))
	{
		XPLMCommandRef commandRef = XPLMFindCommand(m[1].str().c_str());
		if (commandRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid command ref (at line: " << current_line_nr << "): " << line << std::endl;
			return EXIT_FAILURE;
		}
		CommandType command_type = CommandType::NONE;

		if (m.size() >= 3)
		{
			if (m[2] == "begin")
				command_type = CommandType::BEGIN;
			else if (m[2] == "end")
				command_type = CommandType::END;
			else
				command_type = CommandType::ONCE;
		}

		Action* dec_action = new Action(commandRef, command_type);
		dec_action->set_dynamic_speed_params(speed_mid, multi_mid, speed_high, multi_high);
		Logger(TLogLevel::logDEBUG) << "parser: rot encoder dec command " << m[1].str() << std::endl;
		config.device_configs.back().encoder_dec_actions[section_id].push_back(dec_action);
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_ROT_ENCODER_ON_INC_DATAREF_CHANGE)))
	{
		XPLMDataRef dataRef = XPLMFindDataRef(m[1].str().c_str());
		if (dataRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid data ref (at line: " << current_line_nr << "): " << line << std::endl;
			return EXIT_FAILURE;
		}
		Action* inc_action = new Action(dataRef, stof(m[2]), stof(m[4]), stof(m[3]));
		Logger(TLogLevel::logDEBUG) << "parser: encoder inc dataref " << m[1].str() << std::endl;
		inc_action->set_dynamic_speed_params(speed_mid, multi_mid, speed_high, multi_high);
		config.device_configs.back().encoder_inc_actions[section_id].push_back(inc_action);
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_ROT_ENCODER_ON_DEC_DATAREF_CHANGE)))
	{
		XPLMDataRef dataRef = XPLMFindDataRef(m[1].str().c_str());
		if (dataRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid data ref (at line: " << current_line_nr << "): " << line << std::endl;
			return EXIT_FAILURE;
		}
		Action* dec_action = new Action(dataRef, stof(m[2]), stof(m[4]), stof(m[3]));
		Logger(TLogLevel::logDEBUG) << "parser: encoder dec dataref " << m[1].str() << std::endl;
		dec_action->set_dynamic_speed_params(speed_mid, multi_mid, speed_high, multi_high);
		config.device_configs.back().encoder_dec_actions[section_id].push_back(dec_action);
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_ROT_ENCODER_ON_INC_LUA)))
	{
		Action* inc_action = new Action(m[1]);
		Logger(TLogLevel::logDEBUG) << "parser: rot encoder inc lua string " << m[1].str() << std::endl;
		config.device_configs.back().encoder_inc_actions[section_id].push_back(inc_action);
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_ROT_ENCODER_ON_DEC_LUA)))
	{
		Action* dec_action = new Action(m[1]);
		Logger(TLogLevel::logDEBUG) << "parser: rot encoder dec lua string " << m[1].str() << std::endl;
		config.device_configs.back().encoder_inc_actions[section_id].push_back(dec_action);
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_LIGHT)))
	{
		section_id = m[1];
		Logger(TLogLevel::logDEBUG) << "parser: light detected " << section_id << std::endl;
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_TRIGGER_LIT)))
	{
		XPLMDataRef dataRef = XPLMFindDataRef(m[1].str().c_str());
		if (dataRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid data ref (at line: " << current_line_nr << "): " << line << std::endl;
			return EXIT_FAILURE;
		}
		Trigger* lit_trigger = new Trigger(dataRef, stoi(m[2]), TriggerType::LIT);
		Logger(TLogLevel::logDEBUG) << "parser: light lit dataref " << m[1].str() << std::endl;
		config.device_configs.back().light_triggers[section_id].push_back(lit_trigger);
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_TRIGGER_UNLIT)))
	{
		XPLMDataRef dataRef = XPLMFindDataRef(m[1].str().c_str());
		if (dataRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid data ref (at line: " << current_line_nr << "): " << line << std::endl;
			return EXIT_FAILURE;
		}
		Trigger* unlit_trigger = new Trigger(dataRef, stoi(m[2]), TriggerType::UNLIT);
		Logger(TLogLevel::logDEBUG) << "parser: light unlit dataref " << m[1].str() << std::endl;
		config.device_configs.back().light_triggers[section_id].push_back(unlit_trigger);
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_TRIGGER_BLINK)))
	{
		XPLMDataRef dataRef = XPLMFindDataRef(m[1].str().c_str());
		if (dataRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid data ref (at line: " << current_line_nr << "): " << line << std::endl;
			return EXIT_FAILURE;
		}
		Trigger* unlit_trigger = new Trigger(dataRef, stoi(m[2]), TriggerType::BLINK);
		Logger(TLogLevel::logDEBUG) << "parser: light blink dataref " << m[1].str() << std::endl;
		config.device_configs.back().light_triggers[section_id].push_back(unlit_trigger);
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_TRIGGER_LIT_LUA)))
	{
		Trigger* lit_trigger = new Trigger(m[1], stoi(m[2]), TriggerType::LIT);
		Logger(TLogLevel::logDEBUG) << "parser: light lit lua " << m[1].str() << std::endl;
		config.device_configs.back().light_triggers[section_id].push_back(lit_trigger);
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_TRIGGER_UNLIT_LUA)))
	{
		Trigger* unlit_trigger = new Trigger(m[1], stoi(m[2]), TriggerType::UNLIT);
		Logger(TLogLevel::logDEBUG) << "parser: light unlit lua " << m[1].str() << std::endl;
		config.device_configs.back().light_triggers[section_id].push_back(unlit_trigger);
		return EXIT_SUCCESS;
	}

	// ---- Generic display ----
	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_DISPLAY)))
	{
		section_id = m[1];
		bool use_bcd = m[2].str() == "yes" ? true : false;

		config.device_configs.back().generic_displays[section_id] = new GenericDisplay(use_bcd);
		Logger(TLogLevel::logDEBUG) << "parser: generic display detected " << section_id << std::endl;
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_DISPLAY_LINE)))
	{
		XPLMDataRef dataRef = XPLMFindDataRef(m[1].str().c_str());
		if (dataRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid data ref (at line: " << current_line_nr << "): " << line << std::endl;
			return EXIT_FAILURE;
		}
		config.device_configs.back().generic_displays[section_id]->add_dataref(dataRef);
		Logger(TLogLevel::logDEBUG) << "parser: generic display add line " << section_id << " " << m[1] << std::endl;
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_DISPLAY_LINE_ARRAY)))
	{
		XPLMDataRef dataRef = XPLMFindDataRef(m[1].str().c_str());
		if (dataRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid data ref (at line: " << current_line_nr << "): " << line << std::endl;
			return EXIT_FAILURE;
		}
		config.device_configs.back().generic_displays[section_id]->add_dataref(dataRef, stoi(m[2]));
		Logger(TLogLevel::logDEBUG) << "parser: generic display add line " << section_id << " " << m[1] << "[" << m[2] << "]" << std::endl;
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_DISPLAY_LINE_CONST)))
	{
		double const_value = std::stod(m[1].str().c_str());
		config.device_configs.back().generic_displays[section_id]->add_const(const_value);
		Logger(TLogLevel::logDEBUG) << "parser: generic display add line " << section_id << " " << m[1] << std::endl;
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_DISPLAY_LINE_LUA)))
	{
		config.device_configs.back().generic_displays[section_id]->add_lua(m[1]);
		Logger(TLogLevel::logDEBUG) << "parser: generic display add line " << section_id << " " << m[1] << std::endl;
		return EXIT_SUCCESS;
	}

	// ---- Multi purpose display -----
	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_MULTI_DISPLAY)))
	{
		section_id = m[1];
		config.device_configs.back().multi_displays[section_id] = new MultiPurposeDisplay();
		Logger(TLogLevel::logDEBUG) << "parser: multi display detected " << section_id << std::endl;
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_MULTI_DISPLAY_LINE)))
	{
		XPLMDataRef dataRef = XPLMFindDataRef(m[2].str().c_str());
		if (dataRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid data ref (at line: " << current_line_nr << "): " << line << std::endl;
			return EXIT_FAILURE;
		}
		config.device_configs.back().multi_displays[section_id]->add_condition(m[1], dataRef);
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_MULTI_DISPLAY_LINE_CONST)))
	{
		double const_value = std::stod(m[2].str().c_str());
		config.device_configs.back().multi_displays[section_id]->add_condition(m[1], const_value);
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_MULTI_DISPLAY_LINE_LUA)))
	{
		config.device_configs.back().multi_displays[section_id]->add_condition(m[1], m[2]);
		return EXIT_SUCCESS;
	}
	
	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_FIP_SCREEN)))
	{
		FIPScreen* screen = new FIPScreen();
		config.device_configs.back().fip_screens[section_id] = screen;
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_FIP_PAGE)))
	{
		config.device_configs.back().fip_screens[section_id]->add_page(m[1], false);
		return EXIT_SUCCESS;
	}

 	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_FIP_LAYER)))
	{
		int page_index = config.device_configs.back().fip_screens[section_id]->get_last_page_index();
		if (page_index >= 0)
		{
			std::filesystem::path bmp_file_absolute_path = std::filesystem::path(config.aircraft_path);
			bmp_file_absolute_path /= std::string(m[1]);

			if (config.device_configs.back().fip_screens[section_id]->add_layer_to_page(page_index, bmp_file_absolute_path.string(), stoi(m[2]), stoi(m[3]), stoi(m[4])) < 0)
				return EXIT_FAILURE;
			else
				return EXIT_SUCCESS;
		}
		else
		{
			Logger(logERROR) << "Parser: invalid FIP page index" << std::endl;
			return EXIT_FAILURE;
		}
	}
	
	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_FIP_TEXT_LAYER)))
	{
		int page_index = config.device_configs.back().fip_screens[section_id]->get_last_page_index();
		if (page_index >= 0)
		{
			std::filesystem::path fip_fonts_bmp = config.plugin_path;
			fip_fonts_bmp /= "fip-fonts.bmp";

			if (config.device_configs.back().fip_screens[section_id]->add_text_layer_to_page(page_index, fip_fonts_bmp.string(), 0) < 0)
				return EXIT_FAILURE;
			else
				return EXIT_SUCCESS;
		}
		else
		{
			Logger(logERROR) << "Parser: invalid FIP page index" << std::endl;
			return EXIT_FAILURE;
		}
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_FIP_TEXT_VALUE_CONST)))
	{
		int page_index = config.device_configs.back().fip_screens[section_id]->get_last_page_index();
		int layer_index = config.device_configs.back().fip_screens[section_id]->get_last_layer_index(page_index);

		if (page_index >= 0 && layer_index >= 0)
		{
			config.device_configs.back().fip_screens[section_id]->set_text(page_index, layer_index, m[1]);
			Logger(logTRACE) << "Parser: set const text for page " << page_index << " / layer " << layer_index << m[1] << std::endl;
			return EXIT_SUCCESS;
		}
		else
		{
			Logger(logERROR) << "Parser: invalid FIP page or layer index" << std::endl;
			return EXIT_FAILURE;
		}
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_FIP_TEXT_VALUE_DATAREF)))
	{
		ScreenAction* action = new ScreenAction();

		action->page_index = config.device_configs.back().fip_screens[section_id]->get_last_page_index();
		action->layer_index = config.device_configs.back().fip_screens[section_id]->get_last_layer_index(action->page_index);

		XPLMDataRef dataRef = XPLMFindDataRef(m[1].str().c_str());
		if (dataRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid data ref (at line: " << current_line_nr << "): " << line << std::endl;
			return EXIT_FAILURE;
		}
		action->data_ref = dataRef;
		action->data_ref_type = XPLMGetDataRefTypes(dataRef);
		action->type = SC_SET_TEXT;

		Logger(logTRACE) << "config parser: add FIP text set dataref: " << action->data_ref << std::endl;
		config.device_configs.back().fip_screens[section_id]->add_screen_action(action);
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_FIP_TEXT_VALUE_LUA)))
	{
		ScreenAction* action = new ScreenAction();

		action->page_index = config.device_configs.back().fip_screens[section_id]->get_last_page_index();
		action->layer_index = config.device_configs.back().fip_screens[section_id]->get_last_layer_index(action->page_index);
		action->lua_str = m[1];
		action->type = SC_SET_TEXT;

		Logger(logTRACE) << "config parser: add FIP set text lua: " << action->lua_str << std::endl;
		config.device_configs.back().fip_screens[section_id]->add_screen_action(action);
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_FIP_LAYER_MASK)))
	{
		int page_index = config.device_configs.back().fip_screens[section_id]->get_last_page_index();
		int layer_index = config.device_configs.back().fip_screens[section_id]->get_last_layer_index(page_index);

		if (page_index >= 0 && layer_index >= 0)
		{
			MaskWindow mask(stoi(m[1]), stoi(m[2]), stoi(m[4]), stoi(m[3]));
			config.device_configs.back().fip_screens[section_id]->set_mask(page_index, layer_index, mask);
			Logger(logTRACE) << "Parser: set mask for page " << page_index << " / layer " << layer_index << std::endl;
			return EXIT_SUCCESS;
		}
		else
		{
			Logger(logERROR) << "Parser: invalid FIP page or layer index" << std::endl;
			return EXIT_FAILURE;
		}
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_FIP_OFFSET_CONST)))
	{
		ScreenAction *action =new ScreenAction();
		action->page_index = config.device_configs.back().fip_screens[section_id]->get_last_page_index();
		action->layer_index = config.device_configs.back().fip_screens[section_id]->get_last_layer_index(action->page_index);
		action->constant_val = stoi(m[2]);
		action->type = m[1] == "x" ? SC_TRANSLATION_X : SC_TRANSLATION_Y;

		Logger(logTRACE) << "config parser: add FIP offset const: " << action->constant_val << std::endl;
		config.device_configs.back().fip_screens[section_id]->add_screen_action(action);
		return EXIT_SUCCESS;
	}
	
	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_FIP_OFFSET_DATAREF)))
	{
		ScreenAction* action = new ScreenAction();

		action->page_index = config.device_configs.back().fip_screens[section_id]->get_last_page_index();
		action->layer_index = config.device_configs.back().fip_screens[section_id]->get_last_layer_index(action->page_index);		
		action->scale_factor = std::stod(m[3].str().c_str());

		XPLMDataRef dataRef = XPLMFindDataRef(m[2].str().c_str());
		if (dataRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid data ref (at line: " << current_line_nr << "): " << line << std::endl;
			return EXIT_FAILURE;
		}
		action->data_ref = dataRef;
		action->data_ref_type = XPLMGetDataRefTypes(dataRef);
		action->type = m[1] == "x" ? SC_TRANSLATION_X : SC_TRANSLATION_Y;
		
		Logger(logTRACE) << "config parser: add FIP offset dataref: " << action->data_ref << std::endl;
		config.device_configs.back().fip_screens[section_id]->add_screen_action(action);
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_FIP_OFFSET_LUA)))
	{
		ScreenAction* action = new ScreenAction();

		action->page_index = config.device_configs.back().fip_screens[section_id]->get_last_page_index();
		action->layer_index = config.device_configs.back().fip_screens[section_id]->get_last_layer_index(action->page_index);
		action->lua_str = m[2];
		action->type = m[1] == "x" ? SC_TRANSLATION_X : SC_TRANSLATION_Y;

		Logger(logTRACE) << "config parser: add FIP offset lua: " << action->lua_str << std::endl;
		config.device_configs.back().fip_screens[section_id]->add_screen_action(action);
		return EXIT_SUCCESS;
	}
	
	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_FIP_ROTATION_CONST)))
	{
		ScreenAction* action = new ScreenAction();

		action->page_index = config.device_configs.back().fip_screens[section_id]->get_last_page_index();
		action->layer_index = config.device_configs.back().fip_screens[section_id]->get_last_layer_index(action->page_index);
		action->constant_val = stoi(m[1]);
		action->type = SC_ROTATION;

		Logger(logTRACE) << "config parser: add FIP rotation const: " << action->constant_val << std::endl;
		config.device_configs.back().fip_screens[section_id]->add_screen_action(action);
		return EXIT_SUCCESS;
	}
	
	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_FIP_ROTATION_DATAREF)))
	{
		ScreenAction* action = new ScreenAction();

		action->page_index = config.device_configs.back().fip_screens[section_id]->get_last_page_index();
		action->layer_index = config.device_configs.back().fip_screens[section_id]->get_last_layer_index(action->page_index);
		action->scale_factor = std::stod(m[2].str().c_str());
		XPLMDataRef dataRef = XPLMFindDataRef(m[1].str().c_str());
		if (dataRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid data ref (at line: " << current_line_nr << "): " << line << std::endl;
			return EXIT_FAILURE;
		}
		action->data_ref = dataRef;
		action->data_ref_type = XPLMGetDataRefTypes(dataRef);
		action->type = SC_ROTATION;

		Logger(logTRACE) << "config parser: add FIP rotation dataref: " << action->data_ref << std::endl;
		config.device_configs.back().fip_screens[section_id]->add_screen_action(action);
		return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_FIP_ROTATION_LUA)))
	{
		ScreenAction* action = new ScreenAction();

		action->page_index = config.device_configs.back().fip_screens[section_id]->get_last_page_index();
		action->layer_index = config.device_configs.back().fip_screens[section_id]->get_last_layer_index(action->page_index);
		action->lua_str = m[1];
		action->type = SC_ROTATION;

		Logger(logTRACE) << "config parser: add FIP rotation lua: " << action->lua_str << std::endl;
		config.device_configs.back().fip_screens[section_id]->add_screen_action(action);
		return EXIT_SUCCESS;
	}

	Logger(TLogLevel::logERROR) << "parser: unknown error (at line: " << current_line_nr << "): " << line << std::endl;
	return EXIT_FAILURE;
}
