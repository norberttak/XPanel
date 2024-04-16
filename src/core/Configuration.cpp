/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Logger.h"
#include "Configuration.h"
#include "fip/FIPScreen.h"

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
	serial_number(other.serial_number)
{
	Logger(TLogLevel::logTRACE) << "ClassConfiguration copy constructor called" << std::endl;
	for (auto push_act : other.push_actions)
		for (auto act : push_act.second)
			push_actions[push_act.first].push_back(new Action(act));

	for (auto release_act : other.release_actions)
		for (auto act : release_act.second)
			release_actions[release_act.first].push_back(new Action(act));

	for (auto enc_inc_act : other.encoder_inc_actions)
		for (auto act : enc_inc_act.second)
			encoder_inc_actions[enc_inc_act.first].push_back(new Action(act));

	for (auto enc_dec_act : other.encoder_dec_actions)
		for (auto act : enc_dec_act.second)
			encoder_dec_actions[enc_dec_act.first].push_back(new Action(act));

	for (auto l_trig : other.light_triggers)
		for (auto trg : l_trig.second)
			light_triggers[l_trig.first].push_back(new Trigger(trg));

	for (auto m_disp : other.multi_displays)
		multi_displays[m_disp.first] = new MultiPurposeDisplay(m_disp.second);

	for (auto g_disp : other.generic_displays)
		generic_displays[g_disp.first] = new GenericDisplay(g_disp.second);

	for (auto fip_screen : other.fip_screens)
		fip_screens[fip_screen.first] = new FIPScreen(fip_screen.second);
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

	for (auto push_act : other.push_actions)
		for (auto act : push_act.second)
				push_actions[push_act.first].push_back(new Action(act));

	for (auto release_act : other.release_actions)
		for (auto act : release_act.second)
			release_actions[release_act.first].push_back(new Action(act));

	for (auto enc_inc_act : other.encoder_inc_actions)
		for (auto act : enc_inc_act.second)
			encoder_inc_actions[enc_inc_act.first].push_back(new Action(act));

	for (auto enc_dec_act : other.encoder_dec_actions)
		for (auto act : enc_dec_act.second)
			encoder_dec_actions[enc_dec_act.first].push_back(new Action(act));

	for (auto l_trig : other.light_triggers)
		for (auto trg : l_trig.second)
			light_triggers[l_trig.first].push_back(new Trigger(trg));
		
	for (auto m_disp : other.multi_displays)
		multi_displays[m_disp.first] = new MultiPurposeDisplay(m_disp.second);

	for (auto g_disp : other.generic_displays)
		generic_displays[g_disp.first] = new GenericDisplay(g_disp.second);

	for (auto fip_screen : other.fip_screens)
		fip_screens[fip_screen.first] = new FIPScreen(fip_screen.second);

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

	for (auto& scr : fip_screens)
		delete scr.second;
	fip_screens.clear();
}

ClassConfiguration::~ClassConfiguration()
{
	Logger(TLogLevel::logTRACE) << "ClassConfiguration destructor called" << std::endl;
	clear();
}
