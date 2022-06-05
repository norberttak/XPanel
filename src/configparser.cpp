#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include "configparser.h"
#include "logger.h"

int Configparser::parse_file(std::string file_name, std::vector<Configuration>& config)
{
    last_error_message = "";
    std::ifstream input_file(file_name);
    Logger(TLogLevel::logINFO) << "parse config file: " << file_name << std::endl;
    if (!input_file.is_open()) {
        Logger(TLogLevel::logERROR) << "parser: error open config file: " << file_name << std::endl;        
        return EXIT_FAILURE;
    }

    std::string line;
    int current_line_nr = 1;
    std::stringstream error_details;
    while (std::getline(input_file, line)) {
        current_line_nr++;
        if (parse_line(line, config) != EXIT_SUCCESS)
        {
            Logger(TLogLevel::logERROR) << "parser: error parse line (at line "<< current_line_nr <<"): " << line << std::endl;
            return EXIT_FAILURE;
        }
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
        {
            Logger(TLogLevel::logERROR) << "parser: uknown device type (at line " << current_line_nr << "): " << line << std::endl;
            return EXIT_FAILURE;
        }
        Logger(TLogLevel::logDEBUG) << "parser: new device detected " << std::endl;
        return EXIT_SUCCESS;
    }

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_VID)))
	{
		std::stringstream ss;
		ss << std::hex << m[1];
		ss >> config.back().vid;
        Logger(TLogLevel::logDEBUG) << "parser: vid="<< config.back().vid << std::endl;
        return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_PID)))
	{
		std::stringstream ss;
		ss << std::hex << m[1];
		ss >> config.back().pid;
        Logger(TLogLevel::logDEBUG) << "parser: pid=" << config.back().pid << std::endl;
        return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_SCRIPT)))
	{
		config.back().script_file = m[1];
        Logger(TLogLevel::logDEBUG) << "parser: script file=" << config.back().script_file << std::endl;
        return EXIT_SUCCESS;
	}

	if (std::regex_match(line.c_str(), m, std::regex(TOKEN_ACF)))
	{
		config.back().aircraft_acf = m[1];
        Logger(TLogLevel::logDEBUG) << "parser: script file=" << config.back().aircraft_acf << std::endl;
        return EXIT_SUCCESS;
	}

    if (std::regex_match(line.c_str(), m, std::regex(TOKEN_LOG_LEVEL)))
    {
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
        Action *push_action = new Action(dataRef, stoi(m[2]));
        Logger(TLogLevel::logDEBUG) << "parser: button push dataref " << m[1].str() << std::endl;
		config.back().push_actions[section_id].push_back(push_action);
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
        config.back().push_actions[section_id].push_back(push_action);
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
        config.back().release_actions[section_id].push_back(release_action);
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
        config.back().release_actions[section_id].push_back(release_action);
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
            if (m[2] == ":begin")
                command_type = CommandType::BEGIN;
            else if (m[2] == ":end")
                command_type = CommandType::END;
            else
                command_type = CommandType::ONCE;

        Action* push_action = new Action(commandRef, command_type);
        Logger(TLogLevel::logDEBUG) << "parser: button push command " << m[1].str() << std::endl;
        config.back().push_actions[section_id].push_back(push_action);
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
            if (m[2] == ":begin")
                command_type = CommandType::BEGIN;
            else if (m[2] == ":end")
                command_type = CommandType::END;
            else
                command_type = CommandType::ONCE;

        Action* release_action = new Action(commandRef, command_type);
        Logger(TLogLevel::logDEBUG) << "parser: button release command " << m[1].str() << std::endl;
        config.back().release_actions[section_id].push_back(release_action);
        return EXIT_SUCCESS;
    }

    Logger(TLogLevel::logERROR) << "parser: unknown error (at line: " << current_line_nr << "): " << line << std::endl;
    return EXIT_FAILURE;
}