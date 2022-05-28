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

	return *this;
}

Configuration::~Configuration()
{
	push_actions.clear();
	release_actions.clear();
}
