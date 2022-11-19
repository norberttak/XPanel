/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <string>
#include <vector>
#include <list>
#include <map>
#include "Action.h"
#include "Trigger.h"
#include "MultiPurposeDisplay.h"

class FIPScreen;

class DeviceConfiguration;

/*------ Plugin level configuration options ------------*/
class Configuration
{
public:
	~Configuration();
	Configuration& operator=(const Configuration& other);
	void clear();

	std::string version = "";
	std::string aircraft_acf = "";
	std::string script_file = "";
	std::string aircraft_path = "";

	std::vector<DeviceConfiguration> device_configs;
};

/* ----------- Device specific configuration options ------------------*/
typedef enum {
	UNKNOWN_DEVICE_TYPE,
	SAITEK_MULTI,
	SAITEK_RADIO,
	SAITEK_SWITCH,
	HOME_COCKPIT,
	LOGITECH_FIP
} DeviceType;

class DeviceConfiguration
{
public:
	~DeviceConfiguration();
	DeviceConfiguration& operator=(const DeviceConfiguration& other);
	DeviceType device_type = UNKNOWN_DEVICE_TYPE;
	unsigned int vid = 0;
	unsigned int pid = 0;
	std::string serial_number = "";
	std::map<std::string, std::list<Action*>> push_actions;
	std::map<std::string, std::list<Action*>> release_actions;
	std::map<std::string, std::list<Trigger*>> light_triggers;
	std::map<std::string, MultiPurposeDisplay*> multi_displays;
	std::map<std::string, GenericDisplay*> generic_displays;
	std::map<std::string, FIPScreen*> fip_screens;
};
