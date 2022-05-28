#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include "configparser.h"

int Configparser::parse_file(std::string file_name, std::vector<Configuration>& config)
{
    std::ifstream input_file(file_name);
    if (!input_file.is_open()) {
        return EXIT_FAILURE;
    }

    std::string line;
    while (std::getline(input_file, line)) {
        if (parse_line(line, config) != EXIT_SUCCESS)
            return EXIT_FAILURE;
    }

    input_file.close();
    return EXIT_SUCCESS;
}

int Configparser::parse_line(std::string line, std::vector<Configuration>& config)
{
    // comments are marked by semicolumn (;)
    size_t pos = line.find(';', 0);
    if (pos != std::string::npos)
        line.erase(line.find(';', 0), line.length() - pos);
    
    // remove the leading and trailing whitespaces
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

        Configuration* c = new Configuration();
        config.push_back(*c);

        if (section_id == DEVICE_TYPE_SAITEK_MULTI)
            config.back().device_type = SAITEK_MULTI;
        else if (section_id == DEVICE_TYPE_SAITEK_RADIO)
            config.back().device_type = SAITEK_RADIO;
        else if (section_id == DEVICE_TYPE_SAITEK_SWITCH)
            config.back().device_type = SAITEK_SWITCH;
        else if (section_id == DEVICE_TYPE_ARDUINO_HOME_COCKPIT)
            config.back().device_type = HOME_COCKPIT;
        else
            return EXIT_FAILURE;

        return EXIT_SUCCESS;
    }
 
	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_VID)))
	{
		std::stringstream ss;
		ss << std::hex << m[1];
		ss >> config.back().vid;
        return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_PID)))
	{
		std::stringstream ss;
		ss << std::hex << m[1];
		ss >> config.back().pid;
        return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_SCRIPT)))
	{
		config.back().script_file = m[1];
        return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_ACF)))
	{
		config.back().aircraft_acf = m[1];
        return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_VID)))
	{
		std::stringstream ss;
		ss << std::hex << m[1];
		ss >> config.back().vid;
        return EXIT_SUCCESS;
	}

    if (std::regex_match(line.c_str(), m, std::regex(TOKEN_BUTTON)))
    {
        section_id = m[1];
        return EXIT_SUCCESS;
    }

    if (std::regex_match(line.c_str(), m, std::regex(TOKEN_BUTTON_PUSH_DATAREF)))
	{
        XPLMDataRef dataRef = XPLMFindDataRef(m[1].str().c_str());
        Action *push_action = new Action(dataRef, stoi(m[2]));
		config.back().push_actions[section_id] = *push_action;
        return EXIT_SUCCESS;
	}

    if (std::regex_match(line.c_str(), m, std::regex(TOKEN_BUTTON_RELEASE_DATAREF)))
    {
        XPLMDataRef dataRef = XPLMFindDataRef(m[1].str().c_str());
        Action* release_action = new Action(dataRef, stoi(m[2]));
        config.back().release_actions[section_id] = *release_action;
        return EXIT_SUCCESS;
    }

    if (std::regex_match(line.c_str(), m, std::regex(TOKEN_BUTTON_PUSH_COMMANDREF)))
    {
        XPLMCommandRef commandRef = XPLMFindCommand(m[1].str().c_str());
        CommandType command_type = CommandType::NONE;

        if (m.size() >= 3)
            if (m[2] == ":begin")
                command_type = CommandType::BEGIN;
            else if (m[2] == ":end")
                command_type = CommandType::END;
            else
                command_type = CommandType::ONCE;

        Action* push_action = new Action(commandRef, command_type);
        config.back().push_actions[section_id] = *push_action;
        return EXIT_SUCCESS;
    }

    if (std::regex_match(line.c_str(), m, std::regex(TOKEN_BUTTON_RELEASE_COMMANDREF)))
    {
        XPLMCommandRef commandRef = XPLMFindCommand(m[1].str().c_str());
        CommandType command_type = CommandType::ONCE;

        if (m.size() >= 3)
            if (m[2] == ":begin")
                command_type = CommandType::BEGIN;
            else if (m[2] == ":end")
                command_type = CommandType::END;
            else
                command_type = CommandType::ONCE;

        Action* release_action = new Action(commandRef, command_type);
        config.back().release_actions[section_id] = *release_action;
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}