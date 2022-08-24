#include "configuration.h"

/*------ Plugin level configuration options ------------*/
Configuration& Configuration::operator=(const Configuration& other)
{
	if (this == &other)
		return *this;
	aircraft_acf = other.aircraft_acf;
	script_file = other.script_file;
	version = other.version;

	device_configs = other.device_configs;
}

void Configuration::clear()
{
	aircraft_acf = "";
	script_file = "";
	version = "";

	device_configs.clear();
}

Configuration::~Configuration()
{
	device_configs.clear();
}

/* ----------- Device specific configuration options ------------------*/
DeviceConfiguration& DeviceConfiguration::operator=(const DeviceConfiguration& other)
{
	if (this == &other)
		return *this;

	device_type = other.device_type;
	pid = other.pid;
	vid = other.vid;
	push_actions = other.push_actions;
	release_actions = other.release_actions;
	light_triggers = other.light_triggers;
	multi_displays = other.multi_displays;
	generic_displays = other.generic_displays;

	return *this;
}

DeviceConfiguration::~DeviceConfiguration()
{
	for (auto act : push_actions)
		act.second.clear();
	push_actions.clear();

	for (auto act : release_actions)
		act.second.clear();
	release_actions.clear();
}
