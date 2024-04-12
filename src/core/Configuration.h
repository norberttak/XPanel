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

class ClassConfiguration;

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
	std::string plugin_path = "";

	std::vector<ClassConfiguration> class_configs;
};

/* ----------- Class specific configuration options ------------------*/
typedef enum {
	UNKNOWN_DEVICE_TYPE,
	SAITEK_MULTI,
	SAITEK_RADIO,
	SAITEK_SWITCH,
	HOME_COCKPIT,
	LOGITECH_FIP,
	TRC1000_PFD,
	TRC1000_AUDIO
} DeviceClassType;

class ClassConfiguration
{
public:
	~ClassConfiguration();
	ClassConfiguration();
	ClassConfiguration(const ClassConfiguration& other);
	ClassConfiguration& operator=(const ClassConfiguration& other);
	void clear();

	DeviceClassType device_class_type = UNKNOWN_DEVICE_TYPE;
	unsigned int vid = 0;
	unsigned int pid = 0;
	std::string serial_number = "";
	std::map<std::string, std::list<Action*>> push_actions;
	std::map<std::string, std::list<Action*>> release_actions;
	std::map<std::string, std::list<Action*>> encoder_inc_actions;
	std::map<std::string, std::list<Action*>> encoder_dec_actions;
	std::map<std::string, std::list<Trigger*>> light_triggers;
	std::map<std::string, MultiPurposeDisplay*> multi_displays;
	std::map<std::string, GenericDisplay*> generic_displays;
	std::map<std::string, FIPScreen*> fip_screens;
};
