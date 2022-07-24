#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include "ArduinoHomeCockpit.h"
#include "UsbHidDevice.h"
#include "logger.h"

#define WRITE_BUFFER_SIZE 9
#define READ_BUFFER_SIZE 9

ArduinoHomeCockpit::ArduinoHomeCockpit(DeviceConfiguration& config) :UsbHidDevice(config, READ_BUFFER_SIZE, WRITE_BUFFER_SIZE)
{
	if (read_board_configuration("board-config.ini", config.vid, config.pid) != EXIT_SUCCESS)
	{
		Logger(TLogLevel::logERROR) << "Arduino home cockpit. Error parsing board config file" << std::endl;
		return;
	}

	register_buttons(arduino_buttons);
}

int ArduinoHomeCockpit::read_board_configuration(std::string file_name, int expected_vid, int expected_pid)
{
	std::ifstream input_file(file_name);
	Logger(TLogLevel::logINFO) << "board config: parse file: " << file_name << std::endl;
	if (!input_file.is_open()) {
		Logger(TLogLevel::logERROR) << "board config: error open config file: " << file_name << std::endl;
		return EXIT_FAILURE;
	}

	std::string line;
	std::stringstream error_details;
	int current_line_nr = 0;
	int exit_status = EXIT_SUCCESS;
	while (std::getline(input_file, line)) {
		current_line_nr++;

		// comments are marked by semi column (;)
		size_t pos = line.find(';', 0);
		if (pos != std::string::npos)
			line.erase(line.find(';', 0), line.length() - pos);

		// remove the leading and trailing white spaces
		std::regex r("^\\s+");
		line = std::regex_replace(line, r, "");
		r = "\\s+$";
		line = std::regex_replace(line, r, "");

		if (line.size() == 0)
			continue;

		std::cmatch m; 
		
		if (std::regex_match(line.c_str(), m, std::regex(TOKEN_DEVICE)))
		{
			Logger(TLogLevel::logDEBUG) << "board config: device id = " << m[1] << std::endl;
			continue;
		}

		if (std::regex_match(line.c_str(), m, std::regex(TOKEN_VID)))
		{
			std::stringstream ss;
			ss << std::hex << m[1];
			unsigned int vid;
			ss >> vid;
			if (vid != expected_vid)
			{
				Logger(TLogLevel::logWARNING) << "board config: vid " << vid << " doesn't match with expected " << expected_vid << std::endl;
			}
			Logger(TLogLevel::logDEBUG) << "board config: vid=" << vid << std::endl;
			continue;
		}

		if (std::regex_match(line.c_str(), m, std::regex(TOKEN_PID)))
		{
			std::stringstream ss;
			ss << std::hex << m[1];
			unsigned int pid;
			ss >> pid; 
			if (pid != expected_pid)
			{
				Logger(TLogLevel::logWARNING) << "board config: pid " << pid << " doesn't match with expected " << expected_pid << std::endl;
			}
			Logger(TLogLevel::logDEBUG) << "board config: pid=" << pid << std::endl;
			continue;
		}

		if (std::regex_match(line.c_str(), m, std::regex(TOKEN_REGISTER)))
		{
			register_index = stoi(m[1]);
			Logger(TLogLevel::logDEBUG) << "board config: register index: " << register_index << std::endl;
			continue;
		}

		if (std::regex_match(line.c_str(), m, std::regex(TOKEN_BUTTON)))
		{				
			unsigned int bit_index = stoi(m[2]);
			arduino_buttons.push_back(PanelButton(register_index*8 + bit_index, m[1]));

			Logger(TLogLevel::logDEBUG) << "board config: add button: " << m[1] << " [" << register_index << "," << bit_index << "]" << std::endl;
			continue;
		}

		Logger(TLogLevel::logERROR) << "board config: invalid config line (" << current_line_nr << "): " << line << std::endl;
		exit_status = EXIT_FAILURE;
	}

	input_file.close();
	return exit_status;
}

int ArduinoHomeCockpit::connect()
{
	return UsbHidDevice::connect();
}

void ArduinoHomeCockpit::start()
{
	UsbHidDevice::start();
}

void ArduinoHomeCockpit::stop(int timeout)
{
	UsbHidDevice::stop(timeout);
}