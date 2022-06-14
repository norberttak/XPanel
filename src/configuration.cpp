#include "configuration.h"

Configuration& Configuration::operator=(const Configuration& other)
{
	if (this == &other)
		return *this;

	device_type = other.device_type;
	pid = other.pid;
	vid = other.vid;
	version = other.version;
	aircraft_acf = other.aircraft_acf;
	script_file = other.script_file;
	push_actions = other.push_actions;
	release_actions = other.release_actions;
	light_triggers = other.light_triggers;
	multi_displays = other.multi_displays;

	return *this;
}

Configuration::~Configuration()
{
	for (auto act : push_actions)
		act.second.clear();
	push_actions.clear();

	for (auto act : release_actions)
		act.second.clear();
	release_actions.clear();
}
