/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <regex>
#include "XPLMPlugin.h"
#include "arduino-homecockpit/ArduinoHomeCockpit.h"
#include "core/UsbHidDevice.h"
#include "core/Logger.h"

#define WRITE_BUFFER_SIZE 64
#define READ_BUFFER_SIZE 9

std::filesystem::path get_plugin_path();

ArduinoHomeCockpit::ArduinoHomeCockpit(ClassConfiguration& config) :UsbHidDevice(config, READ_BUFFER_SIZE, WRITE_BUFFER_SIZE)
{
	std::filesystem::path board_config_path = get_plugin_path();
	board_config_path /= "board-config.ini";

	if (read_board_configuration(board_config_path.string(), config.vid, config.pid) != EXIT_SUCCESS)
	{
		Logger(TLogLevel::logERROR) << "Arduino home cockpit. Error parsing board config file" << std::endl;
		return;
	}

	register_buttons(arduino_buttons);
	register_displays(arduino_displays);

	for (auto config_display : get_config().generic_displays)
	{
		int nr_of_bytes = 0;
		for (auto &panel_display : arduino_displays)
		{
			if (panel_display.config_name == config_display.first)
				nr_of_bytes = panel_display.width;
		}
		config_display.second->set_nr_bytes(nr_of_bytes);
	}
}

int ArduinoHomeCockpit::read_board_configuration(std::string file_name, unsigned int expected_vid, unsigned int expected_pid)
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

		if (std::regex_match(line.c_str(), m, std::regex(TOKEN_DISPLAY)))
		{
			unsigned int width = stoi(m[2]);
			arduino_displays.push_back(PanelDisplay(register_index, width, m[1]));

			Logger(TLogLevel::logDEBUG) << "board config: add display: " << m[1] << " [" << register_index << "," << width << "]" << std::endl;
			continue;
		}

		Logger(TLogLevel::logERROR) << "board config: invalid config line (" << current_line_nr << "): " << line << std::endl;
		exit_status = EXIT_FAILURE;
	}

	input_file.close();

	register_selectors(arduino_buttons);

	return exit_status;
}

int ArduinoHomeCockpit::connect(hid_device* _device_handle)
{
	if (_device_handle == NULL)
	{
		if (UsbHidDevice::connect() != EXIT_SUCCESS)
		{
			Logger(TLogLevel::logERROR) << "Arduin Home Cockpit connect. Error during connect" << std::endl;
			return EXIT_FAILURE;
		}
	}
	else
	{
		if (UsbHidDevice::connect(_device_handle) != EXIT_SUCCESS)
		{
			Logger(TLogLevel::logERROR) << "Arduin Home Cockpit connect. Error during connect" << std::endl;
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
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
