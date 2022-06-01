#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include "configparser.h"

int Configparser::parse_file(std::string file_name, std::vector<Configuration>& config)
{
    last_error_message = "";
    std::ifstream input_file(file_name);
    if (!input_file.is_open()) {
        error_message("Error open config file:", file_name);
        return EXIT_FAILURE;
    }

    std::string line;
    int current_line_nr = 1;
    std::stringstream error_details;
    while (std::getline(input_file, line)) {
        current_line_nr++;
        if (parse_line(line, config) != EXIT_SUCCESS)
        {
            error_message("Error in file:", line);
            return EXIT_FAILURE;
        }
    }

    input_file.close();
    return EXIT_SUCCESS;
}

void Configparser::error_message(std::string detail, std::string line)
{
    std::stringstream error_details;
    error_details << "XPanel<configparser>:" << detail << " :[line:" << current_line_nr << "]: " << line;
    error_details >> last_error_message;
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
        {
            error_message("Uknown device type", line);
            return EXIT_FAILURE;
        }
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

    if (std::regex_match(line.c_str(), m, std::regex(TOKEN_BUTTON)))
    {
        section_id = m[1];
        return EXIT_SUCCESS;
    }

    if (std::regex_match(line.c_str(), m, std::regex(TOKEN_BUTTON_PUSH_DATAREF)))
	{
        XPLMDataRef dataRef = XPLMFindDataRef(m[1].str().c_str());
        if (dataRef == NULL)
        {
            error_message("Invalid data ref", line);
            return EXIT_FAILURE;
        }
        Action *push_action = new Action(dataRef, stoi(m[2]));
		config.back().push_actions[section_id] = *push_action;
        return EXIT_SUCCESS;
	}

    if (std::regex_match(line.c_str(), m, std::regex(TOKEN_BUTTON_RELEASE_DATAREF)))
    {
        XPLMDataRef dataRef = XPLMFindDataRef(m[1].str().c_str());
        if (dataRef == NULL)
        {
            error_message("Invalid data ref", line);
            return EXIT_FAILURE;
        }
        Action* release_action = new Action(dataRef, stoi(m[2]));
        config.back().release_actions[section_id] = *release_action;
        return EXIT_SUCCESS;
    }

    if (std::regex_match(line.c_str(), m, std::regex(TOKEN_BUTTON_PUSH_COMMANDREF)))
    {
        XPLMCommandRef commandRef = XPLMFindCommand(m[1].str().c_str());
        if (commandRef == NULL)
        {
            error_message("Invalid command ref", line);
            return EXIT_FAILURE;
        }
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
        if (commandRef == NULL)
        {
            error_message("Invalid command ref", line);
            return EXIT_FAILURE;
        }
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

    error_message("Unknown error", line);
    return EXIT_FAILURE;
}

std::string Configparser::get_last_error_message()
{
    return last_error_message;
}