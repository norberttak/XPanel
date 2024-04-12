/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Logger.h"
#include "Configuration.h"

/*------ Plugin level configuration options ------------*/
Configuration& Configuration::operator=(const Configuration& other)
{
	if (this == &other)
		return *this;
	aircraft_acf = other.aircraft_acf;
	script_file = other.script_file;
	version = other.version;
	aircraft_path = other.aircraft_path;

	class_configs = other.class_configs;

	return *this;
}

void Configuration::clear()
{
	aircraft_acf = "";
	script_file = "";
	version = "";
	aircraft_path = "";

	class_configs.clear();
}

Configuration::~Configuration()
{
	Logger(TLogLevel::logTRACE) << "Configuration destructor called" << std::endl;
	clear();
}

/* ----------- Device specific configuration options ------------------*/
ClassConfiguration::ClassConfiguration()
{
	Logger(TLogLevel::logTRACE) << "ClassConfiguration constructor called" << std::endl;
}

ClassConfiguration::ClassConfiguration(const ClassConfiguration& other):
	device_class_type(other.device_class_type),
	pid(other.pid),
	vid(other.vid),
	serial_number(other.serial_number),
	push_actions(other.push_actions),
	release_actions(other.release_actions),
	encoder_inc_actions(other.encoder_inc_actions),
	encoder_dec_actions(other.encoder_dec_actions),
	light_triggers(other.light_triggers),
	multi_displays(other.multi_displays),
	generic_displays(other.generic_displays),
	fip_screens(other.fip_screens)
{
	Logger(TLogLevel::logTRACE) << "ClassConfiguration copy constructor called" << std::endl;
}

ClassConfiguration& ClassConfiguration::operator=(const ClassConfiguration& other)
{
	Logger(TLogLevel::logTRACE) << "ClassConfiguration = operator called" << std::endl;

	if (this == &other)
		return *this;

	device_class_type = other.device_class_type;
	pid = other.pid;
	vid = other.vid;
	serial_number = other.serial_number;
	push_actions = other.push_actions;
	release_actions = other.release_actions;
	encoder_inc_actions = other.encoder_inc_actions;
	encoder_dec_actions = other.encoder_dec_actions;
	light_triggers = other.light_triggers;
	multi_displays = other.multi_displays;
	generic_displays = other.generic_displays;
	fip_screens = other.fip_screens;

	return *this;
}

void ClassConfiguration::clear()
{
	for (auto &act : push_actions)
		act.second.clear();
	push_actions.clear();

	for (auto &act : release_actions)
		act.second.clear();
	release_actions.clear();

	for (auto &act : encoder_inc_actions)
		act.second.clear();
	encoder_inc_actions.clear();

	for (auto &act : encoder_dec_actions)
		act.second.clear();
	encoder_dec_actions.clear();

	for (auto &act : light_triggers)
		act.second.clear();
	light_triggers.clear();
}

ClassConfiguration::~ClassConfiguration()
{
	Logger(TLogLevel::logTRACE) << "ClassConfiguration destructor called" << std::endl;
	clear();
}
